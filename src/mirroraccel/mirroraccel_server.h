#ifndef MIRRORACCEL_SERVER_H_
#define MIRRORACCEL_SERVER_H_
#include <string>

namespace mirroraccel
{
    class Server
    {
    public:
        Server( 
			const std::string& addr,
			const std::string& jsonOption);
        ~Server();
	public:
		int getPort();
	private:
		int port = 0;
    };
}
#endif