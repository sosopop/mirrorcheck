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
    const char *what() const _NOEXCEPT;

public:
    std::string msg;
};
} // namespace mirroraccel
#endif