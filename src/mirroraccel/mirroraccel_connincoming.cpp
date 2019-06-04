#include "mirroraccel_connincoming.h"
#include "mirroraccel_connoutgoing.h"
#include "mirroraccel_mirroritem.h"
#include "mirroraccel_request.h"
#include "mirroraccel_response.h"
#include "mirroraccel_task.h"
#include <thread>
#include <iostream>
#include <spdlog/spdlog.h>

mirroraccel::ConnIncoming::ConnIncoming(
    Server& server,
	std::shared_ptr<Request> request) :
    server(server),
    request(request)
{
    curlMutil = curl_multi_init();

    spdlog::debug("new connection incoming, {}", request->getUrl());
    std::vector<std::shared_ptr<MirrorItem>>& mirrors = server.getMirrors();
    //生成连接
    for (auto& item : mirrors)
    {
        std::shared_ptr<ConnOutgoing> outgoing = std::make_shared<ConnOutgoing>(item, *this);
        conns.insert(outgoing);
    }

    //启动监听线程
    start();
}

mirroraccel::ConnIncoming::~ConnIncoming()
{
    stop();
    conns.clear();
    curl_multi_cleanup(curlMutil);
    spdlog::debug("connection incoming disconnected, {}", request->getUrl());
}

void mirroraccel::ConnIncoming::stop()
{
    stopSignal = true;
    pollThread->join();
}

void mirroraccel::ConnIncoming::start()
{
    stopSignal = false;
    //启动监听线程
    pollThread.reset(new std::thread([this] {
        //生成连接
        while (!stopSignal)
        {
            //等待事件
            poll();
        }
    }));
}

void mirroraccel::ConnIncoming::dispatch()
{
    CURLMsg *msg = nullptr;
    int msgs_left = 0;

    //http处理
    while ((msg = curl_multi_info_read(curlMutil, &msgs_left))) {
        ConnOutgoing* conn = nullptr;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &conn);
        if (msg->msg != CURLMSG_DONE) {
            continue;
        }
        conn->end(msg->data.result);
    }

    switch (status)
    {
    case ST_QUERY:
        if (stillRunning == 0)
        {
            //返回失败信息
        }
        break;
    case ST_QUERY_END:
        //开始下载任务
        for (auto co: conns)
        {
            if (co->getStatus() != ConnOutgoing::ST_QUERY_ERROR) {
                //分配下载任务
                //co->request();
            }
        }
        status = ST_TRANS;
        break;
    case ST_TRANS:
        break;
    default:
        break;
    }
}

void mirroraccel::ConnIncoming::eventWait()
{
    struct timeval timeout;
    int rc; /* select() return code */
    CURLMcode mc; /* curl_multi_fdset() return code */

    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int maxfd = -1;

    long curlTimeo = -1;

    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

    /* set a suitable timeout to play around with */
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    curl_multi_timeout(curlMutil, &curlTimeo);
    if (curlTimeo >= 0) {
        timeout.tv_sec = curlTimeo / 1000;
        if (timeout.tv_sec > 1)
            timeout.tv_sec = 1;
        else
            timeout.tv_usec = (curlTimeo % 1000) * 1000;
    }

    /* get file descriptors from the transfers */
    mc = curl_multi_fdset(curlMutil, &fdread, &fdwrite, &fdexcep, &maxfd);

    if (mc != CURLM_OK) {
        fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
    }

    /* On success the value of maxfd is guaranteed to be >= -1. We call
       select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
       no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
       to sleep 100ms, which is the minimum suggested value in the
       curl_multi_fdset() doc. */

    if (maxfd == -1) {
#ifdef _WIN32
        Sleep(10);
        rc = 0;
#else
        /* Portable sleep for platforms other than Windows. */
        struct timeval wait = { 0, 100 * 1000 }; /* 100ms */
        rc = select(0, NULL, NULL, NULL, &wait);
#endif
    }
    else {
        /* Note that on some platforms 'timeout' may be modified by select().
           If you need access to the original value save a copy beforehand. */
        rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
    }
}

void mirroraccel::ConnIncoming::perform()
{
    stillRunning = 0;
    curl_multi_perform(curlMutil, &stillRunning);
    if (stillRunning)
        spdlog::debug("current running handles {}", stillRunning);
}

bool mirroraccel::ConnIncoming::poll()
{
    //发起请求
    perform();
    //等待事件
    eventWait();
    //状态机处理
    dispatch();
    return true;
}

std::shared_ptr<mirroraccel::Request> mirroraccel::ConnIncoming::getRequest()
{
    return request;
}

void mirroraccel::ConnIncoming::reset(std::shared_ptr<Request> request)
{
    //停止请求线程
    stop();
    this->request = request;
    for (auto co: conns)
    {
        //重新发起请求
        co->query();
    }
    //启动线程
    start();
}

CURLM * mirroraccel::ConnIncoming::handle()
{
    return curlMutil;
}

void mirroraccel::ConnIncoming::removeQueryConn()
{
    CURLMsg *msg = nullptr;
    int msgs_left = 0;
    //http处理
    while ((msg = curl_multi_info_read(curlMutil, &msgs_left))) {
        ConnOutgoing* conn = nullptr;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &conn);
        //检测到查询结束，则关闭掉其他正在查询的连接
        if (status == ST_QUERY) {
            if (conn->getStatus() == ConnOutgoing::ST_QUERY) {
                //分配下载任务
                //conn->request();
            }
        }
    }

    switch (status)
    {
    case ST_QUERY:
        break;
    case ST_QUERY_END:
        break;
    case ST_TRANS:
        break;
    default:
        break;
    }
}

bool mirroraccel::ConnIncoming::onQueryEnd(ConnOutgoing* conn, std::shared_ptr<Response> response)
{
    if (status == ST_QUERY) {
        this->response = response;
        //非range请求
        if (response->rangeTotal == 0) {
            taskSet.insert(std::make_shared<Task>(conn, 0, response->contentLength));
        }
        else {
            std::int64_t rangeStart = response->rangeStart;
            std::int64_t rangeLen = response->contentLength;
            if(response->rangeEnd != response->rangeStart && rangeLen == 0)
                rangeLen = response->rangeEnd - response->rangeStart + 1;
            taskSet.insert(std::make_shared<Task>(
                conn, 
                response->rangeStart,
                rangeLen));
        }
        status = ST_QUERY_END;
        return true;
    }
    return false;
}
