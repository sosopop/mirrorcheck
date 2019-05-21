#ifndef MIRRORACCEL_SERVERMGR_H_
#define MIRRORACCEL_SERVERMGR_H_

#include "mirroraccel.h"

namespace mirroraccel
{
class ServerMgr
{
public:
    ServerMgr();
    ~ServerMgr();
public:
    void init();
    void uninit();
};
}
#endif