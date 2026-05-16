#ifndef POOL_H
#define POOL_H

#include "server.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount = 4);
    ~ThreadPool();

    void submit(std::function<void()> task);
    void shutdown();

private:
    void workerLoop();

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queueMutex_;
    std::condition_variable condition_;
    std::atomic<bool> stopping_{false};
};

#endif // POOL_H
