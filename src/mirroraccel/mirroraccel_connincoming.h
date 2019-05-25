#ifndef MIRRORACCEL_CONNINCOMING_H_
#define MIRRORACCEL_CONNINCOMING_H_

#include "mongoose.h"
#include <list>
#include <thread>
#include <memory>

namespace mirroraccel
{
class Server;
class ConnOutgoing;
class ConnIncoming
{
public:
    ConnIncoming(Server *srv);
    ~ConnIncoming();

private:
    bool stopSignal = false;
    std::shared_ptr<std::thread> pollThread = nullptr;
    std::list<std::shared_ptr<ConnOutgoing>> conns;
    Server *server = nullptr;
};
} // namespace mirroraccel
#endif