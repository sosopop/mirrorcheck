#include "mirroraccel_connoutgoing.h"
#include "mirroraccel_connincoming.h"
#include "mirroraccel_request.h"
#include "mirroraccel_mirroritem.h"
#include "mirroraccel_util.h"
#include "mirroraccel_response.h"
#include <iostream>
#include <spdlog/spdlog.h>

mirroraccel::ConnOutgoing::ConnOutgoing(
    std::shared_ptr<MirrorItem> mirror,
    ConnIncoming& incoming) :
    incoming(incoming),
    mirror(mirror),
    regCode("^HTTP/([^\\s]+)\\s([^\\s]+)\\s(.*)\\s*$"),
    regHeader("^\\s*([^\\s]+)\\s*:\\s*(.+?)\\s*$"),
    regRange("^bytes\\s+(\\d+)-(\\d+)/(\\d+)$")
{
    curl = curl_easy_init();
    query();
}

mirroraccel::ConnOutgoing::~ConnOutgoing()
{
    stop(false);
}

void mirroraccel::ConnOutgoing::end(CURLcode code)
{
	switch (status)
	{
	case ST_QUERY:
		break;
	case ST_REQUEST:
		break;
	default:
		break;
	}
}

void mirroraccel::ConnOutgoing::stop(bool reset)
{
    headers.clear();

    curl_multi_remove_handle(incoming.handle(), curl);
    if (reset)
        curl_easy_reset(curl);
    else
        curl_easy_cleanup(curl);
}

mirroraccel::ConnOutgoing::Status mirroraccel::ConnOutgoing::getStatus()
{
    return status;
}

void mirroraccel::ConnOutgoing::query()
{
	curl_multi_add_handle(incoming.handle(), curl);

	auto request = incoming.getRequest();

	curl_easy_setopt(curl, CURLOPT_PRIVATE, this);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, request->getHeaders());
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_URL, mirror->getUrl().c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5000);
	curl_easy_setopt(curl, CURLOPT_ENCODING, "");
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &headerCallback);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
}

void mirroraccel::ConnOutgoing::request()
{
}

size_t mirroraccel::ConnOutgoing::writeCallback(char * bufptr, size_t size, size_t nitems, void * userp)
{
	ConnOutgoing* conn = static_cast<ConnOutgoing*>(userp);
    return size * nitems;
}

size_t mirroraccel::ConnOutgoing::headerCallback(char * bufptr, size_t size, size_t nitems, void * userp)
{
	ConnOutgoing* conn = static_cast<ConnOutgoing*>(userp);
    if (ST_QUERY == conn->status) {
        //http头缓存下来准备输出到外部
        if (conn->response == nullptr) {
            conn->response = std::make_shared<Response>();
        }
        std::string header(bufptr, size * nitems);
        conn->response->headers += header;
        spdlog::debug("{}", header);

        //匹配返回码
        if (conn->headers.size() == 0) {
            std::smatch matched;
            //header = "HTTP/1.1 200 OK";
            if (std::regex_match(header, matched, conn->regCode, std::regex_constants::match_default) && matched.size() == 4) {
                conn->response->ver = matched[1].str();
                conn->response->status = matched[2].str();
                conn->response->msg = matched[3].str();

                if (conn->response->status != "200" && conn->response->status != "206") {
                    return 0;
                }
            }
            else {
                return 0;
            }
            conn->headers.push_back(header);
        }
        else {
            //header完成
            if (header == "\r\n") {
                conn->incoming.onQueryFinished(conn, conn->response);
            }
            else {
                std::smatch matched;
                if (std::regex_match(header, matched, conn->regHeader, 
                    std::regex_constants::match_default) && matched.size() == 3) {
                    std::string key = matched[1].str();
                    std::string value = matched[2].str();
                    if (util::icompare(key, "content-range")) {
                        if (std::regex_match(header, matched, conn->regRange, 
                            std::regex_constants::match_default) && matched.size() == 4) {
                            conn->response->rangeStart = std::atoll(matched[1].str().c_str());
                            conn->response->rangeEnd = std::atoll(matched[2].str().c_str());
                            conn->response->rangeTotal = std::atoll(matched[3].str().c_str());
                        }
                    }
                    else if (util::icompare(key, "content-length")) {
                        conn->response->contentLength = std::atoll(value.c_str());
                    }
                }
                conn->headers.push_back(header);
            }
        }
    }
    return size * nitems;
}