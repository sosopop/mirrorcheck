#include "mirroraccel_connincoming.h"
#include <thread>
#include "mirroraccel_connoutgoing.h"
#include "mirroraccel_mirroritem.h"

mirroraccel::ConnIncoming::ConnIncoming(
    Server& server,
    const std::string& url,
    std::vector<std::shared_ptr<MirrorItem>> mirrors) :
    server(server), mirrors(mirrors), url(url)
{
    //生成连接
    for (auto& item : this->mirrors)
    {
        std::shared_ptr<ConnOutgoing> outgoing = std::make_shared<ConnOutgoing>(item, *this);
        conns.push_back(outgoing);
    }

    //启动监听线程
    pollThread.reset(new std::thread([this] {
        //生成连接
        while (stopSignal)
        {
            //等待事件
            waitEvent();
            //处理事件
            for (auto& co : conns)
            {
                co->poll();
            }
        }
        //清理连接
        conns.clear();
    }));
}

mirroraccel::ConnIncoming::~ConnIncoming()
{

}

bool mirroraccel::ConnIncoming::waitEvent()
{
    return true;
}
