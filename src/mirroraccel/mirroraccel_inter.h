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
 * @brief 全局服务表
 */
struct ma_srv_s
{
    struct mg_mgr mgr;
    int port;
    ma_thread_t thd;
    unsigned int stop_signal : 1;
    RB_ENTRY(ma_srv_s)
    tree_entry;
};

RB_HEAD(ma_srv_tree_s, ma_srv_s);

/**
 * @brief 连接类型
 */
typedef enum
{
    MA_CONN_TYPE_VC = 0
} ma_conn_type;

#define MA_CONN_TYPE ma_conn_type type

struct ma_conn_s
{
    MA_CONN_TYPE;
};

/**
 * @brief 镜像连接项
 */
struct ma_mirror_item_s
{
    //镜像资源完整url地址
    char* url;
    //curl
    CURL *curl;
    //链表内部连接
    QUEUE link;
};

/**
 * @brief 加速虚拟连接
 */
struct ma_accel_vir_conn_s
{
    MA_CONN_TYPE;
    //镜像连接项
    QUEUE mirror_items;
};

#endif