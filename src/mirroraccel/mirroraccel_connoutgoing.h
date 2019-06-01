#ifndef MIRRORACCEL_CONNOUTGOING_H_
#define MIRRORACCEL_CONNOUTGOING_H_

#include <memory>
#include <list>
#include <curl/curl.h>
#include <regex>

namespace mirroraccel
{
class MirrorItem;
class ConnIncoming;
struct Response;

class ConnOutgoing
{
public:
    ConnOutgoing(
        std::shared_ptr<MirrorItem> mirror,
        ConnIncoming &incoming);
    ~ConnOutgoing();
public:
    enum Status
    {
        ST_QUERY = 0,
        ST_QUERY_END,
        ST_QUERY_ERROR,
        ST_REQUEST,
        ST_REQUEST_END,
        ST_REQUEST_ERROR
    };
public:
    //任务完成
    void end( CURLcode code );
    void query();
    void request();
    void stop(bool reset = true);
    Status getStatus();
private:
    static size_t writeCallback(char *bufptr, size_t size, size_t nitems, void *userp);
	static size_t headerCallback(char *bufptr, size_t size, size_t nitems, void *userp);
private:
	//bool resetSignal = false;
    bool stopSignal = false;
    Status status = ST_QUERY;
    ConnIncoming &incoming;
    std::shared_ptr<MirrorItem> mirror;
    CURL *curl = nullptr;
    std::shared_ptr<Response> response = nullptr;
    std::list<std::string> headers;
    std::regex regCode;
    std::regex regHeader;
    std::regex regRange;
};
} // namespace mirroraccel
#endif