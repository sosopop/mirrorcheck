#ifndef MIRRORACCEL_UTIL_H_
#define MIRRORACCEL_UTIL_H_

#include "mongoose.h"
#include <string>
#include <cctype>

namespace mirroraccel
{
	namespace util
	{
        int startWith(const struct mg_str *str1, const char *str2);
        bool icompare(std::string const& a, std::string const& b);
	};
}
#endif