#ifndef MIRRORACCEL_INTER_H_
#define MIRRORACCEL_INTER_H_

#include "mirroraccel.h"
#include "mongoose.h"
#include "tree.h"

/**
 * @brief 全局服务表
 * 
 */
struct ma_srv_s
{
    struct mg_mgr mgr;
    int port;
    RB_ENTRY(ma_srv_s) tree_entry;
};

RB_HEAD(ma_srv_tree_s, ma_srv_s);

int ma_srv_compare(struct ma_srv_s *a, struct ma_srv_s *b);

RB_PROTOTYPE(ma_srv_tree_s, ma_srv_s, tree_entry, ma_srv_compare);

#endif