#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue
{
private:
    mutable std::mutex mutex;
    std::queue<T> queue;
    std::condition_variable cond_var;

public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue &other) = delete;
    ThreadSafeQueue &operator=(const ThreadSafeQueue &other) = delete;

    void push(T value)
    {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(std::move(value));
        cond_var.notify_one();
    }

    bool try_pop(T &value)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (queue.empty())
        {
            return false;
        }
        value = std::move(queue.front());
        queue.pop();
        return true;
    }

    void wait_and_pop(T &value)
    {
        std::unique_lock<std::mutex> lock(mutex);
        cond_var.wait(lock, [this]
                      { return !queue.empty(); });
        value = std::move(queue.front());
        queue.pop();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }
};