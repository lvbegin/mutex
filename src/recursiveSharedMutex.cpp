#include <recursiveSharedMutex.h>

namespace std_mutex_extra {

thread_local std::vector<uint_fast16_t> RecursiveSharedMutex::recursiveSharedAquire;

RecursiveSharedMutex::RecursiveSharedMutex() = default;
RecursiveSharedMutex::~RecursiveSharedMutex() = default;

void RecursiveSharedMutex::lock_shared() {
	const auto instanceId = RecursiveMutexTemplate<SharedMutex>::instanceId;
	const auto &recursiveAquire = RecursiveMutexTemplate<SharedMutex>::recursiveAquire;

	if (instanceId >= recursiveSharedAquire.size())
		recursiveSharedAquire.resize(instanceId + 1);
	if (0 == recursiveSharedAquire[instanceId] &&
			(instanceId < recursiveAquire.size() && 0 == recursiveAquire[instanceId]))
		RecursiveMutexTemplate<SharedMutex>::mutex.lock_shared();
	recursiveSharedAquire[instanceId]++;
}

void RecursiveSharedMutex::unlock_shared() {
	const auto instanceId = RecursiveMutexTemplate<SharedMutex>::instanceId;

	if (instanceId >= recursiveSharedAquire.size() || 0 == recursiveSharedAquire[instanceId])
		throw std::runtime_error("unlock a non-locked lock.");
	recursiveSharedAquire[instanceId]--;
	if (0 == recursiveSharedAquire[instanceId])
		RecursiveMutexTemplate<SharedMutex>::mutex.unlock_shared();
}

bool RecursiveSharedMutex::try_lock_shared() {
	const auto instanceId = RecursiveMutexTemplate<SharedMutex>::instanceId;
	auto &mutex = RecursiveMutexTemplate<SharedMutex>::mutex;

	if (instanceId >= recursiveSharedAquire.size())
		recursiveSharedAquire.resize(instanceId + 1);
	const auto locked = (0 == recursiveSharedAquire[instanceId]) ? mutex.try_lock_shared() : true;
	if (locked)
		recursiveSharedAquire[instanceId]++;
	return locked;
}

}
