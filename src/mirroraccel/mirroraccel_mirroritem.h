#ifndef MIRRORACCEL_MIRRORITEM_H_
#define MIRRORACCEL_MIRRORITEM_H_
#include <string>
#include <mutex>

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
    std::mutex mux;
};
} // namespace mirroraccel
#endif