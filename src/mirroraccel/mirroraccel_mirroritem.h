#ifndef MIRRORACCEL_MIRRORITEM_H_
#define MIRRORACCEL_MIRRORITEM_H_
#include <string>

namespace mirroraccel
{
class MirrorItem
{
public:
    MirrorItem(const std::string &url);
    ~MirrorItem();

public:
    std::string getUrl();

private:
    std::string url;
};
} // namespace mirroraccel
#endif