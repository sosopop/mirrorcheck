#include "mirroraccel.h"
#include <stdio.h>
#include <assert.h>
#include <curl/curl.h>
#include <spdlog/spdlog.h>
#ifdef ANDROID
#include <spdlog/sinks/android_sink.h>
#else
#include <spdlog/sinks/stdout_color_sinks.h>
#endif

#ifdef WIN32
#include <crtdbg.h>
#include <stdlib.h>
#include <windows.h>

void exit_handle(void)
{
    assert(_CrtDumpMemoryLeaks() == 0);
}
#endif

int main(int argc, char *argv[])
{
    int i = 0;
#ifdef WIN32
    atexit(exit_handle);
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(269);
#endif
    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%L%$] [%t] %v");

// Set the default logger to file logger
#ifdef ANDROID
    spdlog::set_default_logger(spdlog::android_logger_mt("android", "MirrorAccel"));
#else
    spdlog::set_default_logger(spdlog::stdout_color_mt("console"));
#endif

    mirror_accel_init();
    int port = mirror_accel_create("0.0.0.0:7777",
                                   "{"
                                   "\"targets\":["
                                   "{"
                                   "\"url\":\"http://42.51.202.14/xszr.mp4?id=1\""
                                   "},"
                                   "{"
                                   "\"url\":\"http://42.51.202.14/xszr.mp4?id=2\""
                                   "},"
                                   "{"
                                   "\"url\":\"http://42.51.202.14/xszr.mp4?id=3\""
                                   "},"
                                   "{"
                                   "\"url\":\"http://42.51.202.14/xszr.mp4?id=4\""
                                   "},"
                                   "{"
                                   "\"url\":\"http://42.51.202.14/xszr.mp4?id=5\""
                                   "}"
                                   "]"
                                   "}");
    if (port > 0)
    {
        spdlog::info("listening on port: {} \npress ENTER key to shutdown this service", port);
        getchar();
        mirror_accel_destroy(port);
    }
    mirror_accel_uninit();
    return 0;
}