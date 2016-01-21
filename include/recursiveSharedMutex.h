#ifndef RECURSIVE_SHARED_MUTEX_H__
#define RECURSIVE_SHARED_MUTEX_H__

#include <sharedMutex.h>

namespace std_mutex_extra {

class RecursiveSharedMutex : public  RecursiveMutexTemplate<SharedMutex> {
public:
	RecursiveSharedMutex();
	~RecursiveSharedMutex();

	void lock_shared();
	void unlock_shared();
	bool try_lock_shared();
private:
	static thread_local std::vector<uint_fast16_t> recursiveSharedAquire;
};

}

#endif
