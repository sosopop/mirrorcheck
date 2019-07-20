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
        ~Task()
        {
            mbuf_free(&buffer);
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
        size_t writeHeader(char* data, size_t len) {
            std::lock_guard<std::mutex> lock(bufferMux);
            mbuf_append(&buffer, data, len);
            return buffer.len;
        }
        size_t writeData(char* data, size_t len) {
            std::lock_guard<std::mutex> lock(bufferMux);

            std::int64_t pos = rangeStart + rangeCurSize;
            for (std::int64_t i = 0; i < len; i++)
            {
                std::int64_t c = pos + i + 1;
                c = c ^ (c << 1) ^ (c << 2) ^ (c << 4) ^ (c << 6) ^ (c >> 1) ^ (c >> 2) ^ (c >> 4) ^ (c >> 6) ^ (c >> 12);
                data[i] = data[i] ^ (unsigned char)(c) ^ "hello"[(pos + i) % 5];
            }

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