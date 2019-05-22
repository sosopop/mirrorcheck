#include "mirroraccel_servermgr.h"
#include "mirroraccel_server.h"

mirroraccel::ServerMgr::ServerMgr()
{

}

mirroraccel::ServerMgr::~ServerMgr()
{
    std::lock_guard<std::mutex> lock(srvMux);
    servers.clear();
}

int mirroraccel::ServerMgr::create(
	const std::string& addr, 
	const std::string& jsonOption )
{
	std::shared_ptr<Server> server(new Server( addr, jsonOption));
    int port = server->getPort();
    if (port == 0) {
        return port;
    }
    std::lock_guard<std::mutex> lock(srvMux);
    servers.insert(std::make_pair(port, server));
	return port;
}

void mirroraccel::ServerMgr::destroy(
	int port )
{
    std::lock_guard<std::mutex> lock(srvMux);
    servers.erase(port);
}
