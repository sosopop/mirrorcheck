#include "mirroraccel_except.h"
#include <exception>

mirroraccel::Except::Except(const std::string &msg) : msg(msg)
{
}

mirroraccel::Except::~Except()
{
}

const char *mirroraccel::Except::what()
{
    return msg.c_str();
}