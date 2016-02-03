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


#ifndef __SHARED_TIMED_MUTEX_H__
#define __SHARED_TIMED_MUTEX_H__

#include <conditionVariable.h>
#include <templates/sharedMutexTemplate.h>
#include <mutex>

namespace std_mutex_extra {

class SharedTimedMutex {
public:
	SharedTimedMutex() = default;
	~SharedTimedMutex() = default;

	void lock() { share.lock(mutex); }
	void unlock() { share.unlock(mutex); }
	bool try_lock() { return share.try_lock(mutex); }
	template <typename Rep, typename Period>
	bool try_lock_for( const std::chrono::duration<Rep, Period>& timeout_duration ) {
		return share.try_lock_for<Rep, Period>(mutex, timeout_duration);
	}
	template <typename Clock, typename Duration>
	bool try_lock_until( const std::chrono::time_point<Clock, Duration>& timeout_time ) {
		return share.try_lock_until(mutex, timeout_time);
	}
	void lock_shared() { share.lock_shared(mutex); }
	void unlock_shared() { share.unlock_shared(mutex); }
	bool try_lock_shared() { return share.try_lock_shared(mutex); }
	template <typename Rep, typename Period>
	bool try_lock_for_shared( const std::chrono::duration<Rep, Period>& timeout_duration ) {
		return share.try_lock_for_shared<Rep, Period>(mutex, timeout_duration);
	}
	template <typename Clock, typename Duration>
	bool try_lock_until_shared( const std::chrono::time_point<Clock, Duration>& timeout_time ) {
		return share.try_lock_until_shared(mutex, timeout_time);
	}
private:
	std::timed_mutex mutex;
	SharedMutexTemplate<std::timed_mutex, std_mutex_extra::condition_variable<std::timed_mutex>> share;
};

}

#endif
