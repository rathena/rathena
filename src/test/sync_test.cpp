#include <gtest/gtest.h>
#include "common/sync.hpp"
#include <vector>
#include <chrono>

using namespace rathena;

class SyncTest : public ::testing::Test {
protected:
    static constexpr size_t NUM_THREADS = 4;
    static constexpr size_t NUM_ITERATIONS = 1000;

    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(SyncTest, SyncQueuePushPop) {
    sync_queue<int> queue(100);
    std::atomic<int> sum{0};
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // Create producer threads
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        producers.emplace_back([&queue]() {
            for (size_t j = 0; j < NUM_ITERATIONS; ++j) {
                queue.push(static_cast<int>(j));
            }
        });
    }

    // Create consumer threads
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        consumers.emplace_back([&queue, &sum]() {
            int value;
            while (queue.pop(value)) {
                sum += value;
            }
        });
    }

    // Let producers finish and close queue
    for (auto& producer : producers) {
        producer.join();
    }
    queue.close();

    // Wait for consumers
    for (auto& consumer : consumers) {
        consumer.join();
    }

    // Each value from 0 to NUM_ITERATIONS-1 should have been added NUM_THREADS times
    int expected = (NUM_ITERATIONS * (NUM_ITERATIONS - 1) / 2) * NUM_THREADS;
    EXPECT_EQ(sum, expected);
}

TEST_F(SyncTest, ThreadPoolTaskExecution) {
    thread_pool pool(NUM_THREADS);
    std::atomic<int> counter{0};
    std::vector<std::future<void>> futures;

    // Submit tasks
    for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
        futures.push_back(pool.submit([&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        }));
    }

    // Wait for all tasks to complete
    for (auto& future : futures) {
        future.wait();
    }

    EXPECT_EQ(counter, NUM_ITERATIONS);
}

TEST_F(SyncTest, SpinlockExclusiveAccess) {
    spinlock lock;
    std::atomic<int> counter{0};
    std::vector<std::thread> threads;

    for (size_t i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&lock, &counter]() {
            for (size_t j = 0; j < NUM_ITERATIONS; ++j) {
                std::lock_guard<spinlock> guard(lock);
                counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(counter, NUM_THREADS * NUM_ITERATIONS);
}

TEST_F(SyncTest, RWSpinlockConcurrentReads) {
    rw_spinlock lock;
    std::atomic<int> concurrent_readers{0};
    std::atomic<int> max_concurrent_readers{0};
    std::vector<std::thread> readers;

    // Create reader threads
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        readers.emplace_back([&lock, &concurrent_readers, &max_concurrent_readers]() {
            for (size_t j = 0; j < NUM_ITERATIONS; ++j) {
                lock.lock_shared();
                int current = concurrent_readers.fetch_add(1, std::memory_order_relaxed) + 1;
                max_concurrent_readers.store(std::max(max_concurrent_readers.load(), current),
                                          std::memory_order_relaxed);
                std::this_thread::yield(); // Force potential concurrent reads
                concurrent_readers.fetch_sub(1, std::memory_order_relaxed);
                lock.unlock_shared();
            }
        });
    }

    for (auto& reader : readers) {
        reader.join();
    }

    EXPECT_GT(max_concurrent_readers, 1);
    EXPECT_EQ(concurrent_readers, 0);
}

TEST_F(SyncTest, RWSpinlockWriterExclusion) {
    rw_spinlock lock;
    std::atomic<bool> writer_active{false};
    std::atomic<int> writer_conflicts{0};
    std::vector<std::thread> writers;

    // Create writer threads
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        writers.emplace_back([&lock, &writer_active, &writer_conflicts]() {
            for (size_t j = 0; j < NUM_ITERATIONS; ++j) {
                lock.lock();
                if (writer_active) {
                    writer_conflicts++;
                }
                writer_active = true;
                std::this_thread::yield(); // Force potential conflicts
                writer_active = false;
                lock.unlock();
            }
        });
    }

    for (auto& writer : writers) {
        writer.join();
    }

    EXPECT_EQ(writer_conflicts, 0);
}

TEST_F(SyncTest, ThreadGuardRAII) {
    std::mutex mutex;
    std::atomic<int> counter{0};
    std::vector<std::thread> threads;

    for (size_t i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([&mutex, &counter]() {
            for (size_t j = 0; j < NUM_ITERATIONS; ++j) {
                {
                    thread_guard<std::mutex> guard(mutex);
                    counter++;
                } // Guard should automatically release mutex
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(counter, NUM_THREADS * NUM_ITERATIONS);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}