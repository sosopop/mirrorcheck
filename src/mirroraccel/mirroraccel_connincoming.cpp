#include "mirroraccel_connincoming.h"
#include <thread>
#include "mirroraccel_mirroritem.h"

mirroraccel::ConnIncoming::ConnIncoming(
    Server& server,
    const std::string& url,
    std::vector<std::shared_ptr<MirrorItem>> mirrors) :
    server(server), mirrors(mirrors), url(url)
{
    //启动监听线程
    pollThread.reset(new std::thread([this] {
        //生成连接
        for (auto& item : this->mirrors )
        {
            item->getUrl();
        }
    }));
}

mirroraccel::ConnIncoming::~ConnIncoming()
{
}