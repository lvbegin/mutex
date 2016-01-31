#include <recursiveMutex.h>
#include <recursiveSharedMutex.h>
#include <recursiveTimedMutex.h>
#include <sharedTimedMutex.h>
#include <conditionVariable.h>
//#include <templates/sharedMutexTemplate.h>
#include <recursiveSharedTimedMutex.h>
#include <iostream>
#include <thread>
#include <chrono>

typedef std::unique_ptr<std::thread> threadPtr;
typedef std::atomic<unsigned int> atomicUInt;

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
				printf("error in when locking mutex (lock())\n");
			tryLock(mutex);
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				printf("error in when locking mutex (lock())\n");
			tryLock(mutex);
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				printf("error in when locking mutex (lock())\n");
			tryLock(mutex);
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				printf("error in when locking mutex (lock())\n");
			mutex->unlock();
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				printf("error in when locking mutex (lock())\n");
			mutex->unlock();
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				printf("error in when locking mutex (lock())\n");
			mutex->unlock();
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				printf("error in when locking mutex (lock())\n");
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
				printf("error in when locking mutex\n");
			tryLockShared(mutex);
			if (0 != inRecursiveCriticalSection->load())
				printf("error in when locking mutex\n");
			tryLockShared(mutex);
			if (0 != inRecursiveCriticalSection->load())
				printf("error in when locking mutex\n");
			tryLockShared(mutex);
			if (0 != inRecursiveCriticalSection->load())
				printf("error in when locking mutex\n");
			mutex->unlock_shared();
			if (0 != inRecursiveCriticalSection->load())
				printf("error in when locking mutex\n");
			mutex->unlock_shared();
			if (0 != inRecursiveCriticalSection->load())
				printf("error in when locking mutex\n");
			mutex->unlock_shared();
			if (0 != inRecursiveCriticalSection->load())
				printf("error in when locking mutex\n");
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
static void testRecursiveMutexInParallel() {
	printf("test recursive mutex in parallel\n");
	atomicUInt inRecursiveCriticalSection1 { 0 };
	atomicUInt inRecursiveCriticalSection2 { 0 };
	atomicUInt dummyInSharedRecursiveCriticalSection1 { 0 };
	atomicUInt totalExclusive { 0 };
	atomicUInt totalShared { 0 };
	L mutex1;
	L mutex2;
	std::vector<threadPtr> threads;
	for(unsigned int i = 0; i < 100; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseRecursiveMutex<L>, &mutex1, &inRecursiveCriticalSection1, &dummyInSharedRecursiveCriticalSection1, &totalExclusive, &totalShared)));
	}
	for(unsigned int i = 0; i < 100; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseRecursiveMutex<L>, &mutex2, &inRecursiveCriticalSection2, &dummyInSharedRecursiveCriticalSection1, &totalExclusive, &totalShared)));
	}

	for(const auto &t : threads) {
		t->join();
	}
	printf("Exclusive mutex locked %u times\n", totalExclusive.load());

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
				printf("error in try_lock_for\n");
			(*inSharedCriticalSection)--;
			mutex->unlock_shared();
		}
	}
}

template <typename M>
static void threadUseSharedMutex(std_mutex_extra::SharedMutex *mutex,
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
				printf("error in try_lock_until\n");
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
	for(unsigned int i = 0; i < 5; i++) {
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

static void testSharedMutexInParallel() {
	std::cout << "test shared mutex in parallel" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::SharedMutex>( threadUseSharedMutex<std_mutex_extra::SharedMutex>,
			threadUseExclusiveMutex<std_mutex_extra::SharedMutex>);
}

static void testSharedMutexInParallel__try_locks() {
	std::cout << "test shared mutex in parallel (with try_locks)" << std::endl;

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
	std::cout << "test recursive timed mutex in parallel (with try_lock_for)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveTimedMutex>(threadUseRecursiveTryLockForMutex<std_mutex_extra::RecursiveTimedMutex>,
			threadUseRecursiveMutex<std_mutex_extra::RecursiveTimedMutex>);
}

static void testRecursiveTimedMutexInParallel__try_lock_until() {
	std::cout << "test recursive timed mutex in parallel (with try_lock_until)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveTimedMutex>(threadUseRecursiveTryLockUntilMutex<std_mutex_extra::RecursiveTimedMutex>,
			threadUseRecursiveMutex<std_mutex_extra::RecursiveTimedMutex>);
}

static void testSharedTimedMutexInParallel__lock() {
	std::cout << "test shared timed mutex in parallel (with lock())" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::SharedTimedMutex>(threadUseExclusiveMutex<std_mutex_extra::SharedTimedMutex>,
			threadUseTrySharedMutex<std_mutex_extra::SharedTimedMutex>);
}

static void testSharedTimedMutexInParallel__try_lock_for_shared() {
	std::cout << "test shared timed mutex in parallel (with try_locks_for_shared)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::SharedTimedMutex>(threadUseTryLockForMutex<std_mutex_extra::SharedTimedMutex>,
			threadUseTrySharedMutex<std_mutex_extra::SharedTimedMutex>);
}

static void testSharedTimedMutexInParallel__try_lock_until_shared() {
	std::cout << "test shared timed mutex in parallel (with try_locks_until_shared)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::SharedTimedMutex>(threadUseTryLockUntilSharedMutex<std_mutex_extra::SharedTimedMutex>,
			threadUseTryExclusiveMutex<std_mutex_extra::SharedTimedMutex>);
}

static void testSharedTimedMutexInParallel__try_lock_for() {
	std::cout << "test shared timed mutex in parallel (with try_lock_for)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::SharedTimedMutex>(threadUseTryLockForMutex<std_mutex_extra::SharedTimedMutex>,
			threadUseTrySharedMutex<std_mutex_extra::SharedTimedMutex>);
}

static void testSharedTimedMutexInParallel__try_lock_until() {
	std::cout << "test shared timed mutex in parallel (with try_lock_until)" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::SharedTimedMutex>(threadUseTryLockUntilMutex<std_mutex_extra::SharedTimedMutex>,
			threadUseTrySharedMutex<std_mutex_extra::SharedTimedMutex>);
}

static void testRecursiveSharedTimedMutexInParallel__lock() {
	std::cout << "test shared timed mutex in parallel (lock())" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedTimedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedTimedMutex>,
			threadUseRecursiveMutex<std_mutex_extra::RecursiveSharedTimedMutex>);
}

static void testRecursiveSharedTimedMutexInParallel__try_lock() {
	std::cout << "test shared timed mutex in parallel (try_lock())" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedTimedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedTimedMutex>,
			threadUseRecursiveTryLockMutex<std_mutex_extra::RecursiveSharedTimedMutex>);
}

static void testRecursiveSharedTimedMutexInParallel__try_lock_for() {
	std::cout << "test shared timed mutex in parallel (try_lock())" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedTimedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedTimedMutex>,
			threadUseRecursiveTryLockForMutex<std_mutex_extra::RecursiveSharedTimedMutex>);
}

static void testRecursiveSharedTimedMutexInParallel__try_lock_until() {
	std::cout << "test shared timed mutex in parallel (try_lock())" << std::endl;

	testWithTwoTypesOfThreads<std_mutex_extra::RecursiveSharedTimedMutex>(threadUseExclusiveMutex<std_mutex_extra::RecursiveSharedTimedMutex>,
			threadUseRecursiveTryLockUntilMutex<std_mutex_extra::RecursiveSharedTimedMutex>);
}

static void threadThatwaitwithPred(std::timed_mutex *mutex, std_mutex_extra::condition_variable<std::timed_mutex> *condition,
		atomicUInt *nbWaiting) {
	std::unique_lock<std::timed_mutex> lock(*mutex);
	(*nbWaiting)++;
	condition->wait(lock, [nbWaiting]() { return (0 == nbWaiting->load()); });
}

static void threadThatwait(std::timed_mutex *mutex, std_mutex_extra::condition_variable<std::timed_mutex> *condition,
		atomicUInt *nbWaiting) {
	std::unique_lock<std::timed_mutex> lock(*mutex);
	(*nbWaiting)++;
	condition->wait(lock);
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

static void condition_variable__wait_and_notify_one() {

	std::cout << "test condition variable (wait/notify_one)" << std::endl;

	std_mutex_extra::condition_variable<std::timed_mutex> condition;
	std::timed_mutex mutex;
	atomicUInt nbWaiting { 0 };
	threadPtr t(new std::thread(threadThatwait, &mutex, &condition, &nbWaiting));
	while (0 == nbWaiting) {

	}

	std::unique_lock<std::timed_mutex> lock(mutex);
	condition.notify_one();
	lock.unlock();
	t->join();
	std::cout << "test condition variable (wait/notify_one) finished successfully" << std::endl;
}

static void condition_variable__wait_and_notify_all() {

	std::cout << "test condition variable (wait/notify_all)" << std::endl;

	std_mutex_extra::condition_variable<std::timed_mutex> condition;
	std::timed_mutex mutex;
	atomicUInt nbWaiting { 0 };

	std::vector<threadPtr> threads;
	threads.push_back(threadPtr(new std::thread(threadThatwait, &mutex, &condition, &nbWaiting)));
	threads.push_back(threadPtr(new std::thread(threadThatwait, &mutex, &condition, &nbWaiting)));
	while (2 != nbWaiting) {

	}

	std::unique_lock<std::timed_mutex> lock(mutex);
	condition.notify_all();
	lock.unlock();
	for (const auto &t : threads)
		t->join();
	std::cout << "test condition variable (wait/notify_all) finished successfully" << std::endl;
}

int main()
{
	testRecursiveMutexInParallel<std_mutex_extra::RecursiveMutex>();
	testRecursiveMutexInParallel<std_mutex_extra::RecursiveSharedMutex>();
	testSharedMutexInParallel();
	testSharedMutexInParallel__try_locks();
	testSharedRecursiveMutexInParallel__lock();
	testSharedRecursiveMutexInParallel__lock_shared();
	testSharedRecursiveMutexInParallel__try_lock_shared();
	testRecursiveTimedMutexInParallel__try_lock_for();
	testRecursiveTimedMutexInParallel__try_lock_until();
	testSharedTimedMutexInParallel__lock();
	testSharedTimedMutexInParallel__try_lock_for_shared();
	testSharedTimedMutexInParallel__try_lock_until_shared();
	testSharedTimedMutexInParallel__try_lock_for();
	testSharedTimedMutexInParallel__try_lock_until();
	testRecursiveSharedTimedMutexInParallel__lock();
	testRecursiveSharedTimedMutexInParallel__try_lock();
	testRecursiveSharedTimedMutexInParallel__try_lock_for();
	testRecursiveSharedTimedMutexInParallel__try_lock_until();

	condition_variable__wait_and_notify_one();
	condition_variable__wait_with_pred_and_notify_one();
	condition_variable__wait_and_notify_all();
	return 1;
}
