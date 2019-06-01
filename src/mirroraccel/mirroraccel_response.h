#ifndef MIRRORACCEL_RESPONSE_H_
#define MIRRORACCEL_RESPONSE_H_

#include "mongoose.h"
#include "curl/curl.h"

namespace mirroraccel
{
struct Response
{
public:
    std::string ver;
    std::string status;
    std::string msg;
    std::int64_t contentLength = 0;
    std::int64_t rangeStart = 0;
    std::int64_t rangeEnd = 0;
    std::int64_t rangeTotal = 0;
    std::string headers;
};

} // namespace mirroraccel
#endif