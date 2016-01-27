#include <recursiveMutex.h>
#include <recursiveSharedMutex.h>
#include <recursiveTimedMutex.h>
#include <sharedTimedMutex.h>
#include <sharedMutexTemplate.h>
#include <conditionVariable.h>
#include <iostream>
#include <thread>
#include <chrono>

typedef std::unique_ptr<std::thread> threadPtr;
typedef std::atomic<unsigned int> atomicUInt;

template <typename L>
static void threadUseSharedRecursiveMutex(L *mutex, atomicUInt *inRecursiveCriticalSection, atomicUInt *SharedInCriticalSection, atomicUInt *totalShared) {

	for (unsigned int i = 0; i < 10000; i++) {
		mutex->lock_shared();
		(*SharedInCriticalSection)++;
		(*totalShared)++;
		if (inRecursiveCriticalSection->load() != 0)
			printf("error in lock_shared\n");
		mutex->lock_shared();
		if (inRecursiveCriticalSection->load() != 0)
			printf("error in lock_shared\n");
		mutex->lock_shared();
		if (inRecursiveCriticalSection->load() != 0)
			printf("error in lock_shared\n");
		mutex->lock_shared();
		if (inRecursiveCriticalSection->load() != 0)
			printf("error in lock_shared\n");
		mutex->unlock_shared();
		if (inRecursiveCriticalSection->load() != 0)
			printf("error in lock_shared\n");
		mutex->unlock_shared();
		if (inRecursiveCriticalSection->load() != 0)
			printf("error in lock_shared\n");
		mutex->unlock_shared();
		if (inRecursiveCriticalSection->load() != 0)
			printf("error in lock_shared\n");
		(*SharedInCriticalSection)--;
		mutex->unlock_shared();
	}
}

template <typename L>
static void threadUseRecursiveTryLockMutex(L *mutex, std::function<bool(L *)> tryLock, atomicUInt *inRecursiveCriticalSection,
								atomicUInt *SharedInCriticalSection, atomicUInt *totalExclusive) {
	for (unsigned int i = 0; i < 10000; i++) {
		if (tryLock(mutex)) {
			(*inRecursiveCriticalSection)++;
			(*totalExclusive)++;
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				printf("error in when locking mutex\n");
			tryLock(mutex);
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				printf("error in when locking mutex\n");
			tryLock(mutex);
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				printf("error in when locking mutex\n");
			tryLock(mutex);
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				printf("error in when locking mutex\n");
			mutex->unlock();
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				printf("error in when locking mutex\n");
			mutex->unlock();
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				printf("error in when locking mutex\n");
			mutex->unlock();
			if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
				printf("error in when locking mutex\n");
			(*inRecursiveCriticalSection)--;
			mutex->unlock();
		}
	}
}

template <typename L>
static void threadUseRecursiveMutex(L *mutex, atomicUInt *inRecursiveCriticalSection, atomicUInt *SharedInCriticalSection, atomicUInt *totalExclusive) {
	const auto &tryLockFunc = [](L *mutex) { mutex->lock(); return true; };
	threadUseRecursiveTryLockMutex<L>(mutex, tryLockFunc, inRecursiveCriticalSection, SharedInCriticalSection, totalExclusive);
}

template <typename L>
static void threadUseRecursiveTryLockForMutex(L *mutex, atomicUInt *inRecursiveCriticalSection, atomicUInt *SharedInCriticalSection, atomicUInt *totalExclusive) {
	const auto &tryLockFunc = [](L *mutex) { return mutex->try_lock_for(std::chrono::seconds(2)); };
	threadUseRecursiveTryLockMutex<L>(mutex, tryLockFunc, inRecursiveCriticalSection, SharedInCriticalSection, totalExclusive);
}

template <typename L>
static void threadUseRecursiveTryLockUntilMutex(L *mutex, atomicUInt *inRecursiveCriticalSection, atomicUInt *SharedInCriticalSection, atomicUInt *totalExclusive) {
	const auto &tryLockFunc = [](L *mutex) { const auto inTwoSeconds = std::chrono::steady_clock::now() + std::chrono::seconds(2);
												return mutex->try_lock_until(inTwoSeconds); };
	threadUseRecursiveTryLockMutex<L>(mutex, tryLockFunc, inRecursiveCriticalSection, SharedInCriticalSection, totalExclusive);
}

template <typename L>
static void testRecursiveMutexInParallel() {
	printf("test recursive mutex in parallel\n");
	atomicUInt inRecursiveCriticalSection1 { 0 };
	atomicUInt inRecursiveCriticalSection2 { 0 };
	atomicUInt dummyInSharedRecursiveCriticalSection1 { 0 };
	atomicUInt totalExclusive { 0 };
	L mutex1;
	L mutex2;
	std::vector<threadPtr> threads;
	for(unsigned int i = 0; i < 100; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseRecursiveMutex<L>, &mutex1, &inRecursiveCriticalSection1, &dummyInSharedRecursiveCriticalSection1, &totalExclusive)));
	}
	for(unsigned int i = 0; i < 100; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseRecursiveMutex<L>, &mutex2, &inRecursiveCriticalSection2, &dummyInSharedRecursiveCriticalSection1, &totalExclusive)));
	}

	for(const auto &t : threads) {
		t->join();
	}
	printf("Exclusive mutex locked %u times\n", totalExclusive.load());

}

template <typename L>
static void threadUseLockSharedMutex(L *mutex, std::function<bool(L *)> lockFunc, atomicUInt *inExclusiveCriticalSection, atomicUInt *inSharedCriticalSection, atomicUInt *totalShared) {
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
		atomicUInt *inSharedCriticalSection,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *totalShared) {
	const auto &lockFunc = [](M *mutex) { mutex->lock_shared(); return true; };
	threadUseLockSharedMutex<M>(mutex, lockFunc, inExclusiveCriticalSection, inSharedCriticalSection, totalShared);
}

template <typename M>
static void threadUseTrySharedMutex(M *mutex,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *inExclusiveCriticalSection,
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
		atomicUInt *inSharedCriticalSection,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *totalExclusive) {
	const auto &lockFunc = [] (M *mutex) { mutex->lock(); return true; };
	threadLockMutex<M>(mutex, lockFunc, inExclusiveCriticalSection, inSharedCriticalSection, totalExclusive);
}

template <typename M>
static void threadUseTryExclusiveMutex(M *mutex,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *totalExclusive) {
	const auto &lockFunc = [] (M *mutex) { return mutex->try_lock(); };
	threadLockMutex<M>(mutex, lockFunc, inExclusiveCriticalSection, inSharedCriticalSection, totalExclusive);
}

template <typename M>
static void threadUseTryLockForMutex(M *mutex,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *totalExclusive) {
	const auto &lockFunc = [] (M *mutex) { return mutex->try_lock_for(std::chrono::seconds(2)); };
	threadLockMutex<M>(mutex, lockFunc, inExclusiveCriticalSection, inSharedCriticalSection, totalExclusive);
}

template <typename M>
static void threadUseTryLockUntilMutex(M *mutex,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *totalExclusive) {
	const auto &lockFunc = [] (M *mutex) { const auto inTwoSeconds = std::chrono::steady_clock::now() + std::chrono::seconds(2);
						return mutex->try_lock_until(inTwoSeconds); };
	threadLockMutex<M>(mutex, lockFunc, inExclusiveCriticalSection, inSharedCriticalSection, totalExclusive);
}

static void testSharedMutexInParallel() {
	atomicUInt inSharedCriticalSection { 0 };
	atomicUInt inExclusiveCriticalSection { 0 };
	atomicUInt totalShared { 0 };
	atomicUInt totalExclusive { 0 };

	std::cout << "test shared mutex in parallel" << std::endl;
	std_mutex_extra::SharedMutex mutex;
	std::vector<threadPtr> threads;
	for(unsigned int i = 0; i < 200; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseSharedMutex<std_mutex_extra::SharedMutex>, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalShared)));
	}
	for(unsigned int i = 0; i < 2; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseExclusiveMutex<std_mutex_extra::SharedMutex>, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalExclusive)));
	}

	for(const auto &t : threads) {
		t->join();
	}

	printf("Shared mutex locked %u times\n", totalShared.load());
	printf("Exclusive mutex locked %u times\n", totalExclusive.load());
}

static void testSharedMutexInParallel__try_locks() {
	atomicUInt inSharedCriticalSection { 0 };
	atomicUInt inExclusiveCriticalSection { 0 };
	atomicUInt totalShared { 0 };
	atomicUInt totalExclusive { 0 };

	std::cout << "test shared mutex in parallel (with try_locks)" << std::endl;
	std_mutex_extra::SharedMutex mutex;
	std::vector<threadPtr> threads;
	for(unsigned int i = 0; i < 5; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseTrySharedMutex<std_mutex_extra::SharedMutex>, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalShared)));
	}
	for(unsigned int i = 0; i < 100; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseTryExclusiveMutex<std_mutex_extra::SharedMutex>, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalExclusive)));
	}
	for(const auto &t : threads) {
		t->join();
	}

	printf("Shared mutex locked %u times\n", totalShared.load());
	printf("Exclusive mutex locked %u times\n", totalExclusive.load());

}

static void testSharedRecursiveMutexInParallel() {
	atomicUInt inSharedCriticalSection { 0 };
	atomicUInt inExclusiveCriticalSection { 0 };
	atomicUInt totalShared { 0 };
	atomicUInt totalExclusive { 0 };

	std::cout << "test recursive shared mutex in parallel" << std::endl;
	std_mutex_extra::RecursiveSharedMutex mutex;
	std::vector<threadPtr> threads;
	for(unsigned int i = 0; i < 200; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseRecursiveMutex<std_mutex_extra::RecursiveSharedMutex>, &mutex, &inExclusiveCriticalSection, &inSharedCriticalSection, &totalExclusive)));
	}

	for(unsigned int i = 0; i < 200; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseSharedRecursiveMutex<std_mutex_extra::RecursiveSharedMutex>, &mutex, &inExclusiveCriticalSection, &inSharedCriticalSection, &totalShared)));
	}

	for(const auto &t : threads) {
		t->join();
	}

	printf("Shared mutex locked %u times\n", totalShared.load());
	printf("Exclusive mutex locked %u times\n", totalExclusive.load());
}

void try_to_lock_a_timed_mutex(std_mutex_extra::RecursiveTimedMutex *mutex)
{
	std::chrono::duration<int> timeout(2);
	mutex->try_lock_for(timeout);
}

static void testTimeRecursiveMutex__does_not_block()
{
	std::cout << "test recursive timed mutex lock is non blocking" << std::endl;

	std_mutex_extra::RecursiveTimedMutex mutex;
	std::lock_guard<std_mutex_extra::RecursiveTimedMutex>lock1(mutex);
	std::lock_guard<std_mutex_extra::RecursiveTimedMutex>lock2(mutex);
	std::thread thread(try_to_lock_a_timed_mutex, &mutex);
	thread.join();
}

static void testRecursiveTimedMutexInParallel__try_lock_for() {
	atomicUInt inSharedCriticalSection { 0 };
	atomicUInt inExclusiveCriticalSection { 0 };
	atomicUInt totalShared { 0 };
	atomicUInt totalExclusive { 0 };

	std::cout << "test recursive timed mutex in parallel (with try_lock_for)" << std::endl;
	std_mutex_extra::RecursiveTimedMutex mutex;
	std::vector<threadPtr> threads;
	for(unsigned int i = 0; i < 200; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseRecursiveTryLockForMutex<std_mutex_extra::RecursiveTimedMutex>, &mutex, &inExclusiveCriticalSection, &inSharedCriticalSection, &totalExclusive)));
	}
	for(unsigned int i = 0; i < 200; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseRecursiveMutex<std_mutex_extra::RecursiveTimedMutex>, &mutex, &inExclusiveCriticalSection, &inSharedCriticalSection, &totalExclusive)));
	}
	for(const auto &t : threads) {
		t->join();
	}

	printf("Shared mutex locked %u times\n", totalShared.load());
	printf("Exclusive mutex locked %u times\n", totalExclusive.load());
}

static void testRecursiveTimedMutexInParallel__try_lock_until() {
	atomicUInt inSharedCriticalSection { 0 };
	atomicUInt inExclusiveCriticalSection { 0 };
	atomicUInt totalShared { 0 };
	atomicUInt totalExclusive { 0 };

	std::cout << "test recursive timed mutex in parallel (with try_lock_until)" << std::endl;
	std_mutex_extra::RecursiveTimedMutex mutex;
	std::vector<threadPtr> threads;
	for(unsigned int i = 0; i < 200; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseRecursiveTryLockUntilMutex<std_mutex_extra::RecursiveTimedMutex>, &mutex, &inExclusiveCriticalSection, &inSharedCriticalSection, &totalExclusive)));
	}
	for(unsigned int i = 0; i < 200; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseRecursiveMutex<std_mutex_extra::RecursiveTimedMutex>, &mutex, &inExclusiveCriticalSection, &inSharedCriticalSection, &totalExclusive)));
	}
	for(const auto &t : threads) {
		t->join();
	}

	printf("Shared mutex locked %u times\n", totalShared.load());
	printf("Exclusive mutex locked %u times\n", totalExclusive.load());
}

static void testSharedTimedMutexInParallel__try_lock_for_shared() {
	atomicUInt inSharedCriticalSection { 0 };
	atomicUInt inExclusiveCriticalSection { 0 };
	atomicUInt totalShared { 0 };
	atomicUInt totalExclusive { 0 };

	std::cout << "test shared timed mutex in parallel (with try_locks_for_shared)" << std::endl;
	std_mutex_extra::SharedTimedMutex mutex;
	std::vector<threadPtr> threads;
	for(unsigned int i = 0; i < 5; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseExclusiveMutex<std_mutex_extra::SharedTimedMutex>, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalExclusive)));
	}
	for(unsigned int i = 0; i < 5; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseTryLockForMutex<std_mutex_extra::SharedTimedMutex>, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalShared)));
	}
	for(unsigned int i = 0; i < 100; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseTrySharedMutex<std_mutex_extra::SharedTimedMutex>, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalShared)));
	}
	for(const auto &t : threads) {
		t->join();
	}

	printf("Shared mutex locked %u times\n", totalShared.load());
	printf("Exclusive mutex locked %u times\n", totalExclusive.load());

}

static void testSharedTimedMutexInParallel__try_lock_until_shared() {
	atomicUInt inSharedCriticalSection { 0 };
	atomicUInt inExclusiveCriticalSection { 0 };
	atomicUInt totalShared { 0 };
	atomicUInt totalExclusive { 0 };

	std::cout << "test shared timed mutex in parallel (with try_locks_until_shared)" << std::endl;
	std_mutex_extra::SharedTimedMutex mutex;
	std::vector<threadPtr> threads;
	for(unsigned int i = 0; i < 5; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseTryLockUntilSharedMutex<std_mutex_extra::SharedTimedMutex>, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalShared)));
	}
	for(unsigned int i = 0; i < 100; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseTryExclusiveMutex<std_mutex_extra::SharedTimedMutex>, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalExclusive)));
	}
	for(const auto &t : threads) {
		t->join();
	}

	printf("Shared mutex locked %u times\n", totalShared.load());
	printf("Exclusive mutex locked %u times\n", totalExclusive.load());

}

static void testSharedTimedMutexInParallel__try_lock_for() {
	atomicUInt inSharedCriticalSection { 0 };
	atomicUInt inExclusiveCriticalSection { 0 };
	atomicUInt totalShared { 0 };
	atomicUInt totalExclusive { 0 };

	std::cout << "test shared timed mutex in parallel (with try_lock_for)" << std::endl;
	std_mutex_extra::SharedTimedMutex mutex;
	std::vector<threadPtr> threads;
	for(unsigned int i = 0; i < 5; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseTryLockForMutex<std_mutex_extra::SharedTimedMutex>, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalExclusive)));
	}
	for(unsigned int i = 0; i < 100; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseTrySharedMutex<std_mutex_extra::SharedTimedMutex>, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalShared)));
	}
	for(const auto &t : threads) {
		t->join();
	}

	printf("Shared mutex locked %u times\n", totalShared.load());
	printf("Exclusive mutex locked %u times\n", totalExclusive.load());

}

static void testSharedTimedMutexInParallel__try_lock_until() {
	atomicUInt inSharedCriticalSection { 0 };
	atomicUInt inExclusiveCriticalSection { 0 };
	atomicUInt totalShared { 0 };
	atomicUInt totalExclusive { 0 };

	std::cout << "test shared timed mutex in parallel (with try_lock_until)" << std::endl;
	std_mutex_extra::SharedTimedMutex mutex;
	std::vector<threadPtr> threads;
	for(unsigned int i = 0; i < 5; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseTryLockUntilMutex<std_mutex_extra::SharedTimedMutex>, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalExclusive)));
	}
	for(unsigned int i = 0; i < 100; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseTrySharedMutex<std_mutex_extra::SharedTimedMutex>, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalShared)));
	}
	for(const auto &t : threads) {
		t->join();
	}

	printf("Shared mutex locked %u times\n", totalShared.load());
	printf("Exclusive mutex locked %u times\n", totalExclusive.load());

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
	testSharedRecursiveMutexInParallel();
	testTimeRecursiveMutex__does_not_block();
	testRecursiveTimedMutexInParallel__try_lock_for();
	testRecursiveTimedMutexInParallel__try_lock_until();
	testSharedTimedMutexInParallel__try_lock_for_shared();
	testSharedTimedMutexInParallel__try_lock_until_shared();
	testSharedTimedMutexInParallel__try_lock_for();
	testSharedTimedMutexInParallel__try_lock_until();
	condition_variable__wait_and_notify_one();
	condition_variable__wait_with_pred_and_notify_one();
	condition_variable__wait_and_notify_all();
	return 1;
}
