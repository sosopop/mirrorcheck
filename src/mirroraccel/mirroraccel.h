#ifndef MIRRORACCEL_H_
#define MIRRORACCEL_H_

/**
 * @brief 多镜像加速服务全局初始化
 * 
 */
void mirror_accel_init();

/**
 * @brief 多景象加速服务全局卸载
 * 
 */
void mirror_accel_uninit();

/**
 * @brief 创建镜像加速服务
 * 
 * @param addr 地址，例如："0.0.0.0:0"
 * @return int 监听端口号，返回 <= 0 表示错误，一般为原因为端口被占用
 */
int mirror_accel_create( const char* addr, const char* json_opt);

/**
 * @brief 销毁镜像加速服务
 * 
 * @param port 镜像加速服务监听的端口
 */
void mirror_accel_destroy( int port );

#endif