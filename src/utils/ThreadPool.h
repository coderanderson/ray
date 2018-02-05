#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <thread>
#include <queue>
#include "tasks.h"
#include <mutex>
#include <condition_variable>


class ThreadPool {
public:
	ThreadPool(int size);
	~ThreadPool();

	void addTask(Task* task);
	

private:
	void workerFunction();

	std::vector<std::thread> workers;
	std::deque<Task*> tasks;
	std::mutex enqueMutex;
	std::condition_variable condition;
	int size;
};






#endif // __THREADPOOL_H__
