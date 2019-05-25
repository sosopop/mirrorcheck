#include "mirroraccel_connincoming.h"
#include <thread>

mirroraccel::ConnIncoming::ConnIncoming(Server *srv) : server(srv)
{

    //启动监听线程
    pollThread.reset(new std::thread([this] {

    }));
}

mirroraccel::ConnIncoming::~ConnIncoming()
{
}