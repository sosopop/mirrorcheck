#include "mirroraccel_connincoming.h"
#include <thread>
#include "mirroraccel_connoutgoing.h"
#include "mirroraccel_mirroritem.h"

mirroraccel::ConnIncoming::ConnIncoming(
    Server& server,
    struct http_message *hm) :
    server(server),
    request(hm)
{
    curlMutil = curl_multi_init();

    std::vector<std::shared_ptr<MirrorItem>>& mirrors = server.getMirrors();
    //生成连接
    for (auto& item : mirrors)
    {
        std::shared_ptr<ConnOutgoing> outgoing = std::make_shared<ConnOutgoing>(item, *this);
        conns.push_back(outgoing);
    }

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

mirroraccel::ConnIncoming::~ConnIncoming()
{
    stopSignal = true;
    pollThread->join();
    conns.clear();
    curl_multi_cleanup(curlMutil);
}

bool mirroraccel::ConnIncoming::poll()
{
    //发起请求
    {
        int stillRunning = 0;
        curl_multi_perform(curlMutil, &stillRunning);
        printf("current running handles %d \n", stillRunning);
    }

    //检查事件
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
            Sleep(100);
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

    {
        CURLMsg *msg = nullptr;
        int msgs_left = 0;
        /* See how the transfers went */
        while ((msg = curl_multi_info_read(curlMutil, &msgs_left))) {
            if (msg->msg != CURLMSG_DONE) 
                continue;

            ConnOutgoing* conn = nullptr;
            curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &conn);
            conn->doFinish(msg->data.result);
        }
    }
    return true;
}

mirroraccel::Request& mirroraccel::ConnIncoming::getRequest()
{
    return request;
}

void mirroraccel::ConnIncoming::reset()
{
    for ( auto& co: conns)
    {
        co->reset();
    }
}

CURLM * mirroraccel::ConnIncoming::handle()
{
    return curlMutil;
}
