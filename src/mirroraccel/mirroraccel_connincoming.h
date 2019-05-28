#ifndef MIRRORACCEL_CONNINCOMING_H_
#define MIRRORACCEL_CONNINCOMING_H_

#include "mongoose.h"
#include "curl/curl.h"
#include <list>
#include <thread>
#include <memory>
#include "mirroraccel_server.h"
#include "mirroraccel_request.h"

namespace mirroraccel
{
class Server;
class ConnOutgoing;
class MirrorItem;

class ConnIncoming
{
    friend class ConnOutgoing;
public:
    ConnIncoming(
        Server& server,
		struct http_message *hm);

    ~ConnIncoming();

public:
	Request& getRequest();
    void reset();

private:
    bool poll();
    CURLM* handle();

private:
    //connection stop signal
    bool stopSignal = false;
    //mongoose poll thread
    std::shared_ptr<std::thread> pollThread = nullptr;
    //HTTP server
    Server& server;
    //mirror connections
    std::vector<std::shared_ptr<ConnOutgoing>>  conns;
    std::mutex connMux;
    //target url
    std::string url;
    //HTTP range
    std::pair<std::int64_t, std::int64_t> range;
    //curl multi handle
    CURLM* curlMutil = nullptr;
    //HTTP request
	Request request;
};
} // namespace mirroraccel
#endif