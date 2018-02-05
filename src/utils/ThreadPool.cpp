#include "ThreadPool.h"


ThreadPool::ThreadPool(int threadNum):
	size(threadNum) 
{
	for(int ii = 0; ii < size; ii++)
	{  
		this->workers.push_back(std::thread([this]{this->workerFunction();}));
	}
}

void ThreadPool::addTask(Task* task) {
	{
		std::unique_lock<std::mutex> lock(enqueMutex);
		tasks.emplace_back(task);
	}
	condition.notify_one();
}

void ThreadPool::workerFunction() {
	Task* task = nullptr;
	while(true) {
        {
            std::unique_lock<std::mutex> lock(enqueMutex);

            condition.wait(lock, [this]{return !this->tasks.empty();});
            task = tasks.front();
            tasks.pop_front();
        }
        if(task != nullptr) task->run();
    }
}