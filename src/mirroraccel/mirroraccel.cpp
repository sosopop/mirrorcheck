#include "mirroraccel_servermgr.h"
#include <iostream>
#include <spdlog/spdlog.h>

static mirroraccel::ServerMgr* sm = nullptr;

void mirror_accel_init()
{
    sm = new mirroraccel::ServerMgr();
}

void mirror_accel_uninit()
{
    if (sm) {
        delete sm;
    }
}

int mirror_accel_create(const char *addr, const char* json_opt)
{
    try
    {
        return sm->create(addr, json_opt);
    }
    catch (std::exception& e)
    {
        spdlog::error("{}", e.what());
    }
    return 0;
}

void mirror_accel_destroy(int port)
{
    sm->destroy(port);
}