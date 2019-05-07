#ifndef MIRRORACCEL_INTER_H_
#define MIRRORACCEL_INTER_H_

#include "mirroraccel.h"
#include "mongoose.h"
#include "tree.h"
#include "mirroraccel_thread.h"

/**
 * @brief 全局服务表
 */
struct ma_srv_s
{
    struct mg_mgr mgr;
    int port;
    ma_thread_t thd;
    unsigned int stop_signal:1;
    RB_ENTRY(ma_srv_s) tree_entry;
};

RB_HEAD(ma_srv_tree_s, ma_srv_s);

#endif