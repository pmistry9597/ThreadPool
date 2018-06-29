#include "threadpool.h"

void ThreadPool::worker_role() { // to be run by worker threads
	while (active) {
		std::unique_lock<std::mutex> task_guard(task_mu);
		while (active && tasks.empty()) { // if spurious wakeup, while loop will keep it waiting until something appears; active check so thread can quit when supposed to
			task_cv.wait(task_guard);
		}
		if (!active) {
			return;
		}
		// remove task from queue to prevent it being run again
		auto pTask = std::move(tasks.front());
		tasks.pop();
		if (should_report && tasks.empty()) report_cv.notify_all();
		task_guard.unlock(); // unlock so other threads can read from the queue

		// execute task (finally!)
		pTask->operator()();
	}
}
ThreadPool::ThreadPool(int thread_count) {
	active = true;
	should_report = false;
	this->thread_count = thread_count;
	for (int i = 0; i < thread_count; i++) {
		std::thread worker(&ThreadPool::worker_role, this);
		workers.push_back(std::move(worker));
	}
}
void ThreadPool::push_task(const std::function<void()>& newTask) {
	if (!active) { // dont add a task if not even active
		return;
	}
	std::lock_guard<std::mutex> task_guard(task_mu);
	std::unique_ptr<std::function<void()>> pTask(new std::function<void()>(newTask)); // heap function object to prevent excessive copying if the function is big
	tasks.push(std::move(pTask));
	task_cv.notify_one(); // wakeup a thread to read the task
}
template <class Func, class... Args>
void ThreadPool::push_task(Func& func, const Args&... args) {
	if (!active) {
		return;
	}
	std::lock_guard<std::mutex> task_guard(task_mu);
	// bind args to a function object in the heap
	std::unique_ptr<std::function<void()>> pTask(new std::function<void()>(std::bind(func, args...)));
	tasks.push(std::move(pTask));
	task_cv.notify_one(); // wakeup a thread to read the task
}
template <class Type>
void ThreadPool::push_tasks(std::function<Type>* tasksList, int size) { // push an array of tasks and then wakeup threads after
	std::lock_guard<std::mutex> task_lock(task_mu);
	for (int i = 0; i < size; i++) { // push all tasks first
		std::unique_ptr<std::function<void()>> pTask(new std::function<void()>(tasksList[i]));
		tasks.push(std::move(pTask));
	} // now wakeup threads to read tasks
	for (int i = 0; i < size; i++) {
		task_cv.notify_one();
	}
}
bool ThreadPool::is_active() {
	return active;
}
int ThreadPool::threads() {
	return thread_count;
}
void ThreadPool::quit() {
	if (!active) { // dont bother if not active
		return;
	}
	active = false;
	task_cv.notify_all();
}
void ThreadPool::wait_for_quit() {
	quit();
	for (auto it = workers.begin(); it != workers.end(); it++) { // join all threads so when function ends no threads are left over
		(*it).join();
	}
}
// wait until no tasks are left in the queue which means all tasks are finished or executing
void ThreadPool::wait_for_noPending() {
	std::unique_lock<std::mutex> task_lock(task_mu);
	should_report = true;
	while (!tasks.empty()) {
		report_cv.wait(task_lock);
	}
	should_report = false;
}
ThreadPool::~ThreadPool() {
	// will destroy all threads before this object is destroyed
	wait_for_quit();
}

int ThreadPool::task_count() {
	std::lock_guard<std::mutex> lg(task_mu);
	return tasks.size();
}