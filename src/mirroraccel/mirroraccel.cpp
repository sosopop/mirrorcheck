#include "mirroraccel.h"
#include "mirroraccel_servermgr.h"
#include <iostream>
#include <spdlog/spdlog.h>

static mirroraccel::ServerMgr *sm = nullptr;

MA_EXPORT void mirror_accel_init()
{
    sm = new mirroraccel::ServerMgr();
}

MA_EXPORT void mirror_accel_uninit()
{
    if (sm)
    {
        delete sm;
    }
}

MA_EXPORT int mirror_accel_create(const char *addr, const char *json_opt)
{
    try
    {
        return sm->create(addr, json_opt);
    }
    catch (std::exception &e)
    {
        spdlog::error("{}", e.what());
    }
    return 0;
}

MA_EXPORT void mirror_accel_destroy(int port)
{
    sm->destroy(port);
}