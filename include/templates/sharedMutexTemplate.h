/* Copyright 2016 Laurent Van Begin
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef SHARED_MUTEX_TEMPLATE_H__
#define SHARED_MUTEX_TEMPLATE_H__

#include <stdint.h>
#include <memory>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <vector>

namespace std_mutex_extra {

template <typename M, typename C>
class SharedMutexTemplate {
private :
	enum  class lock_status_t { NOT_LOCKED, LOCKED, SHARED_LOCKED };
public:
	SharedMutexTemplate() : id(newId()), nbSharedLocked(0), nbWaitingExclusiveAccess(0), accessQueue(new NoStarvationQueue()) { }
	~SharedMutexTemplate() = default;

	void lock(M &mutex) {
		EnsureMemoryAllocated();
		if (!lockStatusEquals(lock_status_t::NOT_LOCKED))
				throw std::runtime_error("thread tries to lock a mutex that it already locks with shared ownership.");
		std::unique_lock<M> lock(mutex);
		waitForLockExclusive(lock);
		markOwnership();
		accessQueue->removeFirstElementFromWaitingList();
		lock.release();
	}
	void unlock(M &mutex) {
		EnsureMemoryAllocated();
		if (!lockStatusEquals(lock_status_t::LOCKED))
			throw std::runtime_error("thread tries to unlock a mutex that it did not lock.");
		unmarkOwnership();
		mutex.unlock();
	}
	bool try_lock(M &mutex) { return try_lock(mutex, TryLockFunction); }
	void lock_shared(M &mutex) {
		EnsureMemoryAllocated();
		if (!lockStatusEquals(lock_status_t::NOT_LOCKED))
			throw std::runtime_error("thread tries to lock with shared ownership a mutex that it did not lock.");
		std::unique_lock<M> lock(mutex);

		if (!canBypassAccessQueueForSharedLock())
			waitForLockShared(lock);
		markSharedOwnership();
	}
	template<typename Rep, typename Period>
	bool try_lock_for(M &mutex, const std::chrono::duration<Rep, Period>& timeout_duration ) {
		return try_lock(mutex, [timeout_duration](M &mutex) { return mutex.try_lock_for(timeout_duration); });
	}
	template<typename Clock, typename Duration>
	bool try_lock_until(M &mutex, const std::chrono::time_point<Clock, Duration>& timeout_time ) {
		return try_lock(mutex, [timeout_time](M &mutex) { return mutex.try_lock_until(timeout_time); });
	}
	bool try_lock_shared(M &mutex) { return try_lock_shared(mutex, TryLockFunction); }
	template<typename Rep, typename Period>
	bool try_lock_for_shared(M &mutex, const std::chrono::duration<Rep, Period>& timeout_duration ) {
		return try_lock_shared(mutex, [timeout_duration](M &mutex) { return mutex.try_lock_for(timeout_duration); });
	}
	template<typename Clock, typename Duration>
	bool try_lock_until_shared(M &mutex, const std::chrono::time_point<Clock, Duration>& timeout_time ) {
		return try_lock_shared(mutex, [timeout_time](M &mutex) { return mutex.try_lock_until(timeout_time); });
	}
	void unlock_shared(M &mutex) {
		EnsureMemoryAllocated();
		if (!lockStatusEquals(lock_status_t::SHARED_LOCKED))
			throw std::runtime_error("thread tries to unlock a mutex that it did not lock.");
		std::lock_guard<M> lock(mutex);

		unmarkSharedOwnership();
	}
protected:
	static unsigned int newId() {
		static std::atomic<unsigned int> nbInstances { 0 };
		return nbInstances++;
	}
	bool try_lock(M &mutex, std::function<bool(M &mutex)> lockFunction) {
		EnsureMemoryAllocated();
		if (!lockStatusEquals(lock_status_t::NOT_LOCKED))
				throw std::runtime_error("thread tries to lock a mutex that it already locks with shared ownership.");
		if (!lockFunction(mutex))
			return false;
		const auto canKeepMutex = (0 == nbSharedLocked) && accessQueue->isEmpty();
		if (!canKeepMutex)
			mutex.unlock();
		else
			markOwnership();
		return canKeepMutex;
	}
	bool try_lock_shared(M &mutex, std::function<bool(M &mutex)> lockFunction) {
		EnsureMemoryAllocated();
		if (!lockStatusEquals(lock_status_t::NOT_LOCKED))
			throw std::runtime_error("thread tries to lock with shared ownership a mutex that it did not lock.");
		if (!lockFunction(mutex))
			return false;
		const auto queueCanBeAvoidedWithoutStarvation = canBypassAccessQueueForSharedLock();
		if (queueCanBeAvoidedWithoutStarvation)
			markSharedOwnership();
		mutex.unlock();
		return queueCanBeAvoidedWithoutStarvation;
	}

private:
	struct ThreadInfo;
	struct NoStarvationQueue;

	unsigned int id;
	uint_fast16_t nbSharedLocked;
	uint_fast16_t nbWaitingExclusiveAccess;
	std::unique_ptr<NoStarvationQueue> accessQueue;
	static thread_local std::unique_ptr<ThreadInfo> threadInfo;

	static const std::function<bool(M &mutex)> TryLockFunction;

	struct QueueElem {
		QueueElem *next;
		lock_status_t status;
		ThreadInfo *threadInfo;

		QueueElem(ThreadInfo *threadInfo) : next(nullptr), status(lock_status_t::NOT_LOCKED), threadInfo(threadInfo) {}
		~QueueElem() = default;
	};
	struct ThreadInfo {
		C mutexCanBeLocked;
		std::vector<struct QueueElem> waitingQueueElem;

		ThreadInfo() = default;
		~ThreadInfo() = default;
	};
	struct NoStarvationQueue {
		QueueElem *head;
		QueueElem *tail;

		NoStarvationQueue() : head(nullptr), tail(nullptr) {}
		~NoStarvationQueue() = default;
		bool isEmpty() const { return nullptr == head; }
		void addInWaitingList(QueueElem &elem) {
			elem.next = nullptr;
			if (isEmpty())
				head = &elem;
			else
				tail->next = &elem;
			tail = &elem;
		}
		void wait(std::unique_lock<M> &locked, QueueElem &elem, std::function<bool()> condition) {
			addInWaitingList(elem);
			elem.threadInfo->mutexCanBeLocked.wait(locked, std::move(condition));
		}
		void notifyFirstElem() const {
			if (!isEmpty())
				head->threadInfo->mutexCanBeLocked.notify_one();
		}
		void removeFirstElementFromWaitingList() {
			head = head->next;
			if (isEmpty())
				tail = nullptr;
		}
	};

	void waitForLockExclusive(std::unique_lock<M> &lock) {
		nbWaitingExclusiveAccess++;
		accessQueue->wait(lock, threadInfo->waitingQueueElem[id], [this](){ return &threadInfo->waitingQueueElem[id] == accessQueue->head && 0 == nbSharedLocked; } );
		nbWaitingExclusiveAccess--;
	}
	void waitForLockShared(std::unique_lock<M> &lock) const {
		accessQueue->wait(lock, threadInfo->waitingQueueElem[id], [this](){ return &threadInfo->waitingQueueElem[id] == accessQueue->head; } );
		accessQueue->removeFirstElementFromWaitingList();
		accessQueue->notifyFirstElem();
	}
	void EnsureMemoryAllocated() const {
		if (nullptr == threadInfo.get())
			threadInfo.reset(new ThreadInfo());
		if (id >= threadInfo->waitingQueueElem.size())
			threadInfo->waitingQueueElem.resize(id + 1, threadInfo.get());
	}
	void unmarkSharedOwnership() {
		threadInfo->waitingQueueElem[id].status = lock_status_t::NOT_LOCKED;
		nbSharedLocked--;
		if (0 == nbSharedLocked)
			accessQueue->notifyFirstElem();
	}
	void markSharedOwnership() {
		threadInfo->waitingQueueElem[id].status = lock_status_t::SHARED_LOCKED;
		nbSharedLocked++;
	}
	void unmarkOwnership() const {
		threadInfo->waitingQueueElem[id].status = lock_status_t::NOT_LOCKED;
		accessQueue->notifyFirstElem();
	}
	void markOwnership() const {
		threadInfo->waitingQueueElem[id].status = lock_status_t::LOCKED;
	}
	bool lockStatusEquals(lock_status_t status) const {
		return ((nullptr != threadInfo.get() && status == threadInfo->waitingQueueElem[id].status) ||
				(nullptr == threadInfo.get() && lock_status_t::NOT_LOCKED == status));
	}
	bool canBypassAccessQueueForSharedLock() const { return (0 == nbWaitingExclusiveAccess); }
};

template <typename M, typename C>
thread_local std::unique_ptr<struct SharedMutexTemplate<M, C>::ThreadInfo> SharedMutexTemplate<M, C>::threadInfo;

template <typename M, typename C>
const std::function<bool(M &mutex)> SharedMutexTemplate<M, C>::TryLockFunction = [](M &mutex) { return mutex.try_lock(); };

}

#endif
