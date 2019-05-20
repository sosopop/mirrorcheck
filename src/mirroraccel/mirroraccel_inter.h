#ifndef MIRRORACCEL_INTER_H_
#define MIRRORACCEL_INTER_H_

#include "mirroraccel.h"
#include "mongoose.h"
#include "curl/curl.h"
#include "frozen.h"
#include "tree.h"
#include "queue.h"
#include "mirroraccel_thread.h"

/**
 * @brief 错误码
 */
#define MA_ERROR_MAP(XX)    \
    XX(OK, u8"success")     \
    XX(FAILED, u8"unknown") \
    XX(MAX, "")

#define MA_ERROR_GEN(n, s) CODE_##n,
typedef enum
{
    MA_ERROR_MAP(MA_ERROR_GEN)
} ma_error_code;
#undef MA_ERROR_GEN

/**
 * @brief 加速服务
 */
struct ma_srv_s
{
    struct mg_mgr mgr;
    //监听端口
    int port;
    //server线程
    ma_thread_t thd_server;
    //mirror线程
    ma_thread_t thd_mirror;
    //停止标识
    unsigned int stop_signal : 1;
    //镜像连接项
    QUEUE mirror_items;
    //服务树节点
    RB_ENTRY(ma_srv_s) tree_entry;
};

RB_HEAD(ma_srv_tree_s, ma_srv_s);

/**
 * @brief 镜像信息
 */
struct ma_mirror_item_s
{
    //加速服务对象
    struct ma_srv_s* srv;
    //镜像资源完整url地址
    char* url;
    //累计请求次数
    int request_times;
    //累计请求大小
    int request_size;
    //累计失败次数
    int request_failed;
    //链表内部连接
    QUEUE link;
};

/**
 * @brief 镜像连接
 */
struct ma_mirror_conn_s
{
    //加速的连接对象
    struct ma_mirror_item_s* mirror;
    //curl
    CURL *curl;
};

#endif