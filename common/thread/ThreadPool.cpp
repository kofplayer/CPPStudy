//
// Created by 张锦 on 2023/2/22.
//

#include "ThreadPool.h"

#include <memory>
#include <utility>

void ThreadPool::start(int threadCount) {
    m_running = true;
    m_safeQueue.reset();
    if (threadCount == 1) {
        auto p = std::make_shared<std::thread>([this](){
            while (m_running) {
                std::queue<func_type> fs;
                if (m_safeQueue.popAll(fs)) {
                    while(!fs.empty()) {
                        fs.front()();
                        fs.pop();
                    }
                }
            }
        });
        m_threads.push_back(p);
    } else {
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
}

void ThreadPool::stop() {
    m_running = false;
    m_safeQueue.close();
    for (const auto& t : m_threads) {
        t->join();
    }
    m_threads.clear();
}

void ThreadPool::exec(ThreadPool::func_type f) {
    m_safeQueue.push(std::move(f));
}
