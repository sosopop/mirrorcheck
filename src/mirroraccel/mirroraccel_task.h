#ifndef MIRRORACCEL_TASK_H_
#define MIRRORACCEL_TASK_H_

#include "mongoose.h"

namespace mirroraccel
{
    class ConnOutgoing;
    struct Task
    {
        Task(ConnOutgoing* conn, std::int64_t rangeStart, std::int64_t rangeLen)
            :conn(conn), rangeStart(rangeStart), rangeLen(rangeLen) {
        }
        std::int64_t rangeStart = 0;
        std::int64_t rangeLen = 0;
        std::int64_t tail = 0;
        ConnOutgoing* conn = nullptr;
    };
} // namespace mirroraccel
#endif