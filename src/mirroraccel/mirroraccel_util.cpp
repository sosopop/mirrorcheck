#include "mirroraccel_util.h"

int mirroraccel::util::startWith(const struct mg_str *str1, const char *str2)
{
    size_t i = 0;
    while (str2[i] && i < str1->len && str2[i] == str1->p[i])
        i++;
    return str2[i];
}
