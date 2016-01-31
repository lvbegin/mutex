#include <recursiveMutexTemplate.h>

namespace std_mutex_extra {

thread_local std::vector<uint_fast16_t> RecursiveMutexTemplate::recursiveAquireCounters;

}
