#include "mirroraccel_connoutgoing.h"
#include "mirroraccel_connincoming.h"
#include "mirroraccel_request.h"
#include "mirroraccel_mirroritem.h"
#include "mirroraccel_util.h"
#include "mirroraccel_response.h"
#include "mirroraccel_task.h"
#include <iostream>
#include <spdlog/spdlog.h>

mirroraccel::ConnOutgoing::ConnOutgoing(
    std::shared_ptr<MirrorItem> mirror,
    ConnIncoming &incoming) : incoming(incoming),
                              mirror(mirror),
                              regCode("^HTTP/([^\\s]+)\\s([^\\s]+)\\s(.*)\\s*$"),
                              regHeader("^\\s*([^\\s]+)\\s*:\\s*(.+?)\\s*$"),
                              regRange("^bytes\\s+(\\d+)-(\\d+)/(\\d+)$")
{
    spdlog::debug("对外建立镜像连接, {}", mirror->getUrl());
    query();
}

mirroraccel::ConnOutgoing::~ConnOutgoing()
{
    stop(false);
}

void mirroraccel::ConnOutgoing::stop(bool reset)
{
    spdlog::debug("停止对外镜像连接, {}", mirror->getUrl());
    headers.clear();
    curl_multi_remove_handle(incoming.handle(), curl);
    if (reset)
    {
        curl_easy_reset(curl);
    }
    else
    {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
    status = ST_STOPED;
}

void mirroraccel::ConnOutgoing::end(CURLcode code)
{
    if (code != CURLE_OK)
    {
        spdlog::debug("url:{}, error: {}", mirror->getUrl(), curl_easy_strerror(code));
        if (ST_QUERY == status)
        {
            if (code == CURLE_WRITE_ERROR ||
                code == CURLE_ABORTED_BY_CALLBACK) {
                status = ST_QUERY_END;
            }
            else {
                status = ST_QUERY_ERROR;
            }
        }
    }
}

mirroraccel::ConnOutgoing::Status mirroraccel::ConnOutgoing::getStatus()
{
    return status;
}

std::shared_ptr<mirroraccel::Response> mirroraccel::ConnOutgoing::getResponse()
{
    return response;
}

void mirroraccel::ConnOutgoing::query( Status st )
{
    if (!curl)
    {
        curl = curl_easy_init();
    }
    else
    {
        stop(true);
    }

    status = ST_QUERY;
    curl_multi_add_handle(incoming.handle(), curl);

    auto request = incoming.getRequest();

    curl_easy_setopt(curl, CURLOPT_PRIVATE, this);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, request->getHeaders());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_URL, mirror->getUrl().c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5000);
    curl_easy_setopt(curl, CURLOPT_ENCODING, "");
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &headerCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferinfoCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
}

void mirroraccel::ConnOutgoing::request()
{
    if (ST_QUERY_END != status &&
        ST_TRANS != status &&
        ST_QUERY_ERROR != status) {

    }
    auto task = incoming.fetchTask();
    if (task == nullptr) {
        return;
    }
    //第二期支持多连接传输
    spdlog::debug("准备发起 task request：{}", mirror->getUrl());
    query(ST_TRANS);
}

size_t mirroraccel::ConnOutgoing::writeCallback(char *bufptr, size_t size, size_t nitems, void *userp)
{
    ConnOutgoing *conn = static_cast<ConnOutgoing *>(userp);
    if (conn->task->finished())
    {
        return 0;
    }
    if (conn->task->writeData(bufptr, size * nitems) > TASK_MAX_BUFFER_SIZE)
    {
        conn->status = ST_TRANS_WAIT_BUF_AVALID;
        curl_easy_pause(conn->curl, CURLPAUSE_RECV);
    }
    return size * nitems;
}

size_t mirroraccel::ConnOutgoing::headerCallback(char *bufptr, size_t size, size_t nitems, void *userp)
{
    ConnOutgoing *conn = static_cast<ConnOutgoing *>(userp);
    if (ST_QUERY == conn->status)
    {
        try
        {
            //http头缓存下来准备输出到外部
            if (conn->response == nullptr)
            {
                conn->response = std::make_shared<Response>();
            }
            std::string header(bufptr, size * nitems);
            conn->response->headers += header;
            spdlog::debug("{}", header);

            //匹配返回码
            if (conn->headers.size() == 0)
            {
                std::smatch matched;
                //header = "HTTP/1.1 200 OK";
                if (std::regex_match(header, matched, conn->regCode, std::regex_constants::match_default) && matched.size() == 4)
                {
                    conn->response->ver = matched[1].str();
                    conn->response->status = matched[2].str();
                    conn->response->msg = matched[3].str();

                    if (conn->response->status != "200" && conn->response->status != "206")
                    {
                        throw 0;
                    }
                }
                else
                {
                    throw 0;
                }
                conn->headers.push_back(header);
            }
            else
            {
                //header完成
                if (header == "\r\n")
                {
                    if (conn->incoming.onQueryEnd(conn, conn->response))
                    {
                        //第一个获取到任务的连接直接进入传输状态
                        spdlog::debug("第一个获取到任务的连接直接进入传输状态：{}", conn->mirror->getUrl());
                        conn->task = conn->incoming.fetchTask();
                        if (conn->task == nullptr)
                        {
                            conn->status = ST_QUERY_ERROR;
                            return 0;
                        }
                        conn->status = ST_TRANS;
                        conn->task->writeHeader((char *)conn->response->headers.c_str(), conn->response->headers.length());
                    }
                    else
                    {
                        conn->status = ST_QUERY_END;
                        return 0;
                    }
                }
                else
                {
                    std::smatch matched;
                    if (std::regex_match(header, matched, conn->regHeader,
                                         std::regex_constants::match_default) &&
                        matched.size() == 3)
                    {
                        std::string key = matched[1].str();
                        std::string value = matched[2].str();
                        if (util::icompare(key, "content-range"))
                        {
                            if (std::regex_match(value, matched, conn->regRange,
                                                 std::regex_constants::match_default) &&
                                matched.size() == 4)
                            {
                                conn->response->rangeStart = std::atoll(matched[1].str().c_str());
                                conn->response->rangeEnd = std::atoll(matched[2].str().c_str());
                                conn->response->rangeTotal = std::atoll(matched[3].str().c_str());
                            }
                        }
                        else if (util::icompare(key, "content-length"))
                        {
                            conn->response->contentLength = std::atoll(value.c_str());
                        }
                    }
                    conn->headers.push_back(header);
                }
            }
        }
        catch (int e)
        {
            conn->status = ST_QUERY_ERROR;
            return e;
        }
    }
    return size * nitems;
}

int mirroraccel::ConnOutgoing::xferinfoCallback(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    ConnOutgoing *conn = static_cast<ConnOutgoing *>(p);
    if (conn->status == ST_TRANS_WAIT_BUF_AVALID)
    {
        if (conn->task->size() <= TASK_MAX_BUFFER_SIZE)
        {
            conn->status = ST_TRANS;
            curl_easy_pause(conn->curl, CURLPAUSE_RECV_CONT);
        }
        //spdlog::trace("xferinfoCallback ST_TRANS_WAIT_BUF_AVALID");
    }
    else
    {
        //spdlog::trace("xferinfoCallback");
    }
    return 0;
}
