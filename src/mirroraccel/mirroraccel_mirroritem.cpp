#include "mirroraccel_mirroritem.h"

mirroraccel::MirrorItem::MirrorItem(const std::string &url) : url(url)
{
}

mirroraccel::MirrorItem::~MirrorItem()
{
}

std::string mirroraccel::MirrorItem::getUrl()
{
    return url;
}