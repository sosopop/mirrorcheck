#ifndef MIRRORACCEL_H_
#define MIRRORACCEL_H_

#if defined(WIN32)
#if defined(MA_EXPORTS)
#ifdef __cplusplus
#define MA_EXPORT extern "C" __declspec(dllexport)
#else
#define MA_EXPORT __declspec(dllexport)
#endif
#else /* defined (MA_EXPORTS) */
#ifdef __cplusplus
#define MA_EXPORT extern "C"
#else
#define MA_EXPORT
#endif
#endif
#else /* defined (_WIN32) */
#if defined(MA_EXPORTS)
#ifdef __cplusplus
#define MA_EXPORT extern "C" __attribute__((visibility("default")))
#else
#define MA_EXPORT __attribute__((visibility("default")))
#endif
#else /* defined (MA_EXPORTS) */
#ifdef __cplusplus
#define MA_EXPORT extern "C"
#else
#define MA_EXPORT
#endif
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 多镜像加速服务全局初始化
     * 
     */
    MA_EXPORT void mirror_accel_init();

    /**
     * @brief 多景象加速服务全局卸载
     * 
     */
    MA_EXPORT void mirror_accel_uninit();

    /**
     * @brief 创建镜像加速服务
     * 
     * @param addr 地址，例如："0.0.0.0:0"
     * @return int 监听端口号，返回 <= 0 表示错误，一般为原因为端口被占用
     */
    MA_EXPORT int mirror_accel_create(const char *addr, const char *json_opt);

    /**
     * @brief 销毁镜像加速服务
     * 
     * @param port 镜像加速服务监听的端口
     */
    MA_EXPORT void mirror_accel_destroy(int port);

#ifdef __cplusplus
}
#endif
#endif