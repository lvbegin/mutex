This file contains the following section:
- project purpose
- project content
- compilation
---------------------------------------------
Project Purpose
---------------------------------------------
This project contains implementations of mutex that are missing in the c++11 standard. One of the goal is also to provide some classes that are as safe as possible. That is, unlocking a mutex that has not been locked is catched and result in throwing an exception.

---------------------------------------------
Project Content
---------------------------------------------

This project contains an implementation of recursive and shared locks for c++11:
- recursive mutex: The class RecursiveMutex is declared in the file RecursiveMutex.h. That class defines the same API than the class std::mutex:

     - lock() : locks the mutex. The mutex can be locked by one thread at a time. Other threads are blocked until the mutex is unlocked. If the thread calls lock() on a RecursiveMutex several time, it will not block.
     - unlock() : unlocks the mutex. A RecursiveMutex stay locked by a thread until that threads calls the unlock() method the same number of time than the lock() method.
     - try_lock() : locks the mutex. Contrary to the method lock(), try_lock() is non-blocking. If the mutex is not already locked, then try_lock() locks the mutex and returns true. Otherwise, the method direcly returns false. 

Contrary to the class std::mutex, the behavior of the class RecursiveMutex is fully defined when unlock() is called on a unlocked mutex. In that case, a runtime_error exception is thrown. 


- shared mutex: The class SharedMutex is declared in the file SharedMutex.h. That class defines the same API than the class std::shared_mutex (c++17). 
     - lock() : locks the mutex. The mutex can be locked by one thread at a time. Other threads are blocked until the mutex is unlocked. 
     - unlock() : unlocks the mutex.
     - try_lock() : locks the mutex. Contrary to the method lock(), a call to try_lock() is non-blocking. If the mutex is not already locked, then try_lock() locks the mutex and returns true. Otherwise, the method direcly returns false. 
     - lock_shared(): locks the mutex for shared ownership. Several threads can acquire the shared ownership of the mutex. If a thread has already locked the mutex (with lock()), then lock_shared blocks until the mutex is released (by calling unlock()).
     - unlock_shared(): unlocks the mutex (shared ownership). If a thread that called locks() while the mutex is locked with shared ownership blocks until all the threads that locked the mutex with shared ownership release it by calling unlock_shared().
     - try_lock_shared(): locks the mutex (shared ownership). Contrary to the method lock_shared(), that method is non blocking. If the mutex is not already locked with a call to lock(), then try_lock_shared() locks the mutex with shared ownership and returns true. Otherwise, the method direcly returns false. 

Contrary to the class std::shared_mutex (c++17), the behavior is fully defined when the method unlock()/unlock_shared() is called on a SharedMutex which is not locked/locked with shared ownership. In that case, a runtime_error exception is thrown. 
 
- recursive shared mutex: The class RecursiveSharedMutex is declared in the file SharedMutex.h. That class defines the same API than the class SharedMutex but, contrary to the class SharedMutex, it allows a thread to lock (via lock()/try_lock/lock_shared()/try_lock_shared()) even if the mutex is already locked by the thread. It is allowed for a thread to call lock_shared() on a RecursiveSharedMutex while the thread already called lock() on that mutex. In that case, the call to lock_shared() will not block, but the mutex will remain locked with unique ownership (although unlock_shared must be called to fully release the mutex). However, it is not allowed for a thread to call lock() after calling lock_shared() on the same RecursiveSharedMutex (withour calling unlock_shared() in between).  
As for the class RecursiveMutex, the behavior of the class RecursiveSharedMutex is fully defined when unlock() or unlock_shared() is called on a unlocked mutex. In that case, a runtime_error exception is thrown.


---------------------------------------------
Compilation
---------------------------------------------

Compiling files and building binaries do not need specific any flag, library, or header. You should thus be able to compile and build library/binary with any problem. If you encounter some problem, please let me know.  
