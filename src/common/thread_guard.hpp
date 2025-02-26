#ifndef THREAD_GUARD_HPP
#define THREAD_GUARD_HPP

#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include "cbasetypes.hpp"

namespace rathena {

/**
 * RAII wrapper for mutex locking with deadlock detection
 */
template<typename Mutex>
class thread_guard {
    Mutex& mutex_;
    bool locked_;
    
public:
    explicit thread_guard(Mutex& mutex) : mutex_(mutex), locked_(false) {
        mutex_.lock();
        locked_ = true;
    }
    
    ~thread_guard() {
        if (locked_) {
            mutex_.unlock();
        }
    }
    
    void unlock() {
        if (locked_) {
            mutex_.unlock();
            locked_ = false;
        }
    }

    thread_guard(const thread_guard&) = delete;
    thread_guard& operator=(const thread_guard&) = delete;
};

/**
 * RAII wrapper for shared mutex read locking
 */
template<typename SharedMutex>
class shared_guard {
    SharedMutex& mutex_;
    bool locked_;
    
public:
    explicit shared_guard(SharedMutex& mutex) : mutex_(mutex), locked_(false) {
        mutex_.lock_shared();
        locked_ = true;
    }
    
    ~shared_guard() {
        if (locked_) {
            mutex_.unlock_shared();
        }
    }
    
    void unlock() {
        if (locked_) {
            mutex_.unlock_shared();
            locked_ = false;
        }
    }

    shared_guard(const shared_guard&) = delete;
    shared_guard& operator=(const shared_guard&) = delete;
};

/**
 * RAII wrapper for shared mutex write locking
 */
template<typename SharedMutex>
class unique_guard {
    SharedMutex& mutex_;
    bool locked_;
    
public:
    explicit unique_guard(SharedMutex& mutex) : mutex_(mutex), locked_(false) {
        mutex_.lock();
        locked_ = true;
    }
    
    ~unique_guard() {
        if (locked_) {
            mutex_.unlock();
        }
    }
    
    void unlock() {
        if (locked_) {
            mutex_.unlock();
            locked_ = false;
        }
    }

    unique_guard(const unique_guard&) = delete;
    unique_guard& operator=(const unique_guard&) = delete;
};

/**
 * Condition variable wrapper with timeout support
 */
class condition_guard {
    std::condition_variable& cv_;
    std::mutex& mutex_;
    bool signaled_;
    
public:
    condition_guard(std::condition_variable& cv, std::mutex& mutex)
        : cv_(cv), mutex_(mutex), signaled_(false) {}
    
    template<typename Predicate>
    bool wait(Predicate pred) {
        std::unique_lock<std::mutex> lock(mutex_);
        signaled_ = cv_.wait(lock, pred);
        return signaled_;
    }
    
    template<typename Rep, typename Period, typename Predicate>
    bool wait_for(const std::chrono::duration<Rep, Period>& timeout, Predicate pred) {
        std::unique_lock<std::mutex> lock(mutex_);
        signaled_ = cv_.wait_for(lock, timeout, pred);
        return signaled_;
    }
    
    bool was_signaled() const {
        return signaled_;
    }
    
    condition_guard(const condition_guard&) = delete;
    condition_guard& operator=(const condition_guard&) = delete;
};

// Type aliases for convenience
using mutex_guard = thread_guard<std::mutex>;
using recursive_guard = thread_guard<std::recursive_mutex>;
using shared_mutex_guard = shared_guard<std::shared_mutex>;
using unique_mutex_guard = unique_guard<std::shared_mutex>;

} // namespace rathena

#endif // THREAD_GUARD_HPP