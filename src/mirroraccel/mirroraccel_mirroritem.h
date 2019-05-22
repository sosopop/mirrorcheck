#ifndef MIRRORACCEL_MIRRORITEM_H_
#define MIRRORACCEL_MIRRORITEM_H_
#include <string>

namespace mirroraccel
{
    class MirrorItem
    {
    public:
        MirrorItem( const std::string& url );
        ~MirrorItem();
    private:
        std::string url;
    };
}
#endif