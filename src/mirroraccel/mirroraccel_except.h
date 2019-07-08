#ifndef MIRRORACCEL_EXCEPT_H_
#define MIRRORACCEL_EXCEPT_H_

#include <string>
#include <exception>

namespace mirroraccel
{
class Except : public std::exception
{
public:
    Except(const std::string &msg);
    ~Except();

public:
#ifdef WIN32
    const char *what() const;
#else
    const char *what() const _NOEXCEPT;
#endif

public:
    std::string msg;
};
} // namespace mirroraccel
#endif