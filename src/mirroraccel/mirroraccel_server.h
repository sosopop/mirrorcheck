#ifndef MIRRORACCEL_SERVER_H_
#define MIRRORACCEL_SERVER_H_
#include <string>
#include <thread>
#include <list>
#include <mutex>
#include "nlohmann/json.hpp"
#include "mongoose.h"

namespace mirroraccel
{
class MirrorItem;
typedef std::list<std::shared_ptr<MirrorItem>> MirrorList;

class Server
{
public:
    Server(
        const std::string &addr,
        const std::string &jsonOption);
    ~Server();

public:
    int getPort();
    MirrorList getMirrorList();

private:
    static void eventHandler(struct mg_connection *nc, int ev, void *p, void *user_data);

private:
    int port = 0;
    mg_mgr mgr = {};
    bool stopSignal = false;
    std::shared_ptr<std::thread> pollThread = nullptr;
    std::mutex mirrorsMux;
    MirrorList mirrors;
};
} // namespace mirroraccel
#endif