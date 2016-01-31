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

#ifndef RECURSIVE_SHARED_TIMED_MUTEX_H__
#define RECURSIVE_SHARED_TIMED_MUTEX_H__

#include <sharedTimedMutex.h>
#include <recursiveMutex.h>

namespace std_mutex_extra {

class RecursiveSharedTimedMutex {
public:
	RecursiveSharedTimedMutex() : id(RecursiveSharedMutexTemplate::newId()) {}
	~RecursiveSharedTimedMutex() = default;

	void lock() {  RecursiveSharedMutexTemplate::lock<SharedTimedMutex>(id, mutex); }
	void unlock() {  RecursiveSharedMutexTemplate::unlock<SharedTimedMutex>(id, mutex); }
	bool try_lock() {  return RecursiveSharedMutexTemplate::try_lock<SharedTimedMutex>(id, mutex); }
	template <typename Rep, typename Period>
	bool try_lock_for( const std::chrono::duration<Rep, Period>& timeout_duration ) { return RecursiveSharedMutexTemplate::try_lock_for<SharedTimedMutex, Rep, Period>(id, mutex, timeout_duration); }
	template <typename Clock, typename Duration>
	bool try_lock_until( const std::chrono::time_point<Clock, Duration>& timeout_time ) { return RecursiveSharedMutexTemplate::try_lock_until<SharedTimedMutex, Clock, Duration>(id, mutex, timeout_time); }
	void lock_shared() { RecursiveSharedMutexTemplate::lock_shared<SharedTimedMutex>(id, mutex); }
	void unlock_shared() { RecursiveSharedMutexTemplate::unlock_shared<SharedTimedMutex>(id, mutex); }
	bool try_lock_shared() { return RecursiveSharedMutexTemplate::try_lock_shared<SharedTimedMutex>(id, mutex); }
	template <typename Rep, typename Period>
	bool try_lock_for_shared( const std::chrono::duration<Rep, Period>& timeout_duration ) {  return RecursiveSharedMutexTemplate::try_lock_for_shared<SharedTimedMutex, Rep, Period>(id, mutex, timeout_duration); }
	template <typename Clock, typename Duration>
	bool try_lock_until_shared( const std::chrono::time_point<Clock, Duration>& timeout_time ) { return RecursiveSharedMutexTemplate::try_lock_until_shared<SharedTimedMutex, Clock, Duration>(id, mutex, timeout_time); }
private:
	const unsigned int id;
	SharedTimedMutex mutex;
};

}
#endif
