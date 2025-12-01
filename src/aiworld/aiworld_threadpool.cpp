/**
 * aiworld_threadpool.cpp
 * Thread Pool Implementation
 */

#include "aiworld_threadpool.hpp"
#include "aiworld_utils.hpp"

namespace aiworld {

ThreadPool& ThreadPool::getInstance() {
    static ThreadPool instance;
    return instance;
}

void ThreadPool::initialize(size_t num_threads) {
    if (initialized_) {
        log_warn("ThreadPool: Already initialized");
        return;
    }
    
    log_info("ThreadPool: Initializing with " + std::to_string(num_threads) + " threads");
    
    for (size_t i = 0; i < num_threads; ++i) {
        workers_.emplace_back(&ThreadPool::workerThread, this);
    }
    
    initialized_ = true;
}

void ThreadPool::shutdown() {
    if (!initialized_) return;
    
    log_info("ThreadPool: Shutting down...");
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }
    
    condition_.notify_all();
    
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
    initialized_ = false;
    
    log_info("ThreadPool: Shutdown complete");
}

void ThreadPool::submit(Task task) {
    if (!initialized_) {
        log_error("ThreadPool: Cannot submit task - pool not initialized");
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.push(std::move(task));
    }
    
    condition_.notify_one();
}

size_t ThreadPool::pendingTasks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tasks_.size();
}

void ThreadPool::workerThread() {
    while (true) {
        Task task;
        
        {
            std::unique_lock<std::mutex> lock(mutex_);
            
            condition_.wait(lock, [this] {
                return stop_ || !tasks_.empty();
            });
            
            if (stop_ && tasks_.empty()) {
                return;
            }
            
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        
        try {
            task();
        } catch (const std::exception& e) {
            log_error("ThreadPool: Exception in worker thread: " + std::string(e.what()));
        } catch (...) {
            log_error("ThreadPool: Unknown exception in worker thread");
        }
    }
}

} // namespace aiworld