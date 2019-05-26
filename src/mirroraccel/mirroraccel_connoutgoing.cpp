#include "mirroraccel_connoutgoing.h"

mirroraccel::ConnOutgoing::ConnOutgoing(
    std::shared_ptr<MirrorItem> mirror,
    ConnIncoming& incoming):
    incoming(incoming),
    mirror(mirror)
{
    curl = curl_easy_init();
}

mirroraccel::ConnOutgoing::~ConnOutgoing()
{

}
