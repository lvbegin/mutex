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

#ifndef RECURSIVE_MUTEX_TEMPLATE_H__
#define RECURSIVE_MUTEX_TEMPLATE_H__

#include <stdint.h>
#include <atomic>
#include <vector>
#include <stdexcept>
#include <chrono>

namespace std_mutex_extra {

template <typename L>
class RecursiveMutexTemplate {
public:
	RecursiveMutexTemplate() : instanceId(newId()) { }
	~RecursiveMutexTemplate() = default;
	
	void lock() {
		if (instanceId >= recursiveAquire.size())
			recursiveAquire.resize(instanceId + 1);
		if (0 == recursiveAquire[instanceId])
			mutex.lock();
		recursiveAquire[instanceId]++;
	}
	void unlock() {
		if (instanceId >= recursiveAquire.size() || 0 == recursiveAquire[instanceId])
			throw std::runtime_error("unlock a non-locked lock.");
		recursiveAquire[instanceId]--;
		if (0 == recursiveAquire[instanceId])
			mutex.unlock();
	}
	bool try_lock() {
		if (instanceId >= recursiveAquire.size())
			recursiveAquire.resize(instanceId + 1);
		const auto locked = (0 == recursiveAquire[instanceId]) ? mutex.try_lock() : true;
		if (true == locked)
			recursiveAquire[instanceId]++;
		return locked;
	}
protected:
	const unsigned int instanceId;
	static thread_local std::vector<uint_fast16_t> recursiveAquire;
	L mutex;
private:
	unsigned int newId() {
		static std::atomic<unsigned int> nbInstances { 0 };
		return nbInstances++;
	}
};

template<typename L>
thread_local std::vector<uint_fast16_t> RecursiveMutexTemplate<L>::recursiveAquire;

}



#endif
