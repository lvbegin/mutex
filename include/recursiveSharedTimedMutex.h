#ifndef RECURSIVE_SHARED_TIMED_MUTEX_H__
#define RECURSIVE_SHARED_TIMED_MUTEX_H__

#include <sharedTimedMutex.h>
#include <recursiveMutex.h>

namespace std_mutex_extra {

class RecursiveSharedTimedMutex {
public:
	RecursiveSharedTimedMutex() : id(RecursiveMutexTemplate::newId()) {}
	~RecursiveSharedTimedMutex() = default;

	void lock() {  RecursiveMutexTemplate::lock<SharedTimedMutex>(mutex); }
	void unlock() {  RecursiveMutexTemplate::unlock<SharedTimedMutex>(mutex); }
	bool try_lock() {  return RecursiveMutexTemplate::try_lock<SharedTimedMutex>(mutex); }
	template <typename Rep, typename Period>
	bool try_lock_for( const std::chrono::duration<Rep, Period>& timeout_duration ) { return RecursiveMutexTemplate::try_lock_for<SharedTimedMutex, Rep, Period>(mutex, timeout_duration); }
	template <typename Clock, typename Duration>
	bool try_lock_until( const std::chrono::time_point<Clock, Duration>& timeout_time ) { return RecursiveMutexTemplate::try_lock_for<SharedTimedMutex, Clock, Duration>(mutex, timeout_time); }
	void lock_shared() { RecursiveMutexTemplate::lock_shared<SharedTimedMutex>(mutex); }
	void unlock_shared() { RecursiveMutexTemplate::unlock_shared<SharedTimedMutex>(mutex); }
	bool try_lock_shared() { return RecursiveMutexTemplate::try_lock_shared<SharedTimedMutex>(mutex); }
	template <typename Rep, typename Period>
	bool try_lock_for_shared( const std::chrono::duration<Rep, Period>& timeout_duration ) {  return RecursiveMutexTemplate::try_lock_for_shared<SharedTimedMutex, Rep, Period>(mutex, timeout_duration); }
	template <typename Clock, typename Duration>
	bool try_lock_until_shared( const std::chrono::time_point<Clock, Duration>& timeout_time ) { return RecursiveMutexTemplate::try_lock_for_shared<SharedTimedMutex, Clock, Duration>(mutex, timeout_time); }
private:
	const unsigned int id;
	SharedTimedMutex mutex;
};

}
#endif
