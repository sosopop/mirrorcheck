#ifndef MIRRORACCEL_TASK_H_
#define MIRRORACCEL_TASK_H_

#include<chrono>
#include <mutex>
#include "mongoose.h"

namespace mirroraccel
{
    class ConnOutgoing;
    struct Task
    {
        Task(std::int64_t rangeStart, std::int64_t rangeSize)
            : rangeStart(rangeStart), rangeSize(rangeSize) {
            mbuf_init(&buffer, 0);
        }
        bool operator < (const Task& task) const
        {
            return this->rangeStart < task.rangeStart;
        }
        std::int64_t rangeStart = 0;
        std::int64_t rangeSize = 0;
        std::int64_t rangeCurSize = 0;
        //开始时间戳
        std::int64_t startTime = std::chrono::steady_clock::now().time_since_epoch().count();
        mbuf buffer;
    };
} 
// namespace mirroraccel

#endif