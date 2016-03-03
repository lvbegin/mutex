#ifndef LOCK_GUARD_H__
#define LOCK_GUARD_H__

#include <mutex>
#include <functional>

namespace std_mutex_extra {


template< typename... MutexTypes >
class lock_guard {
public:
	lock_guard(MutexTypes & ... m) : unlocker([&m...]() { unlock(m...); }) {
		std::lock(m...);
	};
	~lock_guard() { unlocker(); }
private:
	std::function<void(void)> unlocker;
	template <typename T, typename ...T2>
	static void unlock(T &m, T2 &...tail) { m.unlock(); unlock(tail...); }
	static void unlock() { }
};

}

#endif
