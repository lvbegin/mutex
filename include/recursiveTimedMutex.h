#ifndef RECURSIVE_TIMED_MUTEX_H__
#define RECURSIVE_TIMED_MUTEX_H__

#include <recursiveMutexTemplate.h>

#include <mutex>

namespace std_mutex_extra {

class RecursiveTimedMutex : public RecursiveMutexTemplate<std::timed_mutex> {
public:
	RecursiveTimedMutex() = default;
	~RecursiveTimedMutex() = default;

	template<typename Rep, typename Period>
	bool try_lock_for( const std::chrono::duration<Rep, Period>& timeout_duration ) {
		const auto instanceId = RecursiveMutexTemplate<std::timed_mutex>::instanceId;
		auto &recursiveAquire = RecursiveMutexTemplate<std::timed_mutex>::recursiveAquire;
		auto &mutex = RecursiveMutexTemplate<std::timed_mutex>::mutex;

		if (instanceId >= recursiveAquire.size())
			recursiveAquire.resize(instanceId + 1);
		const auto locked = (0 == recursiveAquire[instanceId]) ? mutex.try_lock_for(timeout_duration) : true;
		if (locked)
			recursiveAquire[instanceId]++;
		return locked;
	}
	template<typename Clock, typename Duration>
	bool try_lock_until( const std::chrono::time_point<Clock, Duration>& timeout_time ) {
		const auto instanceId = RecursiveMutexTemplate<std::timed_mutex>::instanceId;
		auto &recursiveAquire = RecursiveMutexTemplate<std::timed_mutex>::recursiveAquire;
		auto &mutex = RecursiveMutexTemplate<std::timed_mutex>::mutex;

		if (instanceId >= recursiveAquire.size())
			recursiveAquire.resize(instanceId + 1);
		const auto locked = (0 == recursiveAquire[instanceId]) ? mutex.try_lock_until(timeout_time) : true;
		if (locked)
			recursiveAquire[instanceId]++;
		return locked;
	}
};

}

#endif
