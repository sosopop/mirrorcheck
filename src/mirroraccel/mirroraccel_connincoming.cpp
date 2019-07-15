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
    Server &server,
    std::shared_ptr<Request> request) : server(server),
                                        request(request)
{
    curlMutil = curl_multi_init();

    spdlog::debug("new connection incoming, {}", request->getUrl());
    std::vector<std::shared_ptr<MirrorItem>> &mirrors = server.getMirrors();
    //生成连接
    for (auto &item : mirrors)
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
    while ((msg = curl_multi_info_read(curlMutil, &msgs_left)))
    {
        ConnOutgoing *conn = nullptr;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &conn);
        if (msg->msg != CURLMSG_DONE)
        {
            continue;
        }
        conn->end(msg->data.result);
    }
    if (stillRunning == 0 && conns.size() == 1) {
        int a = 0;
    }
    switch (status)
    {
    case ST_QUERY:
        //如果是查询状态，并且当前正在运行的连接为0，则返回错误信息
        //开始下载请求
        for (auto co : conns)
        {
            //排除掉已经开始下载的连接
            auto st = co->getStatus();
        }
        break;
    case ST_TRANS:
        for (auto iter = conns.begin(); iter != conns.end();) {
            auto co = *iter;
            //排除掉已经开始下载的连接
            auto st = co->getStatus();
            switch (st)
            {
            case ConnOutgoing::ST_QUERY_ERROR:
            case ConnOutgoing::ST_STOPED:
                iter = conns.erase(iter);
                continue;
                break;
            case ConnOutgoing::ST_QUERY:
            case ConnOutgoing::ST_QUERY_END:
            case ConnOutgoing::ST_TRANS_END:
                if (!co->request()) {
                    iter = conns.erase(iter);
                    continue;
                }
                break;
            case ConnOutgoing::ST_TRANS:
            case ConnOutgoing::ST_TRANS_WAIT_BUF_AVALID:
                break;
            default:
                //co->stop();
                break;
            }
            iter++;
        }
        break;
    default:
        break;
    }
}

void mirroraccel::ConnIncoming::eventWait()
{
    struct timeval timeout;
    int rc;       /* select() return code */
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
    if (curlTimeo >= 0)
    {
        timeout.tv_sec = curlTimeo / 1000;
        if (timeout.tv_sec > 1)
            timeout.tv_sec = 1;
        else
            timeout.tv_usec = (curlTimeo % 1000) * 1000;
    }

    /* get file descriptors from the transfers */
    mc = curl_multi_fdset(curlMutil, &fdread, &fdwrite, &fdexcep, &maxfd);

    if (mc != CURLM_OK)
    {
        fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
    }

    /* On success the value of maxfd is guaranteed to be >= -1. We call
       select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
       no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
       to sleep 100ms, which is the minimum suggested value in the
       curl_multi_fdset() doc. */

    if (maxfd == -1)
    {
#ifdef _WIN32
        Sleep(10);
        rc = 0;
#else
        /* Portable sleep for platforms other than Windows. */
        struct timeval wait = {0, 100 * 1000}; /* 100ms */
        rc = select(0, NULL, NULL, NULL, &wait);
#endif
    }
    else
    {
        /* Note that on some platforms 'timeout' may be modified by select().
           If you need access to the original value save a copy beforehand. */
        rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
    }
}

void mirroraccel::ConnIncoming::perform()
{
    int newRuning = 0;
    curl_multi_perform(curlMutil, &newRuning);
    if (stillRunning != newRuning)
    {
        stillRunning = newRuning;
        spdlog::debug("current running handles {}, conns {}", stillRunning, conns.size());
        //if (stillRunning == 0)
        //{
        //    status = ST_TRANS_END;
        //}
    }

    {
        if (conns.size() == 0) {
            std::lock_guard<std::mutex> lock(taskDataMux);
            if (taskWorkingList.size() == 0)
            {
                status = ST_CLOSED;
            }
            else
            {
                std::shared_ptr<Task> task = *taskWorkingList.begin();
                if (task->emptyBuffer()) {
                    status = ST_CLOSED;
                }
            }
        }
    }

    //if (status == ST_TRANS_END)
    //{
    //    //如果没有准备发送的任务，则关闭连接
    //    std::lock_guard<std::mutex> lock(taskDataMux);
    //    //if (conns.size() == 0)
    //    {
    //        if (taskWorkingList.size() == 0)
    //        {
    //            status = ST_CLOSED;
    //        }
    //        else
    //        {
    //            std::shared_ptr<Task> task = *taskWorkingList.begin();
    //            if (task->emptyBuffer()) {
    //                status = ST_CLOSED;
    //            }
    //        }
    //    }
    //    //else
    //    //{
    //    //    std::shared_ptr<Task> task = *taskWorkingList.begin();
    //    //    if (task->size() == 0)
    //    //    {
    //    //        status = ST_CLOSED;
    //    //    }
    //    //}
    //}
}

bool mirroraccel::ConnIncoming::poll()
{
    //等待事件
    eventWait();
    //状态机处理
    dispatch();
    //发起请求
    perform();
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
    for (auto co : conns)
    {
        //重新发起请求
        co->query();
    }
    //启动线程
    start();
}

void mirroraccel::ConnIncoming::readData(mbuf &buf)
{
    std::lock_guard<std::mutex> lock(taskDataMux);
    //如果有http头，先发送http头
    if (!header.empty()) {
        mbuf_init(&buf, header.length());
        mbuf_append(&buf, header.c_str(), header.length());
        header = "";
        return;
    }
    std::shared_ptr<Task> task = nullptr;
    auto it = taskWorkingList.begin();
    if (it == taskWorkingList.end()) {
        return;
    }
    task = *it;

    if (task != nullptr && task->size())
        task->read(buf);

    if (task->readFinished())
        taskWorkingList.pop_front();
}

void mirroraccel::ConnIncoming::writeHeader(const std::string& header)
{
    std::lock_guard<std::mutex> lock(taskDataMux);
    this->header = header;
}

mirroraccel::ConnIncoming::Status mirroraccel::ConnIncoming::getStatus()
{
    return status;
}

CURLM *mirroraccel::ConnIncoming::handle()
{
    return curlMutil;
}

bool mirroraccel::ConnIncoming::onQueryEnd(ConnOutgoing *conn, std::shared_ptr<Response> response)
{
    if (status == ST_QUERY)
    {
        this->response = response;
        //非range请求
        if (response->rangeTotal == 0)
        {
            rangeStart = 0;
            rangeSize = response->contentLength;
        }
        else
        {
            rangeStart = response->rangeStart;
            rangeSize = response->contentLength;
            if (response->rangeEnd != response->rangeStart && rangeSize == 0)
            {
                rangeSize = response->rangeEnd - response->rangeStart + 1;
            }
        }
        status = ST_TRANS;
        return true;
    }
    return false;
}

std::shared_ptr<mirroraccel::Task> mirroraccel::ConnIncoming::fetchTask(std::shared_ptr<Task> lastTask)
{
    //没有任务返回空
    if (rangeSize == rangeCurSize)
    {
        return nullptr;
    }

    std::shared_ptr<Task> task = nullptr;

    if ( 
        lastTask != nullptr &&
        taskWorkingList.size() > 0 &&
        conns.size() > 1 ) 
    {
        //通过任务分裂的方式，抢占其他任务

        //先找到传输数据最少的连接和对应的任务
        std::shared_ptr<ConnOutgoing> minco = nullptr;
        for (auto iter = conns.begin(); iter != conns.end(); iter++) {
            auto co = *iter;
            auto task = co->getTask();
            if (task == nullptr) {
                //已经有完成任务的连接情况下，有还没有获取任务的连接，则直接认定为最慢连接，需要移除
                minco = co;
                break;
            }
            if (minco != nullptr) {
                if (co->totalIoSize() < minco->totalIoSize()) {
                    minco = co;
                }
            }
            else {
                minco = co;
            }
        }

        //找到心仪的连接
        if (minco != nullptr) {
            //通过检查是否是相同任务，判断出如果最慢连接是当前连接，则直接退出
            auto minTask = minco->getTask();
            if (minTask != nullptr) {
                if (minTask == lastTask)
                {
                    return nullptr;
                }
                if (!minTask->wirteFinished()) {
                    //分裂任务
                    task = std::make_shared<Task>(
                        minTask->rangeStart + minTask->rangeCurWriteSize,
                        minTask->rangeSize - minTask->rangeCurWriteSize);
                    //收缩任务，停止最慢连接
                    minTask->forceEnd();
                    minco->stop();

                    //插入分裂后的新任务
                    for (auto iter = taskWorkingList.begin(); iter != taskWorkingList.end(); iter++) {
                        if (*iter == minTask) {
                            taskWorkingList.insert(++iter, task);
                            break;
                        }
                    }
                    return task;
                }
                else {
                    minco->stop();
                }
            }
        }
    }

    auto readSize = rangeSize - rangeCurSize;
    if (readSize > TASK_DATA_SIZE && conns.size() > 1)
    {
        readSize = TASK_DATA_SIZE;
    }

    task = std::make_shared<Task>(
        rangeStart + rangeCurSize, readSize);

    rangeCurSize += readSize;
    taskWorkingList.push_back(task);
    return task;
}
