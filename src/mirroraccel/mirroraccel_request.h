#ifndef MIRRORACCEL_REQUEST_H_
#define MIRRORACCEL_REQUEST_H_

#include "mongoose.h"
#include "curl/curl.h"

namespace mirroraccel
{
class Request
{
public:
    Request(struct http_message *hm);
    ~Request();

private:
    //禁止拷贝
    Request(Request &r) = delete;
    Request &operator=(Request &r) = delete;

public:
    std::string getUrl();
    struct curl_slist *getHeaders();
	void reset(struct http_message *hm);

private:
    std::string host;
    std::string url;
    std::string qs;
    struct curl_slist *headers = nullptr;
};

inline Request::Request(struct http_message *hm) 
{
	reset(hm);
}

inline Request::~Request()
{
    if (headers)
    {
        curl_slist_free_all(headers);
    }
}

inline std::string Request::getUrl()
{
    if (qs.empty())
        return url;
    return url + "?" + qs;
}

inline struct curl_slist *Request::getHeaders()
{
    return headers;
}

inline void Request::reset(http_message * hm)
{
	url.assign(hm->uri.p, hm->uri.len);
	qs.assign(hm->query_string.p, hm->query_string.len);

	for (int i = 0; hm->header_names[i].len; i++)
	{
		//跳过host，后面需要替换
		if (mg_vcasecmp(&hm->header_names[i], "Host") == 0)
		{
			host.assign(hm->header_values[i].p, hm->header_values[i].len);
			continue;
		}

		headers = curl_slist_append(
			headers,
			(std::string(hm->header_names[i].p, hm->header_names[i].len) +
				":" +
				std::string(hm->header_values[i].p, hm->header_values[i].len))
			.c_str());
	}
}

} // namespace mirroraccel
#endif