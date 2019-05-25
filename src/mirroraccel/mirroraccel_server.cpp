#include "mirroraccel_server.h"
#include "mirroraccel_mirroritem.h"
#include "mirroraccel_connincoming.h"
#include "mirroraccel_except.h"

mirroraccel::Server::Server(
    const std::string &addr,
    const std::string &jsonOption)
{
    if (jsonOption.empty())
        throw Except("json empty");

    nlohmann::json &&json = nlohmann::json::parse(jsonOption);
    nlohmann::json &targets = json["targets"];
    if (!targets.is_array())
        throw Except("invalid json");

    for (auto &item : targets)
    {
        auto urlJson = item["url"];
        if (!urlJson.is_string())
            throw Except("invalid json");

        std::string url = urlJson;
        if (url.empty())
            throw Except("invalid mirror url");

        std::lock_guard<std::mutex> lock(mirrorsMux);
        mirrors.push_back(std::make_shared<MirrorItem>(url));
    }

    if (mirrors.size() == 0)
        throw Except("mirrors empty");

    mg_mgr_init(&mgr, this);
    mg_connection *conn = mg_bind(&mgr, addr.c_str(), eventHandler, 0);
    if (conn == nullptr)
        throw Except("can't bind address");

    mg_set_protocol_http_websocket(conn);

    char portBuf[10] = {0};
    //获取监听端口
    mg_conn_addr_to_str(conn, portBuf, sizeof(portBuf), MG_SOCK_STRINGIFY_PORT);
    port = atoi(portBuf);

    //启动监听线程
    pollThread.reset(new std::thread([this] {
        while (!stopSignal)
        {
            mg_mgr_poll(&mgr, 100);
        }
    }));
}

mirroraccel::Server::~Server()
{
    stopSignal = true;
    mg_mgr_free(&mgr);
    pollThread->join();
}

int mirroraccel::Server::getPort()
{
    return port;
}

namespace
{
int startWith(const struct mg_str *str1, const char *str2)
{
    size_t i = 0;
    while (str2[i] && i < str1->len && str2[i] == str1->p[i])
        i++;
    return str2[i];
}
} // namespace

void mirroraccel::Server::eventHandler(struct mg_connection *nc, int ev, void *p, void *user_data)
{
    Server *srv = static_cast<Server *>(nc->mgr->user_data);
    if (ev == MG_EV_HTTP_REQUEST)
    {
        struct http_message *hm = (struct http_message *)p;
        if (startWith(&hm->uri, "/stream/") == 0)
        {
            ConnIncoming *conn = new ConnIncoming(srv);
            nc->user_data = conn;
            mg_send_head(nc, 200, sizeof("world") - 1, 0);
            mg_send(nc, "world", sizeof("world") - 1);
        }
        else
        {
            mg_send_head(nc, 200, sizeof("!!!") - 1, 0);
            mg_send(nc, "!!!", sizeof("!!!") - 1);
        }
    }
    else if (ev == MG_EV_CLOSE)
    {
        if (nc->user_data)
        {
            ConnIncoming *conn = (ConnIncoming *)nc->user_data;
            delete conn;
        }
    }
}

mirroraccel::MirrorList mirroraccel::Server::getMirrorList()
{
    std::lock_guard<std::mutex> lock(mirrorsMux);
    mirroraccel::MirrorList ml = mirrors;
    return ml;
}