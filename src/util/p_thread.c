#include "platinum.h"

#ifdef PLATINUM_PLATFORM_WINDOWS

#include <windows.h>
typedef HANDLE PThread;
typedef DWORD PThreadResult;
typedef CRITICAL_SECTION PMutex;

#elif defined PLATINUM_PLATFORM_LINUX

#include <pthread.h>
#include <unistd.h>

struct PThread {
	pthread_t handle;
};

struct PMutex {
	pthread_mutex_t handle;
};

struct PThreadResult {
	void *result;
};

//typedef pthread_t PThread;
//typedef void *PThreadResult;
//typedef pthread_mutex_t PMutex;

#else // For outside library

typedef void *PThread;
typedef void *PThreadResult;
typedef void *PMutex;

#endif // PLATINUM_PLATFORM



/**
 * e_sleep
 *
 * wrapper function that sleeps for a number of miliseconds
 */
void p_sleep_ms(uint milis)
{
#ifdef PLATINUM_PLATFORM_WINDOWS
	Sleep(milis);
#endif
#ifdef PLATINUM_PLATFORM_LINUX
	sleep(milis/1000);
#endif // PLATINUM_PLATFORM_XXXXXX
}

/**
 * p_mutex_lock
 *
 * wrapper function around pthreads and winapithreads
 * that locks a mutex
 */
void p_mutex_lock(PMutex mutex)
{
#ifdef PLATINUM_PLATFORM_WINDOWS
	EnterCriticalSection(mutex);
#endif
#ifdef PLATINUM_PLATFORM_LINUX
	int result = pthread_mutex_lock(&mutex->handle);
	if (result != 0)
	{
		p_log_message(P_LOG_ERROR, L"Thread", L"Could not lock mutex. Error code: %i\n", result);
		exit(1);
	}
#endif
}

/**
 * p_mutex_unlock
 *
 * wrapper function around pthreads and winapithreads
 * that unlocks a mutex
 */
void p_mutex_unlock(PMutex mutex)
{
#ifdef PLATINUM_PLATFORM_WINDOWS
	LeaveCriticalSection(mutex);
#endif
#ifdef PLATINUM_PLATFORM_LINUX
	int result = pthread_mutex_unlock(&mutex->handle);
	if (result != 0)
	{
		p_log_message(P_LOG_ERROR, L"Thread", L"Could not unlock mutex. Error code: %i\n", result);
		exit(1);
	}
#endif
}

/**
 * p_mutex_init
 *
 * wrapper function around pthreads and winapithreads
 * that initializes a mutex
 */
PMutex p_mutex_init(void)
{
	PMutex mutex = malloc(sizeof *mutex);
#ifdef PLATINUM_PLATFORM_WINDOWS
	InitializeCriticalSection(mutex);
	if (mutex == NULL)
	{
		e_log_message(E_LOG_ERROR, L"Thread", L"Could not initiate mutex. Error code: %lu\n", GetLastError());
		exit(1);
	}
#endif
#ifdef PLATINUM_PLATFORM_LINUX
	int result = pthread_mutex_init(&mutex->handle, NULL);
	if (result != 0)
	{
		p_log_message(P_LOG_ERROR, L"Thread", L"Could not initiate mutex. Error code: %i\n", result);
		exit(1);
	}
#endif
	return mutex;
}

/**
 * p_mutex_destroy
 *
 * wrapper function around pthreads and winapithreads
 * that destroys a mutex
 */
void p_mutex_destroy(PMutex mutex)
{
#ifdef PLATINUM_PLATFORM_WINDOWS
	DeleteCriticalSection(mutex);
#endif
#ifdef PLATINUM_PLATFORM_LINUX
	int result = pthread_mutex_destroy(&mutex->handle);
	if (result != 0)
	{
		p_log_message(P_LOG_ERROR, L"Thread", L"Could not destroy mutex. Error code: %i\n", result);
		exit(1);
	}
	free(mutex);
#endif
}

/**
 * p_thread_create
 *
 * wrapper function around pthreads and winapithreads
 * that creates a new thread
 */
PThread p_thread_create(PThreadFunction func, PThreadArguments args)
{
	PThread thread = malloc(sizeof *thread);
#ifdef PLATINUM_PLATFORM_WINDOWS
	DWORD thread_id;
	// Create a thread on Windows
	thread = CreateThread(NULL, 0, func, args, 0, &thread_id);

	if (thread == NULL)
	{
		e_log_message(E_LOG_ERROR, L"Thread", L"Could not create thread. Error code: %lu\n", GetLastError());
		exit(1);
	}
	return thread;
#endif
#ifdef PLATINUM_PLATFORM_LINUX
	int result = pthread_create(&thread->handle, NULL, func, args);
	if (result != 0)
	{
		p_log_message(P_LOG_ERROR, L"Thread", L"Could not create thread. Error code: %i\n", result);
		exit(1);
	}
#endif
	return thread;
}

void p_thread_discard(PThread thread)
{
	free(thread);
}

/**
 * p_thread_self
 *
 * wrapper function around pthreads and winapithreads
 * that returns the current thread
 */
PThread p_thread_self(void)
{
#ifdef PLATINUM_PLATFORM_WINDOWS
	return GetCurrentThread();
#endif
#ifdef PLATINUM_PLATFORM_LINUX
	PThread thread = malloc(sizeof *thread);
	thread->handle = pthread_self();
	return thread;
#endif
}

/**
 * p_thread_join
 *
 * wrapper function around pthreads and winapithreads
 * that joins a thread
 */
void p_thread_join(PThread thread)
{
#ifdef PLATINUM_PLATFORM_WINDOWS
	DWORD result = WaitForSingleObject(thread, INFINITE);
	if (result == WAIT_FAILED || !CloseHandle(thread))
	{
		e_log_message(E_LOG_ERROR, L"Thread", L"Could not join thread. Error code: %lu\n", GetLastError());
		exit(1);
	}
#endif
#ifdef PLATINUM_PLATFORM_LINUX
	int result = pthread_join(thread->handle, NULL);
	if (result != 0)
	{
		p_log_message(P_LOG_ERROR, L"Thread", L"Could not join thread. Error code: %i\n", result);
		exit(1);
	}
#endif
	free(thread);
}

/**
 * p_thread_detach
 *
 * wrapper function around pthreads and winapithreads
 * that detaches a thread
 */
void p_thread_detach(PThread thread)
{
#ifdef PLATINUM_PLATFORM_WINDOWS
	// Windows threads are always detached
	P_UNUSED(thread);
#endif
#ifdef PLATINUM_PLATFORM_LINUX
	int result = pthread_detach(thread->handle);
	if (result != 0)
	{
		p_log_message(P_LOG_ERROR, L"Thread", L"Could not detach thread. Error code: %i\n", result);
		exit(1);
	}
#endif
	free(thread);
}
