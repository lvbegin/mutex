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


#include <sharedMutex.h>

#include <vector>
#include <thread>
#include <condition_variable>

namespace std_mutex_extra {

struct QueueElem {
	const std::thread::id id;
	QueueElem *next;
	std::condition_variable mutexCanBeLocked;

	QueueElem(std::thread::id id) : id(id), next(nullptr), mutexCanBeLocked() {}
	~QueueElem() = default;
};

struct SharedMutex::NoStarvationQueue {
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
	void wait(std::unique_lock<std::mutex> &locked, QueueElem &elem, std::function<bool()> condition) {
		addInWaitingList(elem);
		while (!condition())
			elem.mutexCanBeLocked.wait(locked);
	}
	void notifyFirstElem() {
		if (!isEmpty())
			head->mutexCanBeLocked.notify_one();
	}
	void removeFirstElementFromWaitingList() {
		head = head->next;
		if (nullptr == head)
			tail = nullptr;
	}
};

thread_local std::unique_ptr<QueueElem> queueElem;

SharedMutex::SharedMutex() : mutex(), nbSharedLocked(0), nbWaitingExclusiveAccess(0), accessQueue(new NoStarvationQueue()) { }

SharedMutex::~SharedMutex() = default;

void SharedMutex::lock() {
	std::unique_lock<std::mutex> lock(mutex);

	waitForLockExclusive(lock);
	lock.release();
}

void SharedMutex::waitForLockExclusive(std::unique_lock<std::mutex> &lock)
{
	nbWaitingExclusiveAccess++;
	if (nullptr == queueElem.get())
		queueElem.reset(new QueueElem(std::this_thread::get_id()));
	accessQueue->wait(lock, *queueElem, [this](){ return accessQueue->headMatchesThreadId() && 0 == nbSharedLocked; } );
	accessQueue->removeFirstElementFromWaitingList();
	nbWaitingExclusiveAccess--;
}


void SharedMutex::unlock() {
	accessQueue->notifyFirstElem();
	mutex.unlock();
}

bool SharedMutex::try_lock() {
	if (false == mutex.try_lock())
		return false;
	const auto canKeepMutex = (0 == nbSharedLocked);
		if (!canKeepMutex)
			mutex.unlock();
	return canKeepMutex;
}

void SharedMutex::lock_shared() {
	std::unique_lock<std::mutex> lock(mutex);

	if (!canBypassAccessQueueForSharedLock())
		waitForLockShared(lock);
	nbSharedLocked++;
}

void SharedMutex::waitForLockShared(std::unique_lock<std::mutex> &lock)
{
	if (nullptr == queueElem.get())
		queueElem.reset(new QueueElem(std::this_thread::get_id()));
	accessQueue->wait(lock, *queueElem, [this](){ return accessQueue->headMatchesThreadId(); } );
	accessQueue->removeFirstElementFromWaitingList();
	accessQueue->notifyFirstElem();

}

void SharedMutex::unlock_shared() {
	std::lock_guard<std::mutex> lock(mutex);

	nbSharedLocked--;
	if (0 == nbSharedLocked)
		accessQueue->notifyFirstElem();
}

bool SharedMutex::try_lock_shared() {
	if (false == mutex.try_lock())
		return false;
	const auto queueCanBeAvoidedWithoutStarvation = canBypassAccessQueueForSharedLock();
	if (queueCanBeAvoidedWithoutStarvation)
		nbSharedLocked++;
	mutex.unlock();
	return queueCanBeAvoidedWithoutStarvation;
}

bool SharedMutex::canBypassAccessQueueForSharedLock() {
	return (0 == nbWaitingExclusiveAccess);
}

}
