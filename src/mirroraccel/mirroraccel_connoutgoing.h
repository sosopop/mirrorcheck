#ifndef MIRRORACCEL_CONNOUTGOING_H_
#define MIRRORACCEL_CONNOUTGOING_H_

#include <memory>
#include <curl/curl.h>

namespace mirroraccel
{
class MirrorItem;
class ConnIncoming;

class ConnOutgoing
{
public:
    ConnOutgoing(
        std::shared_ptr<MirrorItem> mirror,
        ConnIncoming &incoming);
    ~ConnOutgoing();

public:
    //任务完成
    void doFinish( CURLcode code );
    void reset();
private:
    static size_t writeCallback(char *bufptr, size_t size, size_t nitems, void *userp);
    static size_t headerCallback(char *bufptr, size_t size, size_t nitems, void *userp);
private:
    enum Status
    {
        ST_QUERY = 0,
        ST_REQUEST,
        ST_CLOSE
    };

private:
    bool stopSignal = false;
    Status status = ST_QUERY;
    ConnIncoming &incoming;
    std::shared_ptr<MirrorItem> mirror;
    CURL *curl = nullptr;
};
} // namespace mirroraccel
#endif