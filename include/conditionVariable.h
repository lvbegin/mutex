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
		lock.unlock();
		waitAndUnlockConditionMutex(conditionLock);
		lock.lock();
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

	void waitAndUnlockConditionMutex(std::unique_lock<std::mutex> &conditionLock) {
		/* we unlock here the condition mutex to avoid deadlocks.
		 * For that, we must always follow the following order when acquiring mutexes:
		 * lock the client mutex, then lock the condition mutex.
		 */
		condition.wait(conditionLock);
		conditionLock.unlock();
	}
};

}

#endif
