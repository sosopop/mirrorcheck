#ifndef MIRRORACCEL_UTIL_H_
#define MIRRORACCEL_UTIL_H_

#include "mongoose.h"

namespace mirroraccel
{
	namespace util
	{
        int startWith(const struct mg_str *str1, const char *str2);
	};
}
#endif