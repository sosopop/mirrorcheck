#ifndef MIRRORACCEL_SERVERMGR_H_
#define MIRRORACCEL_SERVERMGR_H_

#include "mirroraccel.h"
#include <string>
#include <map>
#include <memory>

namespace mirroraccel
{
	class Server;
	class ServerMgr
	{
	public:
		ServerMgr();
		~ServerMgr();
	public:
		//´´½¨server
		int create(
			const std::string& addr,
			const std::string& jsonOption);
		//Ïú»Ùserver
		void destroy(
			int port);
	private:
		std::map<int, std::shared_ptr<Server>> servers;
	};
}
#endif