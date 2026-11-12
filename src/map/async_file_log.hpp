// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef ASYNC_FILE_LOG_HPP
#define ASYNC_FILE_LOG_HPP

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdio>
#include <deque>
#include <mutex>
#include <string>
#include <thread>

// The map thread owns start, append and stop. The worker only sees
// copies of the log path and completed lines.
class AsyncFileLog {
public:
	~AsyncFileLog() {
		stop();
	}

	void start( const char* path ) {
		path_ = path;
		failed_batches_.store( 0, std::memory_order_relaxed );
		worker_ = std::thread( &AsyncFileLog::write, this );
	}

	std::size_t failures() const {
		return failed_batches_.load( std::memory_order_relaxed );
	}

	bool append( const char* line ) {
		if( !worker_.joinable() )
			return false;

		{
			std::unique_lock<std::mutex> lock( mutex_ );
			// Apply backpressure to preserve FIFO order and avoid losing logs.
			space_.wait( lock, [this] { return pending_.size() < capacity_; } );
			pending_.emplace_back( line );
		}
		ready_.notify_one();
		return true;
	}

	std::size_t stop() {
		if( worker_.joinable() ) {
			{
				std::lock_guard<std::mutex> lock( mutex_ );
				stopping_ = true;
			}
			ready_.notify_one();
			worker_.join();
			stopping_ = false;
		}
		return failures();
	}

private:
	void write() {
		std::deque<std::string> batch;
		for( ;; ) {
			{
				std::unique_lock<std::mutex> lock( mutex_ );
				ready_.wait( lock, [this] { return stopping_ || !pending_.empty(); } );
				if( pending_.empty() )
					return;
				batch.swap( pending_ );
			}
			space_.notify_one();

			// Close after each batch so future writes reopen a rotated log.
			FILE* file = std::fopen( path_.c_str(), "a" );
			bool failed = file == nullptr;
			if( file != nullptr ) {
				for( const auto& line : batch ) {
					if( std::fputs( line.c_str(), file ) == EOF )
						failed = true;
				}
				if( std::fclose( file ) != 0 )
					failed = true;
			}
			if( failed )
				failed_batches_.fetch_add( 1, std::memory_order_relaxed );
			batch.clear();
		}
	}

	static constexpr std::size_t capacity_ = 1024;
	std::string path_;
	std::deque<std::string> pending_;
	std::mutex mutex_;
	std::condition_variable ready_;
	std::condition_variable space_;
	std::thread worker_;
	bool stopping_ = false;
	std::atomic<std::size_t> failed_batches_{ 0 };
};

#endif /* ASYNC_FILE_LOG_HPP */
