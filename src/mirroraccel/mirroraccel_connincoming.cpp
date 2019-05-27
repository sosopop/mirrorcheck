#include "mirroraccel_connincoming.h"
#include <thread>
#include "mirroraccel_connoutgoing.h"
#include "mirroraccel_mirroritem.h"

mirroraccel::ConnIncoming::ConnIncoming(
    Server& server,
    std::shared_ptr<Request> request) :
    server(server),
    request(request)
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
            waitEvent();
            //处理事件
            for (auto& co : conns)
            {
                co->poll();
            }
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

bool mirroraccel::ConnIncoming::waitEvent()
{
    return true;
}

std::shared_ptr<mirroraccel::Request> mirroraccel::ConnIncoming::getRequest()
{
    return request;
}

CURLM * mirroraccel::ConnIncoming::handle()
{
    return curlMutil;
}
