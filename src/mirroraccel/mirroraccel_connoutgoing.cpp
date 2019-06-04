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
    spdlog::debug("new connection outgoing, {}", mirror->getUrl());
    query();
}

mirroraccel::ConnOutgoing::~ConnOutgoing()
{
    stop(false);
    spdlog::debug("connection outgoing disconnected, {}", mirror->getUrl());
}

void mirroraccel::ConnOutgoing::stop(bool reset)
{
    headers.clear();
    curl_multi_remove_handle(incoming.handle(), curl);
    if (reset) {
        curl_easy_reset(curl);
    }
    else {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
    status = ST_STOPED;
}

void mirroraccel::ConnOutgoing::end(CURLcode code)
{
    if (code != CURLE_OK) {
        if (ST_QUERY == status) {
            status = ST_QUERY_ERROR;
        }
    }
}

mirroraccel::ConnOutgoing::Status mirroraccel::ConnOutgoing::getStatus()
{
    return status;
}

std::shared_ptr<mirroraccel::Response> mirroraccel::ConnOutgoing::getResponse()
{
    return response;
}

void mirroraccel::ConnOutgoing::query()
{
    if (!curl) {
        curl = curl_easy_init();
    }
    else {
        stop(true);
    }

    status = ST_QUERY;
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

void mirroraccel::ConnOutgoing::request(std::pair<std::int64_t, std::int64_t> blockRange)
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
        try
        {
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
                        throw 0;
                    }
                }
                else {
                    throw 0;
                }
                conn->headers.push_back(header);
            }
            else {
                //header完成
                if (header == "\r\n") {
                    conn->status = ST_QUERY_END;
                    if ( !conn->incoming.onQueryEnd(conn, conn->response))
                    {
                        return 0;
                    }
                }
                else {
                    std::smatch matched;
                    if (std::regex_match(header, matched, conn->regHeader,
                        std::regex_constants::match_default) && matched.size() == 3) {
                        std::string key = matched[1].str();
                        std::string value = matched[2].str();
                        if (util::icompare(key, "content-range")) {
                            if (std::regex_match(value, matched, conn->regRange,
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
        catch (int e)
        {
            conn->status = ST_QUERY_ERROR;
            return e;
        }
    }
    return size * nitems;
}