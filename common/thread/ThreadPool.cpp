//
// Created by 张锦 on 2023/2/22.
//

#include "ThreadPool.h"

#include <memory>

void ThreadPool::start(int threadCount) {
    m_running = true;
    for (int i = 0; i < threadCount; ++i) {
        auto p = std::make_shared<std::thread>([this](){
            while (m_running) {
                func_type f;
                if (m_safeQueue.pop(f)) {
                    f();
                }
            }
        });
        m_threads.push_back(p);
    }
}

void ThreadPool::stop() {
    m_running = false;
    m_safeQueue.close();
    for (auto t : m_threads) {
        t->join();
    }
    m_threads.clear();
}

void ThreadPool::exec(ThreadPool::func_type f) {
    m_safeQueue.push(f);
}
