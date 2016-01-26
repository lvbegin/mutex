#ifndef CONDITION_VARIABLE_H__
#define CONDITION_VARIABLE_H__

#include <mutex>
#include <condition_variable>
#include <iostream>

namespace std_mutex_extra {

template <typename M>
class condition_variable {
public:
	void wait(std::unique_lock<M> &lock) {
		std::unique_lock<std::mutex> conditionLock(condition_mutex);
		wait(lock, conditionLock);
	}
	void wait(std::unique_lock<M> &lock, std::function<bool()> pred) {
		std::unique_lock<std::mutex> conditionLock(condition_mutex);
		while (!pred()) {
			wait(lock, conditionLock);
			conditionLock.lock();
		}
	}
	void notify_one() {
		std::lock_guard<std::mutex> conditionLock(condition_mutex);
		condition.notify_one();
	}
	void notify_all() {
		std::lock_guard<std::mutex> conditionLock(condition_mutex);
		condition.notify_all();
	}
private:
	std::condition_variable condition;
	std::mutex condition_mutex;

	void wait(std::unique_lock<M> &lock, std::unique_lock<std::mutex> &conditionLock) {
		lock.unlock();
		condition.wait(conditionLock);
		conditionLock.unlock();
		lock.lock();
	}
};

}

#endif
