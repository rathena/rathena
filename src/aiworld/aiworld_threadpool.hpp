#pragma once
/**
 * aiworld_threadpool.hpp
 * Simple Thread Pool for Async IPC Operations
 * 
 * Provides thread pool with task queue for background IPC processing
 */

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>
#include <atomic>

namespace aiworld {

using Task = std::function<void()>;

/**
 * Thread Pool (Singleton)
 * Manages a pool of worker threads for async task execution
 */
class ThreadPool {
public:
    static ThreadPool& getInstance();
    
    // Initialize pool with specified number of threads
    void initialize(size_t num_threads = 4);
    
    // Shutdown pool and wait for all tasks to complete
    void shutdown();
    
    // Submit task for async execution
    void submit(Task task);
    
    // Get number of pending tasks
    size_t pendingTasks() const;
    
private:
    ThreadPool() : stop_(false), initialized_(false) {}
    ~ThreadPool() { shutdown(); }
    
    void workerThread();
    
    std::vector<std::thread> workers_;
    std::queue<Task> tasks_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    std::atomic<bool> initialized_;
};

} // namespace aiworld