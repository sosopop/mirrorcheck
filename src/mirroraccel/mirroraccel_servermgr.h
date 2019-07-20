#ifndef MIRRORACCEL_SERVERMGR_H_
#define MIRRORACCEL_SERVERMGR_H_

#include "mirroraccel.h"
#include <string>
#include <map>
#include <memory>
#include <mutex>

namespace mirroraccel
{
class Server;
class ServerMgr
{
public:
	ServerMgr();
	~ServerMgr();

public:
	//创建server
	int create(
		const std::string &addr,
		const std::string &jsonOption);
	//销毁server
	void destroy(
		int port);

private:
	std::mutex srvMux;
	std::map<int, std::shared_ptr<Server>> servers;
};
} // namespace mirroraccel
#endif