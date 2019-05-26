#ifndef MIRRORACCEL_CONNINCOMING_H_
#define MIRRORACCEL_CONNINCOMING_H_

#include "mongoose.h"
#include <list>
#include <thread>
#include <memory>
#include "mirroraccel_server.h"

namespace mirroraccel
{
class Server;
class ConnOutgoing;
class MirrorItem;

class ConnIncoming
{
public:
    ConnIncoming(
        Server& server,
        const std::string& url,
        std::vector<std::shared_ptr<MirrorItem>> mirrors);

    ~ConnIncoming();

private:
    //connection stop signal
    bool stopSignal = false;
    //mongoose poll thread
    std::shared_ptr<std::thread> pollThread = nullptr;
    //HTTP server
    Server& server;
    //mirror information
    std::vector<std::shared_ptr<MirrorItem>> mirrors;
    //mirror connections
    std::vector<std::shared_ptr<ConnOutgoing>>  conns;
    std::mutex connMux;
    //target url
    std::string url;
    //HTTP range
    std::pair<std::int64_t, std::int64_t> range;
};
} // namespace mirroraccel
#endif