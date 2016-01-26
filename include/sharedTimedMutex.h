#ifndef __SHARED_TIMED_MUTEX_H__
#define __SHARED_TIMED_MUTEX_H__

#include <sharedMutexTemplate.h>

namespace std_mutex_extra {

class SharedTimedMutex : public SharedMutexTemplate<std::timed_mutex> {
public:
	SharedTimedMutex() = default;
	~SharedTimedMutex() = default;

	template<typename Rep, typename Period>
	bool try_lock_for( const std::chrono::duration<Rep, Period>& timeout_duration ) {
		const auto lockFunction = [timeout_duration](std::timed_mutex &mutex) { return mutex.try_lock_for(timeout_duration);};
		return try_lock(lockFunction);
	}
	template<typename Clock, typename Duration>
	bool try_lock_until( const std::chrono::time_point<Clock, Duration>& timeout_time ) {
		const auto lockFunction = [timeout_time](std::timed_mutex &mutex) { return mutex.try_lock_until(timeout_time);};
		return try_lock(lockFunction);
	}
	template<typename Rep, typename Period>
	bool try_lock_for_shared( const std::chrono::duration<Rep, Period>& timeout_duration ) {
		const auto lockFunction = [timeout_duration](std::timed_mutex &mutex) { return mutex.try_lock_for(timeout_duration);};
		return try_lock_shared(lockFunction);
	}
	template<typename Clock, typename Duration>
	bool try_lock_until_shared( const std::chrono::time_point<Clock, Duration>& timeout_time ) {
		const auto lockFunction = [timeout_time](std::timed_mutex &mutex) { return mutex.try_lock_until(timeout_time);};
		return try_lock_shared(lockFunction);
	}
private:
	static const std::function<bool(std::timed_mutex &mutex)> tryLockForFunction;
};

}

#endif
