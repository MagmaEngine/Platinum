#include "enigma.h"

/**
 * e_sleep
 *
 * wrapper function that sleeps for a number of miliseconds
 */
ENIGMA_API void e_sleep_ms(uint milis)
{
#ifdef ENIGMA_PLATFORM_WINDOWS
	Sleep(milis);
#endif
#ifdef ENIGMA_PLATFORM_LINUX
	usleep(milis*1000);
#endif // ENIGMA_PLATFORM_XXXXXX
}

/**
 * e_mutex_lock
 *
 * wrapper function around pthreads and winapithreads
 * that locks a mutex
 */
ENIGMA_API void e_mutex_lock(EMutex *mutex)
{
#ifdef ENIGMA_PLATFORM_WINDOWS
	EnterCriticalSection(mutex);
#endif
#ifdef ENIGMA_PLATFORM_LINUX
	int result = pthread_mutex_lock(mutex);
	if (result != 0)
	{
		e_log_message(E_LOG_ERROR, L"Thread", L"Could not lock mutex. Error code: %i\n", result);
		exit(1);
	}
#endif
}

/**
 * e_mutex_unlock
 *
 * wrapper function around pthreads and winapithreads
 * that unlocks a mutex
 */
ENIGMA_API void e_mutex_unlock(EMutex *mutex)
{
#ifdef ENIGMA_PLATFORM_WINDOWS
	LeaveCriticalSection(mutex);
#endif
#ifdef ENIGMA_PLATFORM_LINUX
	int result = pthread_mutex_unlock(mutex);
	if (result != 0)
	{
		e_log_message(E_LOG_ERROR, L"Thread", L"Could not unlock mutex. Error code: %i\n", result);
		exit(1);
	}
#endif
}

/**
 * e_mutex_init
 *
 * wrapper function around pthreads and winapithreads
 * that initializes a mutex
 */
ENIGMA_API void e_mutex_init(EMutex *mutex)
{
#ifdef ENIGMA_PLATFORM_WINDOWS
	InitializeCriticalSection(mutex);
	if (mutex == NULL)
	{
		e_log_message(E_LOG_ERROR, L"Thread", L"Could not initiate mutex. Error code: %lu\n", GetLastError());
		exit(1);
	}
#endif
#ifdef ENIGMA_PLATFORM_LINUX
	int result = pthread_mutex_init(mutex, NULL);
	if (result != 0)
	{
		e_log_message(E_LOG_ERROR, L"Thread", L"Could not initiate mutex. Error code: %i\n", result);
		exit(1);
	}
#endif
}

/**
 * e_mutex_destroy
 *
 * wrapper function around pthreads and winapithreads
 * that destroys a mutex
 */
ENIGMA_API void e_mutex_destroy(EMutex *mutex)
{
#ifdef ENIGMA_PLATFORM_WINDOWS
	DeleteCriticalSection(mutex);
#endif
#ifdef ENIGMA_PLATFORM_LINUX
	int result = pthread_mutex_destroy(mutex);
	if (result != 0)
	{
		e_log_message(E_LOG_ERROR, L"Thread", L"Could not destroy mutex. Error code: %i\n", result);
		exit(1);
	}
#endif
}

/**
 * e_thread_create
 *
 * wrapper function around pthreads and winapithreads
 * that creates a new thread
 */
ENIGMA_API EThread e_thread_create(EThreadFunction func, EThreadArguments args)
{
	EThread thread;
#ifdef ENIGMA_PLATFORM_WINDOWS
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
#ifdef ENIGMA_PLATFORM_LINUX
	int result = pthread_create(&thread, NULL, func, args);
	if (result != 0)
	{
		e_log_message(E_LOG_ERROR, L"Thread", L"Could not create thread. Error code: %i\n", result);
		exit(1);
	}
#endif
	return thread;
}

/**
 * e_thread_self
 *
 * wrapper function around pthreads and winapithreads
 * that returns the current thread
 */
ENIGMA_API EThread e_thread_self(void)
{
#ifdef ENIGMA_PLATFORM_WINDOWS
	return GetCurrentThread();
#endif
#ifdef ENIGMA_PLATFORM_LINUX
	return pthread_self();
#endif
}

/**
 * e_thread_join
 *
 * wrapper function around pthreads and winapithreads
 * that joins a thread
 */
ENIGMA_API void e_thread_join(EThread thread)
{
#ifdef ENIGMA_PLATFORM_WINDOWS
	DWORD result = WaitForSingleObject(thread, INFINITE);
	if (result == WAIT_FAILED || !CloseHandle(thread))
	{
		e_log_message(E_LOG_ERROR, L"Thread", L"Could not join thread. Error code: %lu\n", GetLastError());
		exit(1);
	}
#endif
#ifdef ENIGMA_PLATFORM_LINUX
	int result = pthread_join(thread, NULL);
	if (result != 0)
	{
		e_log_message(E_LOG_ERROR, L"Thread", L"Could not join thread. Error code: %i\n", result);
		exit(1);
	}
	return;
#endif
}

/**
 * e_thread_detach
 *
 * wrapper function around pthreads and winapithreads
 * that detaches a thread
 */
ENIGMA_API void e_thread_detach(EThread thread)
{
#ifdef ENIGMA_PLATFORM_WINDOWS
	// Windows threads are always detached
	E_UNUSED(thread);
#endif
#ifdef ENIGMA_PLATFORM_LINUX
	int result = pthread_detach(thread);
	if (result != 0)
	{
		e_log_message(E_LOG_ERROR, L"Thread", L"Could not detach thread. Error code: %i\n", result);
		exit(1);
	}
	return;
#endif
}
