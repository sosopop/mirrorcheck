#ifdef ANDROID
#include "mirroraccel.h"
#include "mirroraccel_jni.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/android_sink.h>

/*
 * Class:     com_baiduren_lemontv_module_MirrorAccel
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_baiduren_lemontv_module_MirrorAccel_init(JNIEnv *, jclass)
{
    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%L%$] [%t] %v");

// Set the default logger to file logger
#ifdef ANDROID
    spdlog::set_default_logger(spdlog::android_logger_mt("android", "MirrorAccel"));
#else
    spdlog::set_default_logger(spdlog::stdout_color_mt("console"));
#endif

    spdlog::debug("mirror_accel_init");
    mirror_accel_init();
}

/*
 * Class:     com_baiduren_lemontv_module_MirrorAccel
 * Method:    uninit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_baiduren_lemontv_module_MirrorAccel_uninit(JNIEnv *, jclass)
{
    spdlog::debug("mirror_accel_uninit");
    mirror_accel_uninit();
}

/*
 * Class:     com_baiduren_lemontv_module_MirrorAccel
 * Method:    create
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_baiduren_lemontv_module_MirrorAccel_create(JNIEnv *env, jclass, jstring addr, jstring options)
{
    const char *__addr = env->GetStringUTFChars(addr, 0);
    const char *__options = env->GetStringUTFChars(options, 0);
    spdlog::debug("mirror_accel_create {},{}", __addr, __options);
    jint port = 0;
    if (__addr == nullptr || __options == nullptr)
    {
        port = 0;
    }
    else
    {
        port = mirror_accel_create(__addr, __options);
    }
    env->ReleaseStringUTFChars(addr, __addr);
    env->ReleaseStringUTFChars(options, __options);
    return port;
}

/*
 * Class:     com_baiduren_lemontv_module_MirrorAccel
 * Method:    destroy
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_baiduren_lemontv_module_MirrorAccel_destroy(JNIEnv *, jclass, jint port)
{
    spdlog::debug("mirror_accel_destroy");
    mirror_accel_destroy(port);
}

#endif