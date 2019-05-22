#include "mirroraccel_servermgr.h"

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
    return sm->create( addr, json_opt);
}

void mirror_accel_destroy(int port)
{
	sm->destroy(port);
}