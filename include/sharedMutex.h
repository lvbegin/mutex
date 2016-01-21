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


#ifndef SHARED_MUTEX_H__
#define SHARED_MUTEX_H__

#include <stdint.h>
#include <mutex>
#include <memory>
#include <recursiveMutexTemplate.h>

namespace std_mutex_extra {

class SharedMutex {
public:
	SharedMutex();
	~SharedMutex();

	SharedMutex(const SharedMutex &other) = delete;
	SharedMutex &operator=(const SharedMutex &other) = delete;
	SharedMutex(SharedMutex &&other) = delete;
	SharedMutex &operator=(SharedMutex &&other) = delete;

	void lock();
	void unlock();
	bool try_lock();
	void lock_shared();
	void unlock_shared();
	bool try_lock_shared();
private:
	struct NoStarvationQueue;
	struct ThreadInfo;

	std::mutex mutex;
	uint_fast16_t nbSharedLocked;
	uint_fast16_t nbWaitingExclusiveAccess;
	std::unique_ptr<NoStarvationQueue> accessQueue;
	static thread_local std::unique_ptr<ThreadInfo> threadInfo;

	bool canBypassAccessQueueForSharedLock();
	void waitForLockExclusive(std::unique_lock<std::mutex> &lock);
	void waitForLockShared(std::unique_lock<std::mutex> &lock);
	void markSharedOwnership();
	void EnsureMemoryAllocated();
	void unmarkSharedOwnership();
};

//typedef RecursiveSharedMutexTemplate<SharedMutex> RecursiveSharedMutex;

}

#endif
