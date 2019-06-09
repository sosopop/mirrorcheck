#ifndef MIRRORACCEL_TASK_H_
#define MIRRORACCEL_TASK_H_

#include<chrono>
#include <mutex>
#include "mongoose.h"

namespace mirroraccel
{
    enum {
        TASK_INIT_SIZE = 32 * 1024,
        TASK_DATA_SIZE = 1024 * 1024
    };
    class ConnOutgoing;
    struct Task
    {
        Task(std::int64_t rangeStart, std::int64_t rangeSize)
            : rangeStart(rangeStart), rangeSize(rangeSize) {
            mbuf_init(&buffer, TASK_INIT_SIZE);
        }
        bool operator < (const Task& task) const
        {
            return this->rangeStart < task.rangeStart;
        }
        void read(mbuf& buffer) {
            std::lock_guard<std::mutex> lock(bufferMux);
            buffer = this->buffer;
            mbuf_init(&this->buffer, TASK_INIT_SIZE);
        }
        size_t write(char* data, size_t len) {
            std::lock_guard<std::mutex> lock(bufferMux);
            mbuf_append(&buffer, data, len);
            rangeCurSize += len;
            return buffer.len;
        }
        size_t size() {
            std::lock_guard<std::mutex> lock(bufferMux);
            size_t len = buffer.len;
            return len;
        }
        std::int64_t rangeStart = 0;
        std::int64_t rangeSize = 0;
        std::int64_t rangeCurSize = 0;
        //开始时间戳
        std::int64_t startTime = std::chrono::steady_clock::now().time_since_epoch().count();
        mbuf buffer;
        std::mutex bufferMux;
    };
} 
// namespace mirroraccel

#endif