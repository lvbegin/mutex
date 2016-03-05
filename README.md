This file contains the following section:
- project purpose
- project content
- compilation
---------------------------------------------
Project Purpose
---------------------------------------------
This project contains implementations of mutexes that may be missing in some development environment. One of the goal is also to provide classes that are as safe as possible. That is, compared to mutexes dvelopped in the STL, undefined behaviour are removed as much as possible.
Those classes are designed to compile with c++11 compliant compiler.

---------------------------------------------
Project Content
---------------------------------------------

This project contains various implementations of recursive and shared (timed) mutexes:

- recursive mutex: The class RecursiveMutex is declared in the file RecursiveMutex.h. That class is similar to the class std::recursive_mutex defined in the c++11 standard. Contrary to the class std::mutex, the behavior of the class RecursiveMutex is defined when unlock() is called on a unlocked mutex. In that case, a runtime_error exception is thrown. 

- shared mutex: The class SharedMutex is declared in the file SharedMutex.h. That class defines the same API than the class std::shared_mutex (c++17). Undefined behaviour of the class std::shared_mutex is replaced by throwing of std::runtime_error exception. In particular, an exception is thrown when a mutex is locked twice by the same thread (exclusive and/or shared ownership), when a thread tries to unlock (exclusive or shard ownership) a mutex that it did not lock or it locked with the other ownership.

- recursive shared mutex: The class RecursiveSharedMutex is declared in the file recursiveSharedMutex.h. That class defines the same API than the class std::recursive_shared_mutex (c++17). Again, undefined behaviour of the class std::shared_mutex is replaced by throwing of std::runtime_error exception. In particular, an exception is thrown when a mutex is locked twice by the same thread with a different ownership, when a thread tries to unlock (exclusive or shard ownership) a mutex that it did not lock, or it locked with the other ownership.

- recursive timed mutex: The class RecursiveTimedMutex is declared in the file recursiveTimedMutex.h. This class is similar to the class std::recursive_timed_mutex (c++11). The cases where the behaviour of std::recursive_timed_mutex is undefined are replaced by throwing of std::runtime_error exception. In particular, an exception is thrown when a thread tries to unlock a mutex that it did not lock before.

- shared timed mutex: The class SharedTimedMutex is declared in the file sharedTimedMutex.h. That class is similar to the class std::shared_timed_mutex (c++14). The cases where the behaviour of std::shared_timed_mutex is undefined are replaced by throwing of std::runtime_error exception. In particular, an exception is thrown when a mutex is locked twice by the same thread (exclusive and/or shared ownership), when a thread tries to unlock (exclusive or shard ownership) a mutex that it did not lock or it locked with the other ownership. 

- recursive shared timed mutex: The class RecursiveSharedTimedMutex is declared in the file recursiveSharedTimedMutex.h. That class combines the behavior of the shared timed mutex and recursive timed mutex. Again, undefined behaviors of the class std::recursive_shared_timed_mutex are replaced by throwing a std::runtime_error exception. 


This project also contains an implementation of classes that uses mutex:

- condition_variable: The class is similar to the class std::condition_variable but it can be used with mutexes that are instances of classes other than std::mutex. It is defined in the file conditionVariable.h.

- lock_guard: The class is similar to the class std::lock_guard and allows locking several mutexes at once avoiding deadlocks (similar to the definition given in c++17). The class is defined in the file lockGuard.h.  

---------------------------------------------
Compilation
---------------------------------------------

Compiling files and building binaries do not need specific flag, library, or header. You should thus be able to compile and build library/binary without any problem. If you encounter some problem, please let me know.  
