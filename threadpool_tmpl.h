// templates act stupid, so I had to create this to make the template work properly from the header

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