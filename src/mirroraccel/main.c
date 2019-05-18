#include "mirroraccel.h"
#include <stdio.h>
#include <assert.h>
#include <curl/curl.h>

#ifdef WIN32
#include <crtdbg.h>
#include <stdlib.h>
#include <windows.h>

void exit_handle(void)
{
    assert(_CrtDumpMemoryLeaks() == 0);
}
#endif

int main(int argc, char *argv[]) {
    int i = 0;
#ifdef WIN32
    atexit(exit_handle);
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif
    mirror_accel_init();
    int port = mirror_accel_create("0.0.0.0:0");
    if (port > 0) {
        printf("listening on port: %d \npress ENTER key to shutdown this service\n", port);
        getchar();
        mirror_accel_destroy(port);
    }
    mirror_accel_uninit();
    return 0;
}