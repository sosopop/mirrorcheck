#include "mirroraccel_util.h"

int mirroraccel::util::startWith(const struct mg_str *str1, const char *str2)
{
    size_t i = 0;
    while (str2[i] && i < str1->len && str2[i] == str1->p[i])
        i++;
    return str2[i];
}

bool mirroraccel::util::icompare(std::string const & a, std::string const & b)
{
    if (a.length() != b.length())
        return false;

    return std::equal(a.begin(), a.end(), b.begin(),
        [](const char& a, const char& b)
    {
        return (std::tolower(a) == std::tolower(b));
    });
}
