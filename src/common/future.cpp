#include "future.hpp"
#include "jobqueue.hpp"

JobQueue<futureJob> futureJobs;

// Threads here
// * Main Thread
// * Another Thread

// Another Thread Function (mostly)
// (but could be called by main thread also)
void add_future(futureJobFunc callbackFunc, FutureData resultData) {
	futureJob newjob;

	newjob.callbackFunc = callbackFunc;
	newjob.resultData = resultData;

	futureJobs.push_back(newjob);
}

// Main Thread Function
void do_future(void) {
	futureJobs.Run(
		[] (futureJob &job) {
			job.callbackFunc(job.resultData);
		}
	);
}
