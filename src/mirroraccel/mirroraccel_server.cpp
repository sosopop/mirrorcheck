#include "mirroraccel_server.h"
#include "mirroraccel_mirroritem.h"
#include "mirroraccel_connincoming.h"
#include "mirroraccel_except.h"
#include "mirroraccel_request.h"
#include "mirroraccel_util.h"

#define MAX_CONN_CACHE_SIZE 1024 * 1024

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
    pollThread->join();
    mg_mgr_free(&mgr);
}

int mirroraccel::Server::getPort()
{
    return port;
}

std::vector<std::shared_ptr<mirroraccel::MirrorItem>>& mirroraccel::Server::getMirrors()
{
    return mirrors;
}

void mirroraccel::Server::eventHandler(struct mg_connection *nc, int ev, void *p, void *user_data)
{
    Server *srv = static_cast<Server *>(nc->mgr->user_data);
    if (ev == MG_EV_HTTP_REQUEST)
    {
        struct http_message *hm = (struct http_message *)p;
        if (util::startWith(&hm->uri, "/stream/") == 0)
        {
            ConnIncoming *conn = nullptr;
            if (nc->user_data) {
                conn = static_cast<ConnIncoming *>(nc->user_data);
                conn->reset(std::make_shared<Request>(hm));
            }
            else {
                //发起第一次请求，用于获取content-length
                conn = new ConnIncoming(*srv, std::make_shared<Request>(hm));
                nc->user_data = conn;
                mg_set_timer(nc, mg_time() + 0.05);
            }

            //mg_send_head(nc, 200, sizeof("world") - 1, 0);
            //mg_send(nc, "world", sizeof("world") - 1);
        }
        else
        {
            mg_http_send_error(nc, 404, "stream not found");
        }
    }
    else if (ev == MG_EV_CLOSE)
    {
        if (nc->user_data)
        {
            ConnIncoming *conn = static_cast<ConnIncoming *>(nc->user_data);
            delete conn;
        }
    }
    else if (ev == MG_EV_TIMER)
    {
        ConnIncoming *conn = nullptr;
        if (nc->user_data == nullptr)
            return;
        conn = static_cast<ConnIncoming *>(nc->user_data);
        mg_set_timer(nc, mg_time() + 0.05);
        struct mbuf *io = &nc->send_mbuf;
        if (io->len > MAX_CONN_CACHE_SIZE) {
            //等待消费完毕，再去读取
            return;
        }
        while (true)
        {
            mbuf data = { 0,0,0 };
            conn->readData(data);
            if (data.len) {
                mg_send(nc, data.buf, data.len);
                mbuf_free(&data);
            }
            else {
                break;
            }
        }
    }
}