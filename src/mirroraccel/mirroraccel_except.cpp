#include "mirroraccel_except.h"
#include <exception>

mirroraccel::Except::Except(const std::string &msg) : msg(msg)
{
}

mirroraccel::Except::~Except()
{
}

#ifdef WIN32
const char* mirroraccel::Except::what() const
#else
const char* mirroraccel::Except::what() const _NOEXCEPT
#endif
{
    return msg.c_str();
}