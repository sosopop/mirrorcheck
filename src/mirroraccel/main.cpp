#include "mirroraccel.h"
#include <stdio.h>
#include <assert.h>
#include <curl/curl.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

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
    //_CrtSetBreakAlloc(269);
#endif
    spdlog::set_level(spdlog::level::debug); 
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%L%$] [%t] %v");

    // Set the default logger to file logger
    spdlog::set_default_logger(spdlog::stdout_color_mt("console"));


    mirror_accel_init();
    int port = mirror_accel_create("0.0.0.0:7777",
    "{"\
        "\"targets\":["\
            "{"\
                "\"url\":\"http://www.dolit.cn/download/f/bt/DLBT_SDK_3.7.9_2019.zip?id=1\""\
            "},"\
            "{"\
                " \"url\":\"http://www.dolit.cn/download/f/bt/DLBT_SDK_3.7.9_2019.zip?id=2\""\
            "},"\
            "{"\
                " \"url\":\"http://www.dolit.cn/download/f/bt/DLBT_SDK_3.7.9_2019.zip?id=3\""\
            "},"\
            "{"\
                " \"url\":\"http://www.dolit.cn/download/f/bt/DLBT_SDK_3.7.9_2019.zip?id=4\""\
            "},"\
            "{"\
                " \"url\":\"http://www.dolit.cn/download/f/bt/DLBT_SDK_3.7.9_2019.zip?id=5\""\
            "}"\
        "]"\
    "}"
    );
    if (port > 0) {
        spdlog::info("listening on port: {} \npress ENTER key to shutdown this service", port);
        getchar();
        mirror_accel_destroy(port);
    }
    mirror_accel_uninit();
    return 0;
}