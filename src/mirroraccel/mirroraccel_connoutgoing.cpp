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
    curl_easy_cleanup(curl);
}

bool mirroraccel::ConnOutgoing::poll()
{
    switch ( status)
    {
    case ST_QUERY:
        doQuery();
    	break;
    }
    return true;
}

bool mirroraccel::ConnOutgoing::doQuery()
{
    return true;
}
