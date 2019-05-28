#include "mirroraccel_connoutgoing.h"
#include "mirroraccel_connincoming.h"
#include "mirroraccel_request.h"
#include "mirroraccel_mirroritem.h"

mirroraccel::ConnOutgoing::ConnOutgoing(
    std::shared_ptr<MirrorItem> mirror,
    ConnIncoming& incoming):
    incoming(incoming),
    mirror(mirror)
{
    curl = curl_easy_init();
    curl_multi_add_handle(incoming.handle(), curl);

    auto& request = incoming.getRequest();

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, request.getHeaders());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_URL, mirror->getUrl());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5000);
    //curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &HeaderFunction);
    //curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &DownloadFunction);
    //curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    //curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, this);
}

mirroraccel::ConnOutgoing::~ConnOutgoing()
{
    curl_multi_remove_handle(incoming.handle(), curl);
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
