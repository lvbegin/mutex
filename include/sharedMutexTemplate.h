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
#include <mutex>
#include <memory>
#include <recursiveMutexTemplate.h>
#include <thread>
#include <condition_variable>

namespace std_mutex_extra {

template <typename M, typename C>
class SharedMutexTemplate {
public:
	SharedMutexTemplate() : mutex(), nbSharedLocked(0), nbWaitingExclusiveAccess(0), accessQueue(new NoStarvationQueue()) { }
	~SharedMutexTemplate() = default;

	void lock() {
		std::unique_lock<M> lock(mutex);

		EnsureMemoryAllocated();
		waitForLockExclusive(lock);
		lock.release();
	}
	void unlock() {
		if (!accessQueue->headMatchesThreadId())
			throw std::runtime_error("thread tries to unlock a SharedMutex that it did not lock.");
		accessQueue->removeFirstElementFromWaitingList();
		mutex.unlock();
	}
	bool try_lock() { return try_lock(TryLockFunction); }
	void lock_shared() {
		std::unique_lock<M> lock(mutex);

		EnsureMemoryAllocated();
		if (!canBypassAccessQueueForSharedLock())
			waitForLockShared(lock);
		markSharedOwnership();
	}
	void unlock_shared() {
		if (nullptr == threadInfo.get() || !threadInfo->hasSharedLocked)
			throw std::runtime_error("thread tries to unlock a SharedMutex that it did not lock.");
		std::lock_guard<M> lock(mutex);

		unmarkSharedOwnership();
	}
	bool try_lock_shared() { return try_lock_shared(TryLockFunction); }

protected:
	bool try_lock(std::function<bool(M &mutex)> lockFunction)
	{
		if (!lockFunction(mutex))
			return false;
		const auto canKeepMutex = (0 == nbSharedLocked) && accessQueue->isEmpty();
		if (!canKeepMutex)
			mutex.unlock();
		else {
			EnsureMemoryAllocated();
			accessQueue->addInWaitingList(threadInfo->waitingQueueElem);
		}
		return canKeepMutex;
	}
	bool try_lock_shared(std::function<bool(M &mutex)> lockFunction) {
		if (!lockFunction(mutex))
			return false;
		const auto queueCanBeAvoidedWithoutStarvation = canBypassAccessQueueForSharedLock();
		if (queueCanBeAvoidedWithoutStarvation)
		{
			EnsureMemoryAllocated();
			markSharedOwnership();
		}
		mutex.unlock();
		return queueCanBeAvoidedWithoutStarvation;
	}

private:
	struct QueueElem {
		const std::thread::id id;
		QueueElem *next;
		C mutexCanBeLocked;

		QueueElem(std::thread::id id) : id(id), next(nullptr), mutexCanBeLocked() {}
		~QueueElem() = default;
	};

	struct ThreadInfo {
		struct QueueElem waitingQueueElem;
		bool hasSharedLocked;

		ThreadInfo(std::thread::id id) : waitingQueueElem(id), hasSharedLocked(false) {}
		~ThreadInfo() = default;
	};

	struct NoStarvationQueue {
		QueueElem *head;
		QueueElem *tail;

		NoStarvationQueue() : head(nullptr), tail(nullptr) {}
		~NoStarvationQueue() = default;
		bool isEmpty() { return nullptr == head; }
		bool headMatchesThreadId() { return std::this_thread::get_id() == head->id; }
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
			elem.mutexCanBeLocked.wait(locked, condition);
		}
		void notifyFirstElem() {
			if (!isEmpty())
				head->mutexCanBeLocked.notify_one();
		}
		void removeFirstElementFromWaitingList() {
			head = head->next;
			if (isEmpty())
				tail = nullptr;
			else
				head->mutexCanBeLocked.notify_one();
		}
	};

	M mutex;
	uint_fast16_t nbSharedLocked;
	uint_fast16_t nbWaitingExclusiveAccess;
	std::unique_ptr<NoStarvationQueue> accessQueue;
	static thread_local std::unique_ptr<ThreadInfo> threadInfo;
	static const std::function<bool(M &mutex)> TryLockFunction;

	void waitForLockExclusive(std::unique_lock<M> &lock)
	{
		nbWaitingExclusiveAccess++;
		accessQueue->wait(lock, threadInfo->waitingQueueElem, [this](){ return accessQueue->headMatchesThreadId() && 0 == nbSharedLocked; } );
		nbWaitingExclusiveAccess--;
	}

	void waitForLockShared(std::unique_lock<M> &lock)
	{
		accessQueue->wait(lock, threadInfo->waitingQueueElem, [this](){ return accessQueue->headMatchesThreadId(); } );
		accessQueue->removeFirstElementFromWaitingList();
	}

	void EnsureMemoryAllocated() {
		if (nullptr == threadInfo.get())
			threadInfo.reset(new ThreadInfo(std::this_thread::get_id()));
	}

	void unmarkSharedOwnership() {
		threadInfo->hasSharedLocked = false;
		nbSharedLocked--;
		if (0 == nbSharedLocked)
			accessQueue->notifyFirstElem();
	}

	void markSharedOwnership() {
		threadInfo->hasSharedLocked = true;
		nbSharedLocked++;
	}

	bool canBypassAccessQueueForSharedLock() { return (0 == nbWaitingExclusiveAccess); }

};

template <typename M, typename C>
thread_local std::unique_ptr<struct SharedMutexTemplate<M, C>::ThreadInfo> SharedMutexTemplate<M, C>::threadInfo;

template <typename M, typename C>
const std::function<bool(M &mutex)> SharedMutexTemplate<M, C>::TryLockFunction = [](M &mutex) { return mutex.try_lock(); };


}

#endif
