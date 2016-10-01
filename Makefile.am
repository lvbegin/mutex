
lib_LIBRARIES=libmutex.a
libmutex_a_SOURCES=src/recursiveMutexTemplate.cpp src/recursiveSharedMutexTemplate.cpp
include_HEADERS=include/conditionVariable.h include/lockGuard.h include/recursiveMutex.h include/recursiveSharedMutex.h include/recursiveSharedTimedMutex.h include/recursiveTimedMutex.h include/sharedMutex.h include/sharedTimedMutex.h include/templates/recursiveMutexTemplate.h include/templates/recursiveSharedMutexTemplate.h include/templates/sharedMutexTemplate.h
libmutex_a_CPPFLAGS=-I${srcdir}/include
TESTS=mutex_check
check_PROGRAMS=mutex_check
mutex_check_SOURCES=test/mutexTest.cpp
mutex_check_LDADD=-L${builddir} -lmutex -lpthread
mutex_check_CPPFLAGS=-I${srcdir}/include
