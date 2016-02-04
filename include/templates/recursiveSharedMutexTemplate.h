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

#ifndef RECURSIVE_SHARED_MUTEX_TEMPLATE_H__
#define RECURSIVE_SHARED_MUTEX_TEMPLATE_H__

#include <sharedMutex.h>
#include <templates/recursiveMutexTemplate.h>

namespace std_mutex_extra {

class RecursiveSharedMutexTemplate : public  RecursiveMutexTemplate {
public:
	template <typename M>
	static void lock_shared(unsigned int instanceId, M &mutex) {
		const auto &tryLockFunction = [](M &mutex) { mutex.lock_shared(); return true; };
		recursiveTryLockShared<M>(instanceId, mutex, std::move(tryLockFunction));
	}
	template <typename M>
	static void unlock_shared(unsigned int instanceId, M &mutex) {
		if (instanceId >= recursiveSharedAquire.size() || 0 == recursiveSharedAquire[instanceId])
			throw std::runtime_error("unlock a non-locked mutex.");
		recursiveSharedAquire[instanceId]--;
		if (0 == recursiveSharedAquire[instanceId])
			mutex.unlock_shared();
	}
	template <typename M>
	static bool try_lock_shared(unsigned int instanceId, M &mutex) {
		const auto &tryLockFunction = [](M &mutex) { return mutex.try_lock_shared(); };
		return recursiveTryLockShared<M>(instanceId, mutex, std::move(tryLockFunction));
	}
	template<typename M, typename Rep, typename Period>
	static bool try_lock_for_shared(unsigned int instanceId, M &mutex, const std::chrono::duration<Rep, Period>& timeout_duration ) {
		const auto &tryLockFunction = [timeout_duration](M &mutex) { return mutex.try_lock_for_shared(timeout_duration); };
		return recursiveTryLockShared<M>(instanceId, mutex, std::move(tryLockFunction));
	}
	template<typename M, typename Clock, typename Duration>
	static bool try_lock_until_shared(unsigned int instanceId, M &mutex, const std::chrono::time_point<Clock, Duration>& timeout_time ) {
		const auto &tryLockFunction = [timeout_time](M &mutex) { return mutex.try_lock_until_shared(timeout_time); };
		return recursiveTryLockShared<M>(instanceId, mutex, std::move(tryLockFunction));
	}
private:
	const unsigned int id;
	SharedMutex mutex;
	static thread_local std::vector<uint_fast16_t> recursiveSharedAquire;

	template <typename M>
	static bool recursiveTryLockShared(unsigned int instanceId, M &mutex, std::function<bool(M &mutex)> tryLockFunction) {
		if (instanceId >= recursiveSharedAquire.size())
			recursiveSharedAquire.resize(instanceId + 1);
		const auto locked = (0 == recursiveSharedAquire[instanceId]) ? tryLockFunction(mutex) : true;
		if (locked)
			recursiveSharedAquire[instanceId]++;
		return locked;
	}
};

}
#endif
