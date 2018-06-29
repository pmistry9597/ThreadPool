#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <functional>
#include <memory>
#include <thread>

// used to create pools of waiting threads ready to process a "task" which is a function with parameters
// which is useful to avoid overhead from thread creation and destruction
class ThreadPool {
	int thread_count;
	std::queue<std::unique_ptr<std::function<void()>>> tasks; // tasks stored as functions to be executed with arguments binded
	std::mutex task_mu; // protects tasks queue
	std::condition_variable task_cv;// wakes a thread to run a task
	std::atomic<bool> active; // if threadpool has threads waiting to run tasks
	std::vector<std::thread> workers; // all threads so they can be joined or whatever

	std::condition_variable report_cv; // when waiting for no tasks to be pending
	std::atomic<bool> should_report; // true if threads should report whenever tasks are finished
	void worker_role(); // code to be run by the worker threads
public:
	ThreadPool(int thread_count);
	void push_task(const std::function<void()>& newTask); // task to be added to queue in case the user already has their own function object as a task
	template <class Func, class... Args>
		void push_task(Func& func, const Args&... args); // task that is inputted in the form of a function and arguments
	template <class Type>
		void push_tasks(std::function<Type>* taskList, int size); // when user wants to push a list of tanks such as an array or vector
	bool is_active(); // if the threadpool has been shutdown or not
	int threads(); // returns worker count
	void quit(); // order to shutdown thread pool
	void wait_for_quit(); // calls quit() and blocks until all threads have been shutdown
	void wait_for_noPending(); // blocks until no tasks are left in the queue (tasks are either executing or finished)
	~ThreadPool();

	int task_count(); // return number of tasks currently in queue (Try to use this rarely as this can interfere with the worker threads reading from the task queue!)
};

#include "threadpool_tmpl.h"

#endif