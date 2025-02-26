#ifndef SYNC_HPP
#define SYNC_HPP

#include "thread_guard.hpp"
#include <atomic>
#include <thread>
#include <future>
#include <queue>
#include <functional>

namespace rathena {

/**
 * Thread-safe queue implementation
 */
template<typename T>
class sync_queue {
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    size_t capacity_;
    std::atomic<bool> closed_{false};

public:
    explicit sync_queue(size_t capacity = std::numeric_limits<size_t>::max())
        : capacity_(capacity) {}

    bool push(T&& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (closed_) return false;
        
        while (queue_.size() >= capacity_ && !closed_) {
            not_full_.wait(lock);
        }
        
        if (closed_) return false;
        
        queue_.push(std::move(item));
        not_empty_.notify_one();
        return true;
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        while (queue_.empty() && !closed_) {
            not_empty_.wait(lock);
        }
        
        if (queue_.empty()) return false;
        
        item = std::move(queue_.front());
        queue_.pop();
        not_full_.notify_one();
        return true;
    }

    bool try_pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty()) return false;
        
        item = std::move(queue_.front());
        queue_.pop();
        not_full_.notify_one();
        return true;
    }

    void close() {
        std::unique_lock<std::mutex> lock(mutex_);
        closed_ = true;
        not_empty_.notify_all();
        not_full_.notify_all();
    }

    bool is_closed() const {
        return closed_;
    }

    size_t size() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.size();
    }

    bool empty() const {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.empty();
    }
};

/**
 * Thread pool implementation
 */
class thread_pool {
    std::vector<std::thread> workers_;
    sync_queue<std::function<void()>> tasks_;
    std::atomic<bool> stopping_{false};

public:
    explicit thread_pool(size_t num_threads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    if (!tasks_.pop(task)) {
                        if (stopping_) break;
                        continue;
                    }
                    task();
                }
            });
        }
    }

    ~thread_pool() {
        stopping_ = true;
        tasks_.close();
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using return_type = std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        
        std::future<return_type> result = task->get_future();
        tasks_.push([task]() { (*task)(); });
        return result;
    }

    size_t size() const { return workers_.size(); }
    size_t queue_size() const { return tasks_.size(); }

    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;
};

/**
 * Spinlock implementation for low-contention scenarios
 */
class spinlock {
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;

public:
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
    }

    bool try_lock() {
        return !flag_.test_and_set(std::memory_order_acquire);
    }

    void unlock() {
        flag_.clear(std::memory_order_release);
    }
};

/**
 * Read-write spinlock implementation
 */
class rw_spinlock {
    std::atomic<int> readers_{0};
    std::atomic_flag write_lock_ = ATOMIC_FLAG_INIT;

public:
    void lock_shared() {
        while (true) {
            int count = readers_.load(std::memory_order_relaxed);
            if (count >= 0 && readers_.compare_exchange_weak(
                count, count + 1, std::memory_order_acquire)) {
                break;
            }
            std::this_thread::yield();
        }
    }

    void unlock_shared() {
        readers_.fetch_sub(1, std::memory_order_release);
    }

    void lock() {
        while (write_lock_.test_and_set(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        int count = 0;
        while (!readers_.compare_exchange_weak(
            count, -1, std::memory_order_acquire)) {
            count = 0;
            std::this_thread::yield();
        }
    }

    void unlock() {
        readers_.store(0, std::memory_order_release);
        write_lock_.clear(std::memory_order_release);
    }
};

} // namespace rathena

#endif // SYNC_HPP