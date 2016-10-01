#include <recursiveMutex.h>
#include <recursiveSharedMutex.h>
#include <recursiveTimedMutex.h>
#include <sharedTimedMutex.h>
#include <conditionVariable.h>
#include <recursiveSharedTimedMutex.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <lockGuard.h>

typedef std::unique_ptr<std::thread> threadPtr;
typedef std::atomic<unsigned int> atomicUInt;

static std::atomic<unsigned int> nbErrors { 0 };

static inline void errorOccured(std::string message) {
	nbErrors++;
	std::cout << message << std::endl;
}

template <typename L>
static void threadUseRecursiveGenericTryLockMutex(L *mutex, std::function<bool(L *)> tryLock,
		atomicUInt *inRecursiveCriticalSection,
		atomicUInt *SharedInCriticalSection,
		atomicUInt *totalExclusive) {
	for (unsigned int i = 0; i < 10000; i++) {
		if (tryLock(mutex)) {
			(*inRecursiveCriticalSection)++;
			(*totalExclusive)++;
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				errorOccured("error in when locking mutex (lock())");
			tryLock(mutex);
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				errorOccured("error in when locking mutex (lock())");
			tryLock(mutex);
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				errorOccured("error in when locking mutex (lock())");
			tryLock(mutex);
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				errorOccured("error in when locking mutex (lock())");
			mutex->unlock();
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				errorOccured("error in when locking mutex (lock())");
			mutex->unlock();
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				errorOccured("error in when locking mutex (lock())");
			mutex->unlock();
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				errorOccured("error in when locking mutex (lock())");
			(*inRecursiveCriticalSection)--;
			mutex->unlock();
		}
	}
}

template <typename L>
static void threadUseRecursiveGenericTryLockSharedMutex(L *mutex, std::function<bool(L *)> tryLockShared,
		atomicUInt *inRecursiveCriticalSection,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *totalShared) {
	for (unsigned int i = 0; i < 10000; i++) {
		if (tryLockShared(mutex)) {
			(*inSharedCriticalSection)++;
			(*totalShared)++;
			if (0 != inRecursiveCriticalSection->load())
				errorOccured("error in when locking mutex");
			tryLockShared(mutex);
			if (0 != inRecursiveCriticalSection->load())
				errorOccured("error in when locking mutex");
			tryLockShared(mutex);
			if (0 != inRecursiveCriticalSection->load())
				errorOccured("error in when locking mutex");
			tryLockShared(mutex);
			if (0 != inRecursiveCriticalSection->load())
				errorOccured("error in when locking mutex");
			mutex->unlock_shared();
			if (0 != inRecursiveCriticalSection->load())
				errorOccured("error in when locking mutex");
			mutex->unlock_shared();
			if (0 != inRecursiveCriticalSection->load())
				errorOccured("error in when locking mutex");
			mutex->unlock_shared();
			if (0 != inRecursiveCriticalSection->load())
				errorOccured("error in when locking mutex");
			(*inSharedCriticalSection)--;
			mutex->unlock_shared();
		}
	}
}


template <typename L>
static void threadUseRecursiveMutex(L *mutex,
		atomicUInt *inRecursiveCriticalSection,
		atomicUInt *SharedInCriticalSection,
		atomicUInt *totalExclusive,
		atomicUInt *totalShared) {
	const auto &tryLockFunc = [](L *mutex) { mutex->lock(); return true; };
	threadUseRecursiveGenericTryLockMutex<L>(mutex, tryLockFunc, inRecursiveCriticalSection, SharedInCriticalSection, totalExclusive);
}

template <typename L>
static void threadUseRecursiveTryLockMutex(L *mutex,
		atomicUInt *inRecursiveCriticalSection,
		atomicUInt *SharedInCriticalSection,
		atomicUInt *totalExclusive,
		atomicUInt *totalShared) {
	const auto &tryLockFunc = [](L *mutex) { return mutex->try_lock(); };
	threadUseRecursiveGenericTryLockMutex<L>(mutex, tryLockFunc, inRecursiveCriticalSection, SharedInCriticalSection, totalExclusive);
}


template <typename L>
static void threadUseRecursiveTryLockForMutex(L *mutex,
		atomicUInt *inRecursiveCriticalSection,
		atomicUInt *SharedInCriticalSection,
		atomicUInt *totalExclusive,
		atomicUInt *totalShared) {
	const auto &tryLockFunc = [](L *mutex) { return mutex->try_lock_for(std::chrono::seconds(2)); };
	threadUseRecursiveGenericTryLockMutex<L>(mutex, tryLockFunc, inRecursiveCriticalSection, SharedInCriticalSection, totalExclusive);
}

template <typename L>
static void threadUseRecursiveTryLockUntilMutex(L *mutex,
		atomicUInt *inRecursiveCriticalSection,
		atomicUInt *SharedInCriticalSection,
		atomicUInt *totalExclusive,
		atomicUInt *totalShared) {
	const auto &tryLockFunc = [](L *mutex) { const auto inTwoSeconds = std::chrono::steady_clock::now() + std::chrono::seconds(2);
												return mutex->try_lock_until(inTwoSeconds); };
	threadUseRecursiveGenericTryLockMutex<L>(mutex, tryLockFunc, inRecursiveCriticalSection, SharedInCriticalSection, totalExclusive);
}

template <typename L>
static void threadUseRecursiveSharedMutex(L *mutex,
		atomicUInt *inRecursiveCriticalSection,
		atomicUInt *SharedInCriticalSection,
		atomicUInt *totalExclusive,
		atomicUInt *totalShared) {
	const auto &tryLockFunc = [](L *mutex) { mutex->lock_shared(); return true; };
	threadUseRecursiveGenericTryLockSharedMutex<L>(mutex, tryLockFunc, inRecursiveCriticalSection, SharedInCriticalSection, totalShared);
}

template <typename L>
static void threadUseRecursiveTryLockSharedMutex(L *mutex,
		atomicUInt *inRecursiveCriticalSection,
		atomicUInt *SharedInCriticalSection,
		atomicUInt *totalExclusive,
		atomicUInt *totalShared) {
	const auto &tryLockFunc = [](L *mutex) { return mutex->try_lock_shared(); };
	threadUseRecursiveGenericTryLockSharedMutex<L>(mutex, tryLockFunc, inRecursiveCriticalSection, SharedInCriticalSection, totalShared);
}

template <typename L>
static void threadUseRecursiveTryLockForSharedMutex(L *mutex,
		atomicUInt *inRecursiveCriticalSection,
		atomicUInt *SharedInCriticalSection,
		atomicUInt *totalExclusive,
		atomicUInt *totalShared) {
	const auto &tryLockFunc = [](L *mutex) { return mutex->try_lock_for_shared(std::chrono::seconds(2)); };
	threadUseRecursiveGenericTryLockSharedMutex<L>(mutex, tryLockFunc, inRecursiveCriticalSection, SharedInCriticalSection, totalShared);
}

template <typename L>
static void threadUseRecursiveTryLockUntilSharedMutex(L *mutex,
		atomicUInt *inRecursiveCriticalSection,
		atomicUInt *SharedInCriticalSection,
		atomicUInt *totalExclusive,
		atomicUInt *totalShared) {
	const auto &tryLockFunc = [](L *mutex) { const auto inTwoSeconds = std::chrono::steady_clock::now() + std::chrono::seconds(2);
												return mutex->try_lock_until_shared(inTwoSeconds); };
	threadUseRecursiveGenericTryLockSharedMutex<L>(mutex, tryLockFunc, inRecursiveCriticalSection, SharedInCriticalSection, totalShared);
}

template <typename L>
static void threadUseLockSharedMutex(L *mutex, std::function<bool(L *)> lockFunc,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *totalShared) {
	for (unsigned int i = 0; i < 10000; i++) {
		if (lockFunc(mutex)) {
			(*inSharedCriticalSection)++;
			(*totalShared)++;
			if (inExclusiveCriticalSection->load() != 0)
				errorOccured("error in try_lock_for");
			(*inSharedCriticalSection)--;
			mutex->unlock_shared();
		}
	}
}

template <typename M>
static void threadUseSharedMutex(M *mutex,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *totalExclusive,
		atomicUInt *totalShared) {
	const auto &lockFunc = [](M *mutex) { mutex->lock_shared(); return true; };
	threadUseLockSharedMutex<M>(mutex, lockFunc, inExclusiveCriticalSection, inSharedCriticalSection, totalShared);
}

template <typename M>
static void threadUseTrySharedMutex(M *mutex,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *totalExclusive,
		atomicUInt *totalShared) {
	const auto &lockFunc = [](M *mutex) { return mutex->try_lock_shared(); };
	threadUseLockSharedMutex<M>(mutex, lockFunc, inExclusiveCriticalSection, inSharedCriticalSection, totalShared);
}

template <typename M>
static void threadUseTryLockForSharedMutex(M *mutex,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *totalShared) {
	const auto &lockFunc = [](M *mutex) { return mutex->try_lock_for_shared(std::chrono::seconds(2)); };
	threadUseLockSharedMutex<M>(mutex, lockFunc, inExclusiveCriticalSection, inSharedCriticalSection, totalShared);
}

template <typename M>
static void threadUseTryLockUntilSharedMutex(M *mutex,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *totalExclusive,
		atomicUInt *totalShared) {
	const auto &lockFunc = [](M *mutex) { const auto inTwoSeconds = std::chrono::steady_clock::now() + std::chrono::seconds(2);
										return mutex->try_lock_until_shared(inTwoSeconds); };
	threadUseLockSharedMutex<M>(mutex, lockFunc, inExclusiveCriticalSection, inSharedCriticalSection, totalShared);
}

template <typename M>
static void threadLockMutex(M *mutex, std::function<bool (M*)> lockFunc,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *totalExclusive) {
	for (unsigned int i = 0; i < 10000; i++) {
		if (lockFunc(mutex)) {
			(*inExclusiveCriticalSection)++;
			(*totalExclusive)++;
			if (1 != inExclusiveCriticalSection->load() || 0 != inSharedCriticalSection->load())
				errorOccured("error in try_lock_until");
			(*inExclusiveCriticalSection)--;
			mutex->unlock();
		}
	}
}

template <typename M>
static void threadUseExclusiveMutex(M *mutex,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *totalExclusive,
		atomicUInt *totalShared) {
	const auto &lockFunc = [] (M *mutex) { mutex->lock(); return true; };
	threadLockMutex<M>(mutex, lockFunc, inExclusiveCriticalSection, inSharedCriticalSection, totalExclusive);
}

template <typename M>
static void threadUseTryExclusiveMutex(M *mutex,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *totalExclusive,
		atomicUInt *totalShared) {
	const auto &lockFunc = [] (M *mutex) { return mutex->try_lock(); };
	threadLockMutex<M>(mutex, lockFunc, inExclusiveCriticalSection, inSharedCriticalSection, totalExclusive);
}

template <typename M>
static void threadUseTryLockForMutex(M *mutex,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *totalExclusive,
		atomicUInt *totalShared) {
	const auto &lockFunc = [] (M *mutex) { return mutex->try_lock_for(std::chrono::seconds(2)); };
	threadLockMutex<M>(mutex, lockFunc, inExclusiveCriticalSection, inSharedCriticalSection, totalExclusive);
}

template <typename M>
static void threadUseTryLockUntilMutex(M *mutex,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *totalExclusive,
		atomicUInt *totalShared) {
	const auto &lockFunc = [] (M *mutex) { const auto inTwoSeconds = std::chrono::steady_clock::now() + std::chrono::seconds(2);
						return mutex->try_lock_until(inTwoSeconds); };
	threadLockMutex<M>(mutex, lockFunc, inExclusiveCriticalSection, inSharedCriticalSection, totalExclusive);
}

template <typename M>
static void testWithTwoTypesOfThreads( std::function<void (M *mutex,
				atomicUInt *inExclusiveCriticalSection,
				atomicUInt *inSharedCriticalSection,
				atomicUInt *totalExclusive,
				atomicUInt *totalShared)> body1,
		std::function<void (M *mutex,
				atomicUInt *inExclusiveCriticalSection,
				atomicUInt *inSharedCriticalSection,
				atomicUInt *totalExclusive,
				atomicUInt *totalShared)> body2) {
	M mutex;
	atomicUInt inSharedCriticalSection { 0 };
	atomicUInt inExclusiveCriticalSection { 0 };
	atomicUInt totalShared { 0 };
	atomicUInt totalExclusive { 0 };

	std::vector<threadPtr> threads;
	for(unsigned int i = 0; i < 100; i++) {
		threads.push_back(threadPtr(new std::thread(body1, &mutex, &inExclusiveCriticalSection, &inSharedCriticalSection, &totalExclusive, &totalShared)));
	}
	for(unsigned int i = 0; i < 100; i++) {
		threads.push_back(threadPtr(new std::thread(body2, &mutex, &inExclusiveCriticalSection, &inSharedCriticalSection, &totalExclusive, &totalShared)));
	}
	for(const auto &t : threads) {
		t->join();
	}

	printf("Shared mutex locked %u times\n", totalShared.load());
	printf("Exclusive mutex locked %u times\n", totalExclusive.load());
}

static void testSharedMutexInParallel__lock_shared() {
	std::cout << "test shared mutex in parallel (lock_shared)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::SharedMutex>( threadUseSharedMutex<std_mutex_extra::SharedMutex>,
			threadUseExclusiveMutex<std_mutex_extra::SharedMutex>);
}

static void testSharedMutexInParallel__try_locks() {
	std::cout << "test shared mutex in parallel (try_locks_shared)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::SharedMutex>(threadUseTrySharedMutex<std_mutex_extra::SharedMutex>,
			threadUseTryExclusiveMutex<std_mutex_extra::SharedMutex>);
}

static void testSharedRecursiveMutexInParallel__lock() {
	std::cout << "test recursive shared mutex in parallel (lock)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedMutex>(threadUseRecursiveMutex<std_mutex_extra::RecursiveSharedMutex>,
			threadUseRecursiveMutex<std_mutex_extra::RecursiveSharedMutex>);
}

static void testSharedRecursiveMutexInParallel__lock_shared() {
	std::cout << "test recursive shared mutex in parallel (lock_shared)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedMutex>(threadUseRecursiveMutex<std_mutex_extra::RecursiveSharedMutex>,
			threadUseRecursiveSharedMutex<std_mutex_extra::RecursiveSharedMutex>);
}

static void testSharedRecursiveMutexInParallel__try_lock_shared() {
	std::cout << "test recursive shared mutex in parallel (try_lock_shared)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedMutex>(threadUseRecursiveMutex<std_mutex_extra::RecursiveSharedMutex>,
			threadUseRecursiveTryLockSharedMutex<std_mutex_extra::RecursiveSharedMutex>);
}

static void testRecursiveTimedMutexInParallel__try_lock_for() {
	std::cout << "test recursive timed mutex in parallel (try_lock_for)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveTimedMutex>(threadUseRecursiveTryLockForMutex<std_mutex_extra::RecursiveTimedMutex>,
			threadUseRecursiveMutex<std_mutex_extra::RecursiveTimedMutex>);
}

static void testRecursiveTimedMutexInParallel__try_lock_until() {
	std::cout << "test recursive timed mutex in parallel (try_lock_until)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveTimedMutex>(threadUseRecursiveTryLockUntilMutex<std_mutex_extra::RecursiveTimedMutex>,
			threadUseRecursiveMutex<std_mutex_extra::RecursiveTimedMutex>);
}


static void testSharedTimedMutexInParallel__lock() {
	std::cout << "test shared timed mutex in parallel (lock)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::SharedTimedMutex>(threadUseExclusiveMutex<std_mutex_extra::SharedTimedMutex>,
			threadUseTrySharedMutex<std_mutex_extra::SharedTimedMutex>);
}

static void testSharedTimedMutexInParallel__try_lock_for() {
	std::cout << "test shared timed mutex in parallel (try_lock_for)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::SharedTimedMutex>(threadUseTryLockForMutex<std_mutex_extra::SharedTimedMutex>,
			threadUseTrySharedMutex<std_mutex_extra::SharedTimedMutex>);
}

static void testSharedTimedMutexInParallel__try_lock_until() {
	std::cout << "test shared timed mutex in parallel (try_lock_until)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::SharedTimedMutex>(threadUseTryLockUntilMutex<std_mutex_extra::SharedTimedMutex>,
			threadUseTrySharedMutex<std_mutex_extra::SharedTimedMutex>);
}

static void testSharedTimedMutexInParallel__lock_shared() {
	std::cout << "test shared timed mutex in parallel (lock_shared)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::SharedTimedMutex>(threadUseTryLockForMutex<std_mutex_extra::SharedTimedMutex>,
			threadUseSharedMutex<std_mutex_extra::SharedTimedMutex>);

}

static void testSharedTimedMutexInParallel__try_lock_for_shared() {
	std::cout << "test shared timed mutex in parallel (try_lock_for_shared)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::SharedTimedMutex>(threadUseTryLockForMutex<std_mutex_extra::SharedTimedMutex>,
			threadUseTrySharedMutex<std_mutex_extra::SharedTimedMutex>);
}

static void testSharedTimedMutexInParallel__try_lock_until_shared() {
	std::cout << "test shared timed mutex in parallel (try_lock_until_shared)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::SharedTimedMutex>(threadUseTryLockUntilSharedMutex<std_mutex_extra::SharedTimedMutex>,
			threadUseTryExclusiveMutex<std_mutex_extra::SharedTimedMutex>);
}

static void testRecursiveSharedMutexInParallel__lock() {
	std::cout << "test recursive shared mutex in parallel (lock)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedMutex>,
			threadUseRecursiveMutex<std_mutex_extra::RecursiveSharedMutex>);
}

static void testRecursiveSharedMutexInParallel__try_lock() {
	std::cout << "test recursive shared mutex in parallel (try_lock)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedMutex>,
			threadUseRecursiveTryLockMutex<std_mutex_extra::RecursiveSharedMutex>);
}

static void testRecursiveSharedMutexInParallel__lock_shared() {
	std::cout << "test recursive shared mutex in parallel (lock_shared)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedMutex>,
			threadUseRecursiveSharedMutex<std_mutex_extra::RecursiveSharedMutex>);
}

static void testRecursiveSharedMutexInParallel__try_lock_shared() {
	std::cout << "test recursive shared mutex in parallel (try_lock_shared)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedMutex>,
			threadUseRecursiveTryLockSharedMutex<std_mutex_extra::RecursiveSharedMutex>);
}

static void testRecursiveSharedTimedMutexInParallel__lock() {
	std::cout << "test recursive shared timed mutex in parallel (lock)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedTimedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedTimedMutex>,
			threadUseRecursiveMutex<std_mutex_extra::RecursiveSharedTimedMutex>);
}

static void testRecursiveSharedTimedMutexInParallel__try_lock() {
	std::cout << "test recursive shared timed mutex in parallel (try_lock)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedTimedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedTimedMutex>,
			threadUseRecursiveTryLockMutex<std_mutex_extra::RecursiveSharedTimedMutex>);
}

static void testRecursiveSharedTimedMutexInParallel__try_lock_for() {
	std::cout << "test recursive shared timed mutex in parallel (try_lock_for)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedTimedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedTimedMutex>,
			threadUseRecursiveTryLockForMutex<std_mutex_extra::RecursiveSharedTimedMutex>);
}

static void testRecursiveSharedTimedMutexInParallel__try_lock_until() {
	std::cout << "test recursive shared timed mutex in parallel (try_lock_until)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedTimedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedTimedMutex>,
			threadUseRecursiveTryLockUntilMutex<std_mutex_extra::RecursiveSharedTimedMutex>);
}

static void testRecursiveSharedTimedMutexInParallel__lock_shared() {
	std::cout << "test recursive shared timed mutex in parallel (lock_shared)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedTimedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedTimedMutex>,
			threadUseRecursiveSharedMutex<std_mutex_extra::RecursiveSharedTimedMutex>);
}

static void testRecursiveSharedTimedMutexInParallel__try_lock_shared() {
	std::cout << "test recursive shared timed mutex in parallel (try_lock_shared)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedTimedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedTimedMutex>,
			threadUseRecursiveTryLockSharedMutex<std_mutex_extra::RecursiveSharedTimedMutex>);
}

static void testRecursiveSharedTimedMutexInParallel__try_lock_for_shared() {
	std::cout << "test recursive shared timed mutex in parallel (try_lock_for_shared)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedTimedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedTimedMutex>,
			threadUseRecursiveTryLockForSharedMutex<std_mutex_extra::RecursiveSharedTimedMutex>);
}

static void testRecursiveSharedTimedMutexInParallel__try_lock_until_shared() {
	std::cout << "test recursive shared timed mutex in parallel (try_lock_until_shared)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedTimedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedTimedMutex>,
			threadUseRecursiveTryLockUntilSharedMutex<std_mutex_extra::RecursiveSharedTimedMutex>);
}


static void threadThatwaitwithPred(std::timed_mutex *mutex, std_mutex_extra::condition_variable<std::timed_mutex> *condition,
		atomicUInt *nbWaiting) {
	std::unique_lock<std::timed_mutex> lock(*mutex);
	(*nbWaiting)++;
	condition->wait(lock, [nbWaiting]() { return (0 == nbWaiting->load()); });
}

template <typename M>
static void threadThatWaitTemplate(std::timed_mutex *mutex,
		atomicUInt *nbWaiting, std::function<std::cv_status(std::unique_lock<M> &)> waitFunction) {
	std::unique_lock<M> lock(*mutex);
	(*nbWaiting)++;
	if (std::cv_status::timeout == waitFunction(lock))
		errorOccured("ERROR: timeout occured while waiting");
}

static void condition_variable__wait_with_pred_and_notify_one() {

	std::cout << "test condition variable (wait with pred/notify_one)" << std::endl;

	std_mutex_extra::condition_variable<std::timed_mutex> condition;
	std::timed_mutex mutex;
	atomicUInt nbWaiting { 0 };
	threadPtr t(new std::thread(threadThatwaitwithPred, &mutex, &condition, &nbWaiting));
	while (0 == nbWaiting) {

	}

	std::unique_lock<std::timed_mutex> lock(mutex);
	condition.notify_one();
	lock.unlock();
	lock.lock();
	nbWaiting--;
	condition.notify_one();
	lock.unlock();
	t->join();
	std::cout << "test condition variable (wait with pred/notify_one) finished successfully" << std::endl;
}

template <typename M>
static void condition_variable__wait_and_notify_one_template(std_mutex_extra::condition_variable<M> &condition,
		std::function<std::cv_status(std::unique_lock<M> &)> waitFunction) {

	M mutex;
	atomicUInt nbWaiting { 0 };
	threadPtr t(new std::thread(threadThatWaitTemplate<M>, &mutex, &nbWaiting, waitFunction));
	while (0 == nbWaiting) {

	}

	std::unique_lock<std::timed_mutex> lock(mutex);
	condition.notify_one();
	lock.unlock();
	t->join();
}

static void condition_variable__wait_and_notify_one() {

	std::cout << "test condition variable (wait/notify_one)" << std::endl;

	std_mutex_extra::condition_variable<std::timed_mutex> condition;
	const auto &waitFunction = [&condition](std::unique_lock<std::timed_mutex> &lock) { condition.wait(lock); return std::cv_status::no_timeout;};
	condition_variable__wait_and_notify_one_template<std::timed_mutex>(condition, waitFunction);

	std::cout << "test condition variable (wait/notify_one) finished successfully" << std::endl;
}


static void condition_variable__wait_for_and_notify_one() {

	std::cout << "test condition variable (wait_for/notify_one)" << std::endl;

	std_mutex_extra::condition_variable<std::timed_mutex> condition;
	const auto &waitFunction = [&condition](std::unique_lock<std::timed_mutex> &lock) { return condition.wait_for(lock, std::chrono::seconds(2)); };
	condition_variable__wait_and_notify_one_template<std::timed_mutex>(condition, waitFunction);

	std::cout << "test condition variable (wait_for/notify_one) finished successfully" << std::endl;
}

static void condition_variable__wait_until_and_notify_one() {

	std::cout << "test condition variable (wait_until/notify_one)" << std::endl;

	std_mutex_extra::condition_variable<std::timed_mutex> condition;
	const auto &waitFunction = [&condition](std::unique_lock<std::timed_mutex> &lock) { return condition.wait_until(lock, std::chrono::steady_clock::now()  +  std::chrono::seconds(2)); };
	condition_variable__wait_and_notify_one_template<std::timed_mutex>(condition, waitFunction);

	std::cout << "test condition variable (wait_until/notify_one) finished successfully" << std::endl;
}

template <typename M>
static void condition_variable__wait_and_notify_all_template(std_mutex_extra::condition_variable<M> &condition,
		std::function<std::cv_status(std::unique_lock<M> &)> waitFunction) {

	M mutex;
	atomicUInt nbWaiting { 0 };

	std::vector<threadPtr> threads;
	threads.push_back(threadPtr(new std::thread(threadThatWaitTemplate<M>, &mutex, &nbWaiting, waitFunction)));
	threads.push_back(threadPtr(new std::thread(threadThatWaitTemplate<M>, &mutex, &nbWaiting, waitFunction)));
	while (2 != nbWaiting) { }

	std::unique_lock<std::timed_mutex> lock(mutex);
	condition.notify_all();
	lock.unlock();
	for (const auto &t : threads)
		t->join();
}

static void condition_variable__wait_and_notify_all() {

	std::cout << "test condition variable (wait/notify_all)" << std::endl;

	std_mutex_extra::condition_variable<std::timed_mutex> condition;
	const auto &waitFunction = [&condition](std::unique_lock<std::timed_mutex> &lock) {condition.wait(lock); return std::cv_status::no_timeout;};
	condition_variable__wait_and_notify_all_template<std::timed_mutex>(condition, waitFunction);

	std::cout << "test condition variable (wait/notify_all) finished successfully" << std::endl;
}

static void condition_variable__wait_for_and_notify_all() {

	std::cout << "test condition variable (wait_for/notify_all)" << std::endl;

	std_mutex_extra::condition_variable<std::timed_mutex> condition;
	const auto &waitFunction = [&condition](std::unique_lock<std::timed_mutex> &lock) {return condition.wait_for(lock, std::chrono::seconds(2)); };
	condition_variable__wait_and_notify_all_template<std::timed_mutex>(condition, waitFunction);

	std::cout << "test condition variable (wait_for/notify_all) finished successfully" << std::endl;
}

static void condition_variable__wait_until_and_notify_all() {

	std::cout << "test condition variable (wait_until/notify_all)" << std::endl;

	std_mutex_extra::condition_variable<std::timed_mutex> condition;
	const auto &waitFunction = [&condition](std::unique_lock<std::timed_mutex> &lock) {return condition.wait_until(lock, std::chrono::steady_clock::now() + std::chrono::seconds(2)); };
	condition_variable__wait_and_notify_all_template<std::timed_mutex>(condition, waitFunction);

	std::cout << "test condition variable (wait_until/notify_all) finished successfully" << std::endl;
}

static void condition_variable__wait_for_does_not_block()
{
	std::cout << "test condition variable (wait_for does not block)" << std::endl;

	std::timed_mutex mutex;
	std::unique_lock<std::timed_mutex> lock(mutex);
	std_mutex_extra::condition_variable<std::timed_mutex> condition;

	std::cv_status status = condition.wait_for(lock, std::chrono::seconds(2));
	if (std::cv_status::no_timeout == status)
		errorOccured("Error: wait_for() does not return std::cv_status::timeout");
}

static void condition_variable__wait_until_does_not_block()
{
	std::cout << "test condition variable (wait_until does not block)" << std::endl;
	std::timed_mutex mutex;
	std::unique_lock<std::timed_mutex> lock(mutex);
	std_mutex_extra::condition_variable<std::timed_mutex> condition;

	std::cv_status status = condition.wait_until(lock, std::chrono::steady_clock::now() +  std::chrono::seconds(2));
	if (std::cv_status::no_timeout == status)
		errorOccured("Error: wait_for() does not return std::cv_status::timeout");
}

template<typename M>
static void testseveralMutexesInParallel() {
	M mutex1;
	M mutex2;
	atomicUInt inSharedCriticalSection1 { 0 };
	atomicUInt inExclusiveCriticalSection1 { 0 };
	atomicUInt totalShared1 { 0 };
	atomicUInt totalExclusive1 { 0 };
	atomicUInt inSharedCriticalSection2 { 0 };
	atomicUInt inExclusiveCriticalSection2 { 0 };
	atomicUInt totalShared2 { 0 };
	atomicUInt totalExclusive2 { 0 };

	std::vector<threadPtr> threads;
	threads.push_back(threadPtr(new std::thread(threadUseRecursiveMutex<M> ,&mutex1, &inExclusiveCriticalSection1, &inSharedCriticalSection1, &totalExclusive1, &totalShared1)));
	threads.push_back(threadPtr(new std::thread(threadUseRecursiveMutex<M> ,&mutex2, &inExclusiveCriticalSection2, &inSharedCriticalSection2, &totalExclusive2, &totalShared2)));

	for(const auto &t : threads) {
		t->join();
	}

	printf("Shared mutex locked %u times\n", totalShared1.load() + totalShared2.load());
	printf("Exclusive mutex locked %u times\n", totalExclusive1.load() + totalExclusive2.load());
}

static void testseveralRecursiveMutexesInParallel() {
	std::cout << "test two recursive lock in parallel" << std::endl;

	testseveralMutexesInParallel<std_mutex_extra::RecursiveMutex>();
}

static void testseveralRecursiveSharedMutexesInParallel() {
	std::cout << "test two recursive shared lock in parallel" << std::endl;

	testseveralMutexesInParallel<std_mutex_extra::RecursiveSharedMutex>();
}

static void testseveralRecursiveSharedTimedMutexesInParallel() {
	std::cout << "test two recursive shared timed lock in parallel" << std::endl;

	testseveralMutexesInParallel<std_mutex_extra::RecursiveSharedTimedMutex>();
}


static void lockUnlock( std::mutex &a, std::mutex &b)
{
	std_mutex_extra::lock_guard<std::mutex, std::mutex>(a, b);

}

static void testLockGuard() {
	std::cout << "lockGard called sequentially" << std::endl;

	std::mutex a;
	std::mutex b;
	lockUnlock(a, b);
	lockUnlock(a, b);
}

int main() {
	testSharedMutexInParallel__lock_shared();
	testSharedMutexInParallel__try_locks();
	testSharedRecursiveMutexInParallel__lock();
	testSharedRecursiveMutexInParallel__lock_shared();
	testSharedRecursiveMutexInParallel__try_lock_shared();
	testRecursiveTimedMutexInParallel__try_lock_for();
	testRecursiveTimedMutexInParallel__try_lock_until();
	testSharedTimedMutexInParallel__lock();
	testSharedTimedMutexInParallel__try_lock_for();
	testSharedTimedMutexInParallel__try_lock_until();
	testSharedTimedMutexInParallel__lock_shared();
	testSharedTimedMutexInParallel__try_lock_for_shared();
	testSharedTimedMutexInParallel__try_lock_until_shared();
	testRecursiveSharedMutexInParallel__lock();
	testRecursiveSharedMutexInParallel__try_lock();
	testRecursiveSharedMutexInParallel__lock_shared();
	testRecursiveSharedMutexInParallel__try_lock_shared();
	testRecursiveSharedTimedMutexInParallel__lock();
	testRecursiveSharedTimedMutexInParallel__try_lock();
	testRecursiveSharedTimedMutexInParallel__try_lock_for();
	testRecursiveSharedTimedMutexInParallel__try_lock_until();
	testRecursiveSharedTimedMutexInParallel__lock_shared();
	testRecursiveSharedTimedMutexInParallel__try_lock_shared();
	testRecursiveSharedTimedMutexInParallel__try_lock_for_shared();
	testRecursiveSharedTimedMutexInParallel__try_lock_until_shared();
	testseveralRecursiveMutexesInParallel();
	testseveralRecursiveSharedMutexesInParallel();
	testseveralRecursiveSharedTimedMutexesInParallel();

	condition_variable__wait_and_notify_one();
	condition_variable__wait_for_and_notify_one();
	condition_variable__wait_until_and_notify_one();
	condition_variable__wait_with_pred_and_notify_one();
	condition_variable__wait_and_notify_all();
	condition_variable__wait_for_and_notify_all();
	condition_variable__wait_until_and_notify_all();

	condition_variable__wait_for_does_not_block();
	condition_variable__wait_until_does_not_block();

	testLockGuard();
	return nbErrors.load();
}
