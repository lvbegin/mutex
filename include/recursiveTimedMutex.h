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

#ifndef RECURSIVE_TIMED_MUTEX_H__
#define RECURSIVE_TIMED_MUTEX_H__

#include <recursiveMutexTemplate.h>

#include <mutex>

namespace std_mutex_extra {

class RecursiveTimedMutex : public RecursiveMutexTemplate<std::timed_mutex> {
public:
	RecursiveTimedMutex() = default;
	~RecursiveTimedMutex() = default;

	template<typename Rep, typename Period>
	bool try_lock_for( const std::chrono::duration<Rep, Period>& timeout_duration ) {
		const auto instanceId = RecursiveMutexTemplate<std::timed_mutex>::instanceId;
		auto &recursiveAquire = RecursiveMutexTemplate<std::timed_mutex>::recursiveAquire;
		auto &mutex = RecursiveMutexTemplate<std::timed_mutex>::mutex;

		if (instanceId >= recursiveAquire.size())
			recursiveAquire.resize(instanceId + 1);
		const auto locked = (0 == recursiveAquire[instanceId]) ? mutex.try_lock_for(timeout_duration) : true;
		if (locked)
			recursiveAquire[instanceId]++;
		return locked;
	}
	template<typename Clock, typename Duration>
	bool try_lock_until( const std::chrono::time_point<Clock, Duration>& timeout_time ) {
		const auto instanceId = RecursiveMutexTemplate<std::timed_mutex>::instanceId;
		auto &recursiveAquire = RecursiveMutexTemplate<std::timed_mutex>::recursiveAquire;
		auto &mutex = RecursiveMutexTemplate<std::timed_mutex>::mutex;

		if (instanceId >= recursiveAquire.size())
			recursiveAquire.resize(instanceId + 1);
		const auto locked = (0 == recursiveAquire[instanceId]) ? mutex.try_lock_until(timeout_time) : true;
		if (locked)
			recursiveAquire[instanceId]++;
		return locked;
	}
};

}

#endif
