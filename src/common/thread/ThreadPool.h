//
// Created by 张锦 on 2023/2/22.
//

#pragma once

#include <thread>
#include <vector>
#include "../queue/SafeQueue.hpp"

class ThreadPool {
public:
    using func_type = std::function<void()>;
    ThreadPool() : m_running(false) {}
    virtual ~ThreadPool() = default;
    void start(int threadCount);
    void stop();
    void exec(func_type f);
private:
    bool m_running;
    std::vector<std::shared_ptr<std::thread>> m_threads;
    SafeQueue<func_type> m_safeQueue;
};
