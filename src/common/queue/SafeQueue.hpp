//
// Created by 张锦 on 2023/2/22.
//

#pragma once

#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>

template<typename T>
class SafeQueue
{
private:
    std::queue<T> m_queue; //利用模板函数构造队列
    mutable  std::mutex m_mutex; // 访问互斥信号量
    mutable  std::condition_variable m_dataCond;
    int m_maxSize = 10;
    bool m_close =  false;
    std::function<void(T)> m_cleanFunc;
public:
    SafeQueue() = default;
    explicit SafeQueue(int size) : m_maxSize(size) {}
    virtual ~SafeQueue() {
        clear();
    }
    void setCleanFunc(std::function<void(T)> f) {
        m_cleanFunc = f;
    }
    bool empty() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }
    int size() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }
    bool push(T data) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_close) return false;
        if(m_maxSize > 0 && m_queue.size() == m_maxSize) {
            return false;
        }
        m_queue.emplace(data);
        m_dataCond.notify_one();
        return true;
    }
    bool pop(T& data) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_close) return false;
        m_dataCond.wait(lock, [this]{return m_close || !this->m_queue.empty();});
        if(m_queue.empty())
        {
            return false;
        }
        else
        {
            data =std::move(m_queue.front());
            m_queue.pop();
            return true;
        }
    }
    bool popAll(std::queue<T>& datas) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_close) return false;
        m_dataCond.wait(lock, [this]{return m_close || !this->m_queue.empty();});
        if(m_queue.empty())
        {
            return false;
        }
        else
        {
            datas =std::move(m_queue);
            return true;
        }
    }
    void clear() {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (!m_queue.empty()) {
            auto data = std::move(m_queue.front());
            m_queue.pop();
            if(m_cleanFunc) {
                m_cleanFunc(data);
            }
        }
    }

    void reset() {
        clear();
        m_close = false;
    }

    void close() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_close = true;
        m_dataCond.notify_all();
    }

};