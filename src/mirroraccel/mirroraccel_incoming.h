#ifndef MIRRORACCEL_INTER_H_
#define MIRRORACCEL_INTER_H_

#include "mirroraccel.h"

class MirrorAccelMgr
{
public:
    MirrorAccelMgr();
    ~MirrorAccelMgr();
public:
    void init();
    void uninit();
};

#endif