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

#include <recursiveSharedMutex.h>

namespace std_mutex_extra {

thread_local std::vector<uint_fast16_t> RecursiveSharedMutex::recursiveSharedAquire;

RecursiveSharedMutex::RecursiveSharedMutex() = default;
RecursiveSharedMutex::~RecursiveSharedMutex() = default;

void RecursiveSharedMutex::lock_shared() {
	const auto instanceId = RecursiveMutexTemplate<SharedMutex>::instanceId;
	const auto &recursiveAquire = RecursiveMutexTemplate<SharedMutex>::recursiveAquire;

	if (instanceId >= recursiveSharedAquire.size())
		recursiveSharedAquire.resize(instanceId + 1);
	if (0 == recursiveSharedAquire[instanceId] &&
			(instanceId < recursiveAquire.size() && 0 == recursiveAquire[instanceId]))
		RecursiveMutexTemplate<SharedMutex>::mutex.lock_shared();
	recursiveSharedAquire[instanceId]++;
}

void RecursiveSharedMutex::unlock_shared() {
	const auto instanceId = RecursiveMutexTemplate<SharedMutex>::instanceId;

	if (instanceId >= recursiveSharedAquire.size() || 0 == recursiveSharedAquire[instanceId])
		throw std::runtime_error("unlock a non-locked lock.");
	recursiveSharedAquire[instanceId]--;
	if (0 == recursiveSharedAquire[instanceId])
		RecursiveMutexTemplate<SharedMutex>::mutex.unlock_shared();
}

bool RecursiveSharedMutex::try_lock_shared() {
	const auto instanceId = RecursiveMutexTemplate<SharedMutex>::instanceId;
	auto &mutex = RecursiveMutexTemplate<SharedMutex>::mutex;

	if (instanceId >= recursiveSharedAquire.size())
		recursiveSharedAquire.resize(instanceId + 1);
	const auto locked = (0 == recursiveSharedAquire[instanceId]) ? mutex.try_lock_shared() : true;
	if (locked)
		recursiveSharedAquire[instanceId]++;
	return locked;
}

}
