#pragma once

#include <mutex>
#include <vector>
#include <functional>

// JobQueue Class Template
// definitions
// <JobType> job : a data stored in the queue vector.
// queue : first in first out vector

template <typename JobType>
class JobQueue {
	// <JobType>job queue, first in first out.
	std::vector<JobType> queue;
	std::mutex mt;

public:
	// Pops out <JobType>jobs from vector queue and run them
	void Run(std::function<void (JobType&)> func) {
		// use mutex here to prevent data racing from push_back()
		std::unique_lock<std::mutex> lock(mt);
		if (!queue.size())
			return;

		std::vector<JobType> QueueForRunning(queue.size());
		QueueForRunning.assign(queue.begin(), queue.end());
		queue.clear();
		lock.unlock();

		for (auto& job : QueueForRunning)
			func(job);
	}
	
	// push a new <JobType>job to the queue
	void push_back(JobType v) {
		// use mutex here to prevent data racing Run()
		std::unique_lock<std::mutex> lock(mt);
		queue.push_back(v);
	}
};
