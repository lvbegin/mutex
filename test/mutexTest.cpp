#include <recursiveMutex.h>
#include <sharedMutex.h>
#include <iostream>
#include <thread>
#include <chrono>

typedef std::unique_ptr<std::thread> threadPtr;
typedef std::atomic<unsigned int> atomicUInt;

template <typename L>
static void threadUseRecursiveMutex(L *mutex, atomicUInt *inRecursiveCriticalSection, atomicUInt *SharedInCriticalSection, atomicUInt *totalExclusive) {
	for (unsigned int i = 0; i < 10000; i++) {
		mutex->lock();
		(*inRecursiveCriticalSection)++;
		(*totalExclusive)++;
		mutex->lock();
		if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
			printf("error\n");
		mutex->lock();
		if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
			printf("error\n");
		mutex->lock();
		if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
			printf("error\n");
		mutex->unlock();
		if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
			printf("error\n");
		mutex->unlock();
		if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
			printf("error\n");
		mutex->unlock();
		if (inRecursiveCriticalSection->load() != 1 || SharedInCriticalSection->load() != 0)
			printf("error\n");
		(*inRecursiveCriticalSection)--;
		mutex->unlock();
	}
}

template <typename L>
static void threadUseSharedRecursiveMutex(L *mutex, atomicUInt *inRecursiveCriticalSection, atomicUInt *SharedInCriticalSection, atomicUInt *totalShared) {
	for (unsigned int i = 0; i < 10000; i++) {
		mutex->lock();
		(*SharedInCriticalSection)++;
		(*totalShared)++;
		mutex->lock();
		if (inRecursiveCriticalSection->load() != 0)
			printf("error\n");
		mutex->lock();
		if (inRecursiveCriticalSection->load() != 0)
			printf("error\n");
		mutex->lock();
		if (inRecursiveCriticalSection->load() != 0)
			printf("error\n");
		mutex->unlock();
		if (inRecursiveCriticalSection->load() != 0)
			printf("error\n");
		mutex->unlock();
		if (inRecursiveCriticalSection->load() != 0)
			printf("error\n");
		mutex->unlock();
		if (inRecursiveCriticalSection->load() != 0)
			printf("error\n");
		(*SharedInCriticalSection)--;
		mutex->unlock();
	}
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

static void threadUseSharedMutex(std_mutex_extra::SharedMutex *mutex,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *totalShared,
		atomicUInt *totalExclusive) {
	for (unsigned int i = 0; i < 10000; i++) {
		mutex->lock_shared();
		(*inSharedCriticalSection)++;
		if (inSharedCriticalSection->load() > 0 && inExclusiveCriticalSection->load() > 0)
			printf("error: (shared) exclusive and shared at the same time. (shared = %u, exclusive = %u)\n", inSharedCriticalSection->load(), inExclusiveCriticalSection->load());
		(*totalShared)++;
		(*inSharedCriticalSection)--;
		mutex->unlock_shared();
	}
}

static void threadUseExclusiveMutex(std_mutex_extra::SharedMutex *mutex,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *totalShared,
		atomicUInt *totalExclusive) {
	for (unsigned int i = 0; i < 10000; i++) {
		mutex->lock();
		(*inExclusiveCriticalSection)++;
		if (inSharedCriticalSection->load() > 0 && inExclusiveCriticalSection->load() > 0)
			printf("error: (exclusive) exclusive and shared at the same time. (shared = %u, exclusive = %u)\n", inSharedCriticalSection->load(), inExclusiveCriticalSection->load());
		(*totalExclusive)++;
		(*inExclusiveCriticalSection)--;
		mutex->unlock();
	}
}

static void threadUseTrySharedMutex(std_mutex_extra::SharedMutex *mutex,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *totalShared,
		atomicUInt *totalExclusive)
{
	for (unsigned int i = 0; i < 10000; i++) {
		const auto acquired = mutex->try_lock_shared();
		if (acquired)
		{
			(*inSharedCriticalSection)++;
			if (inSharedCriticalSection->load() > 0 && inExclusiveCriticalSection->load() > 0)
				printf("error: (try_lock_shared) exclusive and shared at the same time.(shared = %u, exclusive = %u)\n", inSharedCriticalSection->load(), inExclusiveCriticalSection->load());
			(*totalShared)++;
			(*inSharedCriticalSection)--;
			mutex->unlock_shared();
		}
	}
}

static void threadUseTryExclusiveMutex(std_mutex_extra::SharedMutex *mutex,
		atomicUInt *inSharedCriticalSection,
		atomicUInt *inExclusiveCriticalSection,
		atomicUInt *totalShared,
		atomicUInt *totalExclusive) {
	for (unsigned int i = 0; i < 10000; i++) {
		const auto acquired = mutex->try_lock();
		if (acquired) {
			(*inExclusiveCriticalSection)++;
			if (inSharedCriticalSection->load() > 0 && inExclusiveCriticalSection->load() > 0)
				printf("error: (try_lock) exclusive and shared at the same time.(shared = %u, exclusive = %u)\n", inSharedCriticalSection->load(), inExclusiveCriticalSection->load());
			(*totalExclusive)++;
			(*inExclusiveCriticalSection)--;
			mutex->unlock();
		}
	}
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
		threads.push_back(threadPtr(new std::thread(threadUseSharedMutex, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalShared, &totalExclusive)));
	}
	for(unsigned int i = 0; i < 2; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseExclusiveMutex, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalShared, &totalExclusive)));
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

	std::cout << "test shared mutex in parallel (with try locks)" << std::endl;
	std_mutex_extra::SharedMutex mutex;
	std::vector<threadPtr> threads;
	for(unsigned int i = 0; i < 5; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseTrySharedMutex, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalShared, &totalExclusive)));
	}
	for(unsigned int i = 0; i < 100; i++) {
		threads.push_back(threadPtr(new std::thread(threadUseTryExclusiveMutex, &mutex, &inSharedCriticalSection, &inExclusiveCriticalSection, &totalShared, &totalExclusive)));
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
	std::chrono::duration<int> timeout(2);
	std::lock_guard<std_mutex_extra::RecursiveTimedMutex>lock1(mutex);
	std::lock_guard<std_mutex_extra::RecursiveTimedMutex>lock2(mutex);
	std::thread thread(try_to_lock_a_timed_mutex, &mutex);
	thread.join();
}

int main()
{
	testRecursiveMutexInParallel<std_mutex_extra::RecursiveMutex>();
	testRecursiveMutexInParallel<std_mutex_extra::RecursiveSharedMutex>();
	testSharedMutexInParallel();
	testSharedMutexInParallel__try_locks();
	testSharedRecursiveMutexInParallel();
	testTimeRecursiveMutex__does_not_block();
	return 1;
}
