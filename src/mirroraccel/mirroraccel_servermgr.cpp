#include "mirroraccel_servermgr.h"
#include "mirroraccel_server.h"

mirroraccel::ServerMgr::ServerMgr()
{

}

mirroraccel::ServerMgr::~ServerMgr()
{

}

int mirroraccel::ServerMgr::create(
	const std::string& addr, 
	const std::string& jsonOption )
{
	std::shared_ptr<Server> server(new Server());
	server->
	return 0;
}

void mirroraccel::ServerMgr::destroy(
	int port )
{

}
