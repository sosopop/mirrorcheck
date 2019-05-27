#ifndef MIRRORACCEL_SERVER_H_
#define MIRRORACCEL_SERVER_H_
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include "nlohmann/json.hpp"
#include "mongoose.h"

namespace mirroraccel
{
class MirrorItem;
class Server
{
public:
    Server(
        const std::string &addr,
        const std::string &jsonOption);
    ~Server();

public:
    int getPort();
    std::vector<std::shared_ptr<MirrorItem>>& getMirrors();
private:
    static void eventHandler(struct mg_connection *nc, int ev, void *p, void *user_data);

private:
    int port = 0;
    mg_mgr mgr = {};
    bool stopSignal = false;
    std::shared_ptr<std::thread> pollThread = nullptr;
    //std::mutex mirrorsMux;
    std::vector<std::shared_ptr<MirrorItem>> mirrors;
};
} // namespace mirroraccel
#endif