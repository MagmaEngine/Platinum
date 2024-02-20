#ifndef PLATINUM_UTIL_H
#define PLATINUM_UTIL_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <wchar.h>

#ifndef NULL
#define NULL (void*)(0)
#endif

#ifndef _UINT
#define _UINT
typedef unsigned int uint;
#endif // _UINT

#ifndef P_UNUSED
#define P_UNUSED(x) (void)(x)
#endif

#ifdef PLATINUM_DOUBLE_PRECISION
typedef double PFloat;
#else
typedef float PFloat;
#endif // PLATINUM_DOUBLE_PRECISION

// ------------ Integer math --------------
#ifndef P_MAX
#define P_MAX(a, b) ((a) > (b) ? (a) : (b))
#endif // P_MAX
#ifndef P_MIN
#define P_MIN(a, b) ((a) < (b) ? (a) : (b))
#endif // P_MIN

// ----------- Dynamic Array -----------
#define P_PTR_CAST(type, val) &(type){val}
#define P_VPTR_CAST(type, val) (void *)&(type){val}

typedef struct PDynarr PDynarr;

PDynarr *p_dynarr_init(const size_t item_size, const uint item_cap);
PDynarr *p_dynarr_init_arr(const size_t item_size, const uint item_cap, const void * const arr);
void p_dynarr_deinit(PDynarr * const p);
void p_dynarr_add(PDynarr * const p, const void * const item);
void p_dynarr_append(PDynarr * const dest, const PDynarr * const src);
int p_dynarr_set(PDynarr * const p, const uint index, const void * const item);
int p_dynarr_find(const PDynarr * const d, const void * const item);
int p_dynarr_remove_unordered(PDynarr * const p, const uint index);
int p_dynarr_remove_ordered(PDynarr * const p, const uint index);
uint p_dynarr_count(const PDynarr * const p);
size_t p_dynarr_item_size(const PDynarr * const p);
void *p_dynarr_get_arr(const PDynarr * const p);
#define p_dynarr_get(dynarr, type, index) ((type *)p_dynarr_get_arr(dynarr))[index]

// ------------- Threads ---------------
struct PThread;
struct PMutex;
//struct PThreadResult;

typedef struct PThread *PThread;
typedef struct PMutex *PMutex;
typedef void *PThreadResult;
typedef void *PThreadArguments;
typedef PThreadResult (*PThreadFunction)(void *);

// ------------ Logging -------------
enum PLogLevel {
	P_LOG_DEBUG,
	P_LOG_INFO,
	P_LOG_WARNING,
	P_LOG_ERROR,
	P_LOG_MAX
};

void p_log_message(enum PLogLevel level, const wchar_t *channel, const wchar_t *format, ...);

// ------------ File IO --------------
bool p_file_exists(const char *filename);
uint p_file_get_size(const char *filename);
bool p_file_write(const char *filename, void *buffer, uint size);
bool p_file_read(const char *filename, void *buffer, uint size);


void p_sleep_ms(uint milis);
void p_mutex_lock(PMutex mutex);
void p_mutex_unlock(PMutex mutex);
PMutex p_mutex_init(void);
void p_mutex_destroy(PMutex mutex);

PThread p_thread_create(PThreadFunction func, PThreadArguments args);
PThread p_thread_self(void);
void p_thread_detach(PThread thread);
void p_thread_discard(PThread thread);
void p_thread_join(PThread thread);

/* ----- Debugging -----
If PLATINUM_DEBUG_MEMORY is enabled, the memory debugging system will create macros that replace malloc, free and realloc and allows the system to keep track of and report where memory is beeing allocated, how much and if the memory is beeing freed. This is very useful for finding memory leaks in large applications. The system can also over allocate memory and fill it with a magic number and can therfor detect if the application writes outside of the allocated memory. if PLATINUM_EXIT_CRASH is defined, then exit(); will be replaced with a funtion that writes to NULL. This will make it trivial ti find out where an application exits using any debugger., */


void p_debug_memory_init(PMutex *mutex); /* Required for memory debugger to be thread safe */
void *p_debug_mem_malloc(size_t size, char *file, uint line); /* Replaces malloc and records the c file and line where it was called*/
void *p_debug_mem_calloc(size_t nmemb, size_t size, char *file, uint line); /* Replaces malloc and records the c file and line where it was called*/
void *p_debug_mem_realloc(void *pointer, size_t size, char *file, uint line); /* Replaces realloc and records the c file and line where it was called*/
void p_debug_mem_free(void *buf); /* Replaces free and records the c file and line where it was called*/
void p_debug_mem_print(uint min_allocs); /* Prints out a list of all allocations made, their location, how much memorey each has allocated, freed, and how many allocations have been made. The min_allocs parameter can be set to avoid printing any allocations that have been made fewer times then min_allocs */
void p_debug_mem_reset(void); /* f_debug_mem_reset allows you to clear all memory stored in the debugging system if you only want to record allocations after a specific point in your code*/
bool p_debug_memory(void); /*f_debug_memory checks if any of the bounds of any allocation has been over written and reports where to standard out. The function returns TRUE if any error was found*/

#ifdef PLATINUM_DEBUG_MEMORY

#define malloc(n) p_debug_mem_malloc(n, __FILE__, __LINE__) /* Replaces malloc. */
#define calloc(n, m) p_debug_mem_calloc(n, m, __FILE__, __LINE__) /* Replaces malloc. */
#define realloc(n, m) p_debug_mem_realloc(n, m, __FILE__, __LINE__) /* Replaces realloc. */
#define free(n) p_debug_mem_free(n) /* Replaces free. */

#endif // PLATINUM_DEBUG_MEMORY

// Crash on exit.
#ifdef PLATINUM_EXIT_CRASH
void exit_crash(uint i);
#define exit(n) exit_crash(n);
#endif // PLATINUM_EXIT_CRASH

#endif // PLATINUM_UTIL_H
