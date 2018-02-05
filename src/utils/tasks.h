#ifndef __TASKS_H__
#define __TASKS_H__


class Task {
public:
	Task(){}
	~Task(){}

	void run() {
		func();
	}

	std::function<void()> func;

};









#endif // __TASK_H__