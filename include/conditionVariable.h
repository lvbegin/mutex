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

#ifndef CONDITION_VARIABLE_H__
#define CONDITION_VARIABLE_H__

#include <mutex>
#include <condition_variable>
#include <iostream>

namespace std_mutex_extra {

template <typename M>
class condition_variable {
public:
	void wait(std::unique_lock<M> &lock) {
		std::unique_lock<std::mutex> conditionLock(condition_mutex);
		const auto &waitFunction = [this] (std::unique_lock<std::mutex> &conditionLock) { condition.wait(conditionLock); return std::cv_status::no_timeout; };
		lock.unlock();
		waitAndUnlockConditionMutex(conditionLock, std::move(waitFunction));
		lock.lock();
	}
	template< class Rep, class Period >
	std::cv_status wait_for(std::unique_lock<M> &lock, const std::chrono::duration<Rep, Period>& rel_time) {
		std::unique_lock<std::mutex> conditionLock(condition_mutex);
		const auto &waitFunction = [this, rel_time] (std::unique_lock<std::mutex> &conditionLock) { return condition.wait_for(conditionLock, rel_time); };
		lock.unlock();
		const std::cv_status status = waitAndUnlockConditionMutex(conditionLock, std::move(waitFunction));
		lock.lock();
		return status;
	}
	template< class Rep, class Period >
	std::cv_status wait_for(std::unique_lock<M> &lock, const std::chrono::duration<Rep, Period>& rel_time, std::function<bool()> pred) {
		return wait_until(lock, std::chrono::steady_clock::now() + rel_time, std::move(pred));
	}

	template< class Clock, class Duration >
	std::cv_status wait_until(std::unique_lock<M> &lock, const std::chrono::time_point<Clock, Duration>& timeout_time) {
		std::unique_lock<std::mutex> conditionLock(condition_mutex);
		const auto &waitFunction = [this, timeout_time] (std::unique_lock<std::mutex> &conditionLock) { return condition.wait_until(conditionLock, timeout_time); };
		lock.unlock();
		const std::cv_status status = waitAndUnlockConditionMutex(conditionLock, std::move(waitFunction));
		lock.lock();
		return status;
	}
	template< class Clock, class Duration >
	std::cv_status wait_for(std::unique_lock<M> &lock, const std::chrono::time_point<Clock, Duration>& timeout_time, std::function<bool()> pred) {
		while (!pred()) {
			if (std::cv_status::timeout == wait_for(lock, timeout_time))
				return std::cv_status::timeout;
		}
	}

	void wait(std::unique_lock<M> &lock, std::function<bool()> pred) {
		while (!pred())
			wait(lock);
	}
	void notify_one() {
		std::lock_guard<std::mutex> conditionLock(condition_mutex);
		condition.notify_one();
	}
	void notify_all() {
		std::lock_guard<std::mutex> conditionLock(condition_mutex);
		condition.notify_all();
	}
private:
	std::condition_variable condition;
	std::mutex condition_mutex;

	std::cv_status waitAndUnlockConditionMutex(std::unique_lock<std::mutex> &conditionLock,
								std::function<std::cv_status(std::unique_lock<std::mutex> &)> waitFunction) {
		/* we unlock here the condition mutex to avoid deadlocks.
		 * For that, we must always follow the following order when acquiring mutexes:
		 * lock the client mutex, then lock the condition mutex.
		 */
		const std::cv_status status = waitFunction(conditionLock);
		conditionLock.unlock();
		return status;
	}
};

}

#endif
