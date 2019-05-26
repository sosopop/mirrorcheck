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
            ConnIncoming& incoming);
        ~ConnOutgoing();
    private:
        ConnIncoming& incoming;
        std::shared_ptr<MirrorItem> mirror;
        CURL* curl = nullptr;
    };
}
#endif