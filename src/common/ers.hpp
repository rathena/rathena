/***\
 *  Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
 *  For more information, see LICENCE in the main folder
 *
 *  Entry Reuse System (ERS) — C++ RAII Implementation
 *
 *  This header provides a modern, typed, header‑only object pool for
 *  fixed‑size entries via the template `ERS<T>`. It replaces the legacy
 *  C‑style ERS (struct eri + function table) with a simpler RAII API while
 *  preserving the key property of reusing memory to reduce allocations.
 *
 *  Design:
 *  - Typed pool per `T`: Each `ERS<T>` instance manages pages of `T` and a
 *    freelist of returned objects. Objects are constructed in-place and must
 *    be returned to the same pool.
 *  - RAII: Pool pages are released when the `ERS<T>` instance is destroyed.
 *  - Not thread‑safe: The pool does not synchronize access. If used across
 *    threads, callers must provide their own synchronization or use
 *    per‑thread pools.
 *  - DISABLE_ERS: When defined, the pool becomes a thin wrapper over
 *    `new`/`delete` (no pooling), useful for debugging or quick bisects.
 *
 *  Advantages:
 *  - Fewer allocations/frees → better performance, less fragmentation.
 *  - Simple and explicit lifecycle (create/destroy), automatic cleanup.
 *  - Adjustable chunk size to tune allocation bursts.
 *
 *  Trade‑offs:
 *  - Not thread‑safe; callers must guard cross‑thread use.
 *  - May hold extra free capacity until the pool is destroyed.
 *
 *  API (summary):
 *    ERS<T> pool("name", chunk);
 *    T* p = pool.alloc(args...);
 *    pool.free(p);
 *    pool.setChunkSize(n); // or set_chunk_size(n)
 *    pool.entry_size();
 *    // destructor calls finalize() automatically
 *
 *  This header intentionally does not provide global reports or a global
 *  shutdown hook (ers_report/ers_final). Those can be built externally if
 *  needed, but the default is to keep the pool local and RAII‑managed.
 *
 *  WARNING: Not thread‑safe.
\***/
// Modern C++ RAII-based object pool for fixed-size entries
#ifndef ERS_HPP
#define ERS_HPP

#include "cbasetypes.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <new>
#include <utility>

/*
 * Public knobs:
 *  - DISABLE_ERS: Define to disable pooling (falls back to new/delete).
 *  - ERS<T>:      Typed pool with RAII lifetime and adjustable chunk size.
 */

// Define this to disable pooling and use new/delete under the same API.
//#define DISABLE_ERS

// Legacy ERSOptions have been removed. Behavior is adjusted via methods.

#ifdef DISABLE_ERS
// Disabled mode: fall back to plain new/delete
template <typename T>
class ERS {
public:
    explicit ERS(const char* /*name*/, uint32 /*chunk*/ = 64) {}
    ~ERS() {}

    template <typename... Args>
    T* alloc(Args&&... args) { return new T(std::forward<Args>(args)...); }
    void free(T* obj) { delete obj; }

    void setChunkSize(uint32) {}
    std::size_t entry_size() const { return sizeof(T); }
    void finalize() {}

    ERS(const ERS&) = delete;
    ERS& operator=(const ERS&) = delete;
};
#else
// Enabled mode: pooled allocation with page + freelist
template <typename T>
class ERS {
	const char* name_;
public:
    explicit ERS(const char* name, uint32 chunk = 64) : name_(name), chunkSize_(chunk) {}
    ~ERS() { finalize(); }

    template <typename... Args>
    T* alloc(Args&&... args) {
        if (freeList_.empty()) allocateChunk();
        T* slot = freeList_.back();
        freeList_.pop_back();
        return new (slot) T(std::forward<Args>(args)...);
    }
    void free(T* obj) {
        if (!obj) return;
        obj->~T();
        freeList_.push_back(obj);
    }

    void setChunkSize(uint32 n) { if (n > 0) chunkSize_ = n; }
    std::size_t entry_size() const { return sizeof(T); }

    void finalize() {
        for (auto page : pages_) ::operator delete(static_cast<void*>(page));
        pages_.clear();
        freeList_.clear();
    }

    ERS(const ERS&) = delete;
    ERS& operator=(const ERS&) = delete;

private:
    void allocateChunk() {
        T* page = static_cast<T*>(::operator new(sizeof(T) * chunkSize_));
        pages_.push_back(page);
        for (std::size_t i = 0; i < chunkSize_; ++i)
            freeList_.push_back(page + i);
    }

    std::size_t chunkSize_;
    std::vector<T*> pages_;
    std::vector<T*> freeList_;
};
#endif

#endif /* ERS_HPP */
