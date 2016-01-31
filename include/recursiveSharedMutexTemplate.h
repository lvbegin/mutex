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

#include <recursiveMutexTemplate.h>
#include <sharedMutex.h>

namespace std_mutex_extra {

class RecursiveSharedMutexTemplate : public  RecursiveMutexTemplate {
public:
	template <typename M>
	static void lock_shared(unsigned int instanceId, M &mutex) {
		if (instanceId >= recursiveSharedAquire.size())
			recursiveSharedAquire.resize(instanceId + 1);
		if (notAlreadyAquiredShared(instanceId, recursiveSharedAquire, recursiveAquireCounters))
			mutex.lock_shared();
		recursiveSharedAquire[instanceId]++;
	}
	template <typename M>
	static void unlock_shared(unsigned int instanceId, M &mutex) {
		if (instanceId >= recursiveSharedAquire.size() || 0 == recursiveSharedAquire[instanceId])
			throw std::runtime_error("unlock a non-locked lock.");
		recursiveSharedAquire[instanceId]--;
		if (0 == recursiveSharedAquire[instanceId])
			mutex.unlock_shared();
	}
	template <typename M>
	static bool try_lock_shared(unsigned int instanceId, M &mutex) {
		if (instanceId >= recursiveSharedAquire.size())
			recursiveSharedAquire.resize(instanceId + 1);
		const auto locked = notAlreadyAquiredShared(instanceId, recursiveSharedAquire, recursiveAquireCounters) ? mutex.try_lock_shared() : true;
		if (locked)
			recursiveSharedAquire[instanceId]++;
		return locked;
	}
private:
	const unsigned int id;
	SharedMutex mutex;
	static thread_local std::vector<uint_fast16_t> recursiveSharedAquire;

	static bool notAlreadyAquiredShared(unsigned int instanceId, std::vector<uint_fast16_t> &recursiveSharedAquire,
													const std::vector<uint_fast16_t> &recursiveAquire) {
		return (0 == recursiveSharedAquire[instanceId] &&
				((instanceId >= recursiveAquire.size()) || 0 == recursiveAquire[instanceId]));
	}
};

}
#endif
