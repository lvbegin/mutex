#ifndef RECURSIVE_SHARED_MUTEX_TEMPLATE_H__
#define RECURSIVE_SHARED_MUTEX_TEMPLATE_H__

#include <recursiveMutexTemplate.h>
#include <sharedMutex.h>

namespace std_mutex_extra {

class RecursiveSharedMutexTemplate : public  RecursiveMutexTemplate {
public:
	template <typename M>
	static void lock_shared(unsigned int instanceId, M &mutex) {
		if (instanceId >= recursiveSharedAquire.size())
			recursiveSharedAquire.resize(instanceId + 1);
		if (notAlreadyAquiredShared(instanceId, recursiveSharedAquire, recursiveAquireCounters))
			mutex.lock_shared();
		recursiveSharedAquire[instanceId]++;
	}
	template <typename M>
	static void unlock_shared(unsigned int instanceId, M &mutex) {
		if (instanceId >= recursiveSharedAquire.size() || 0 == recursiveSharedAquire[instanceId])
			throw std::runtime_error("unlock a non-locked lock.");
		recursiveSharedAquire[instanceId]--;
		if (0 == recursiveSharedAquire[instanceId])
			mutex.unlock_shared();
	}
	template <typename M>
	static bool try_lock_shared(unsigned int instanceId, M &mutex) {
		if (instanceId >= recursiveSharedAquire.size())
			recursiveSharedAquire.resize(instanceId + 1);
		const auto locked = notAlreadyAquiredShared(instanceId, recursiveSharedAquire, recursiveAquireCounters) ? mutex.try_lock_shared() : true;
		if (locked)
			recursiveSharedAquire[instanceId]++;
		return locked;
	}
private:
	const unsigned int id;
	SharedMutex mutex;
	static thread_local std::vector<uint_fast16_t> recursiveSharedAquire;

	static bool notAlreadyAquiredShared(unsigned int instanceId, std::vector<uint_fast16_t> &recursiveSharedAquire,
													const std::vector<uint_fast16_t> &recursiveAquire) {
		return (0 == recursiveSharedAquire[instanceId] &&
				((instanceId >= recursiveAquire.size()) || 0 == recursiveAquire[instanceId]));
	}
};

}
#endif
