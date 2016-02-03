This file contains the following section:
- project purpose
- project content
- compilation
---------------------------------------------
Project Purpose
---------------------------------------------
This project contains implementations of mutexes that may be missing in some environment. One of the goal is also to provide classes that are as safe as possible. That is, compared to mutexes dvelopped in the STL, undefined behaviour are removed as much as possible. For instance, unlocking a mutex that has not been locked results in throwing an exception.
Those classes are designed to compile with c++11 compliant compiler.

---------------------------------------------
Project Content
---------------------------------------------

This project contains various implementations of recursive and shared (timed) locks:
- recursive mutex: The class RecursiveMutex is declared in the file RecursiveMutex.h. That class is similar to the class std::recursive_mutex defined in the c++11 standard. Contrary to the class std::mutex, the behavior of the class RecursiveMutex is defined when unlock() is called on a unlocked mutex. In that case, a runtime_error exception is thrown. 


- shared mutex: The class SharedMutex is declared in the file SharedMutex.h. That class defines the same API than the class std::shared_mutex (c++17). Some cases where the behaviour is undefined when using the class std::shared_mutex (c++17) is defined when using the class SharedMutex:
- when trying to lock (with exclusive or shared ownership) a SharedMutex by a thread that already owns it, an exception is thrown.
- when unlock() is called by a thread on a mutex that is not locked with exclusive ownership by that thread, an exception is thrown.
- when unlock_shared() is called by a thread on a mutex that is not locked with exclusive ownership by that thread, an exception is thrown.

- recursive shared mutex: The class RecursiveSharedMutex is declared in the file recursiveSharedMutex.h. That class defines the same API than the class std::shared_mutex (c++17). Again, undefined behaviour of the class std::shared_mutex is replaced by a throw of std::runtime_error exception.

-recursive timed mutex: The class RecursiveTimedMutex is declared in the file recursiveTimedMutex.h. This class is similar to the class std::recursive_timed_mutex (c++11). The cases where the behaviour of std::recursive_timed_mutex is undefined are replaced by throws of std::runtime_error exception.

- shared timed mutex: The class SharedTimedMutex is declared in the file sharedTimedMutex.h. That class is similar to the class std::shared_timed_mutex (c++14).

-recursive shared timed mutex: The class RecursiveSharedTimedMutex is declared in the file recursiveSharedTimedMutex.h. That class is similar to the class std::recursive_shared_timed_mutex. That class combines the behavior of the shared timed mutex and recursive timed mutex.

---------------------------------------------
Compilation
---------------------------------------------

Compiling files and building binaries do not need specific any flag, library, or header. You should thus be able to compile and build library/binary with any problem. If you encounter some problem, please let me know.  
