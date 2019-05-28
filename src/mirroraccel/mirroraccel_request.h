#ifndef MIRRORACCEL_REQUEST_H_
#define MIRRORACCEL_REQUEST_H_

#include "mongoose.h"
#include "curl/curl.h"

namespace mirroraccel
{
	class Request
	{
    public:
		Request(struct http_message* hm);
        ~Request();
	private:
		//禁止拷贝
		Request(Request& r) {};
		Request& operator =(Request& r) { return *this; };
    public:
        std::string getUrl( const std::string& base );
        struct curl_slist* getHeaders();
    private:
        std::string host;
        std::string url;
        std::string qs;
        struct curl_slist* headers = nullptr;
	};

    inline Request::Request(struct http_message* hm) :
        url(hm->uri.p, hm->uri.len),
        qs(hm->query_string.p, hm->query_string.len)
    {
        for (int i = 0; hm->header_names[i].len; i++)
        {
            //跳过host，后面需要替换
            if (mg_vcasecmp(&hm->header_names[i], "Host") == 0)
            {
                host.assign(hm->header_values[i].p, hm->header_values[i].len);
                continue;
            }
            //跳过range，后面需要替换
            //if (mg_strncmp(hm->header_names[i], "Range") == 0)
            //{
            //    continue;
            //}

            headers = curl_slist_append(
                headers,
                (std::string(hm->header_names[i].p, hm->header_names[i].len) +
                ":" +
                std::string(hm->header_values[i].p, hm->header_values[i].len)).c_str());
        }
    }

    inline Request::~Request()
    {
        if (headers) {
            curl_slist_free_all(headers);
        }
    }

    inline std::string Request::getUrl(const std::string& base)
    {
        return base + url + "?" + qs;
    }

    inline struct curl_slist* Request::getHeaders()
    {
        return headers;
    }

}
#endif