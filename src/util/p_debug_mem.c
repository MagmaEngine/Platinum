#include <stdlib.h>
#include <string.h>
#include "platinum.h"

extern void p_debug_mem_print(uint min_allocs);

#define P_MEM_OVER_ALLOC 32
#define P_MEM_MAGIC_NUMBER 132

typedef struct{
	uint size;
	void *buf;
}STMemAllocBuf;

typedef struct{
	uint line;
	char file[256];
	STMemAllocBuf *allocs;
	uint alloc_count;
	uint alloc_alocated;
	uint size;
	uint alocated;
	uint freed;
}STMemAllocLine;

STMemAllocLine p_alloc_lines[1024];
uint p_alloc_line_count = 0;
void *p_alloc_mutex = NULL;
void (*p_alloc_mutex_lock)(PMutex mutex) = NULL;
void (*p_alloc_mutex_unlock)(PMutex mutex) = NULL;


void p_debug_memory_init(PMutex *mutex)
{
#ifdef PLATINUM_DEBUG_MEMORY
	p_alloc_mutex = mutex;
	p_alloc_mutex_lock = p_mutex_lock;
	p_alloc_mutex_unlock = p_mutex_unlock;
#else
	E_UNUSED(mutex);
#endif
}

bool p_debug_memory(void)
{
	bool output = false;
#ifdef PLATINUM_DEBUG_MEMORY
	uint i, j, k;
	if (p_alloc_mutex != NULL)
		p_alloc_mutex_lock(p_alloc_mutex);
	for (i = 0; i < p_alloc_line_count; i++)
	{
		for (j = 0; j < p_alloc_lines[i].alloc_count; j++)
		{
			uint8_t *buf;
			uint size;
			buf = p_alloc_lines[i].allocs[j].buf;
			size = p_alloc_lines[i].allocs[j].size;
			for (k = 0; k < P_MEM_OVER_ALLOC; k++)
				if (buf[size + k] != P_MEM_MAGIC_NUMBER)
					break;
			if (k < P_MEM_OVER_ALLOC)
			{
				p_log_message(P_LOG_ERROR, L"Memory", L"Overshoot at line %u in file %s\n", p_alloc_lines[i].line,
						p_alloc_lines[i].file);
				{
					uint *X = NULL;
					X[0] = 0;
				}
				output = true;
			}
		}
	}
	if (p_alloc_mutex != NULL)
		p_alloc_mutex_unlock(p_alloc_mutex);
#endif
	return output;
}

void p_debug_mem_add(void *pointer, uint size, char *file, uint line)
{
	uint i, j;
	for (i = 0; i < P_MEM_OVER_ALLOC; i++)
		((uint8_t *)pointer)[size + i] = P_MEM_MAGIC_NUMBER;

	for (i = 0; i < p_alloc_line_count; i++)
	{
		if (line == p_alloc_lines[i].line)
		{
			for (j = 0; file[j] != 0 && file[j] == p_alloc_lines[i].file[j] ; j++);
			if (file[j] == p_alloc_lines[i].file[j])
				break;
		}
	}
	if (i < p_alloc_line_count)
	{
		if (p_alloc_lines[i].alloc_alocated == p_alloc_lines[i].alloc_count)
		{
			p_alloc_lines[i].alloc_alocated += 1024;
			p_alloc_lines[i].allocs = (realloc)(p_alloc_lines[i].allocs, (sizeof *p_alloc_lines[i].allocs) * p_alloc_lines[i].alloc_alocated);
		}
		p_alloc_lines[i].allocs[p_alloc_lines[i].alloc_count].size = size;
		p_alloc_lines[i].allocs[p_alloc_lines[i].alloc_count++].buf = pointer;
		p_alloc_lines[i].size += size;
		p_alloc_lines[i].alocated++;
	} else
	{
		if (i < 1024)
		{
			p_alloc_lines[i].line = line;
			for (j = 0; j < 255 && file[j] != 0; j++)
				p_alloc_lines[i].file[j] = file[j];
			p_alloc_lines[i].file[j] = 0;
			p_alloc_lines[i].alloc_alocated = 256;
			p_alloc_lines[i].allocs = (malloc)((sizeof *p_alloc_lines[i].allocs) * p_alloc_lines[i].alloc_alocated);
			p_alloc_lines[i].allocs[0].size = size;
			p_alloc_lines[i].allocs[0].buf = pointer;
			p_alloc_lines[i].alloc_count = 1;
			p_alloc_lines[i].size = size;
			p_alloc_lines[i].freed = 0;
			p_alloc_lines[i].alocated++;
			p_alloc_line_count++;
		}
	}
}

void *p_debug_mem_malloc(size_t size, char *file, uint line)
{
	void *pointer;
	if (p_alloc_mutex != NULL)
		p_alloc_mutex_lock(p_alloc_mutex);
	pointer = (malloc)(size + P_MEM_OVER_ALLOC);

	if (pointer == NULL)
	{
		p_log_message(P_LOG_ERROR, L"Memory",
				L"Malloc returns NULL when trying to allocate %zu bytes at line %u in file %s\n", size, line, file);
		if (p_alloc_mutex != NULL)
			p_alloc_mutex_unlock(p_alloc_mutex);
		p_debug_mem_print(0);
		exit(0);
	}
	memset(pointer, P_MEM_MAGIC_NUMBER+1, size + P_MEM_OVER_ALLOC);
	p_debug_mem_add(pointer, size, file, line);
	if (p_alloc_mutex != NULL)
		p_alloc_mutex_unlock(p_alloc_mutex);
	return pointer;
}

void *p_debug_mem_calloc(size_t nmemb, size_t size, char *file, uint line)
{
	void *pointer;
	if (p_alloc_mutex != NULL)
		p_alloc_mutex_lock(p_alloc_mutex);
	pointer = (malloc)((nmemb * size) + P_MEM_OVER_ALLOC);

	if (pointer == NULL)
	{
		p_log_message(P_LOG_ERROR, L"Memory",
				L"Calloc returns NULL when trying to allocate %zu bytes at line %u in file %s\n", size, line, file);
		if (p_alloc_mutex != NULL)
			p_alloc_mutex_unlock(p_alloc_mutex);
		p_debug_mem_print(0);
		exit(0);
	}
	memset(pointer, P_MEM_MAGIC_NUMBER+1, nmemb * size + P_MEM_OVER_ALLOC);
	memset(pointer, 0, nmemb * size);
	p_debug_mem_add(pointer, size, file, line);
	if (p_alloc_mutex != NULL)
		p_alloc_mutex_unlock(p_alloc_mutex);
	return pointer;
}

bool p_debug_mem_remove(void *buf)
{
	uint i, j, k;
	for (i = 0; i < p_alloc_line_count; i++)
	{
		for (j = 0; j < p_alloc_lines[i].alloc_count; j++)
		{
			if (p_alloc_lines[i].allocs[j].buf == buf)
			{
				for (k = 0; k < P_MEM_OVER_ALLOC; k++)
					if (((uint8_t *)buf)[p_alloc_lines[i].allocs[j].size + k] != P_MEM_MAGIC_NUMBER)
						break;
				if (k < P_MEM_OVER_ALLOC)
					p_log_message(P_LOG_ERROR, L"Memory", L"Overshoot at line %u in file %s\n", p_alloc_lines[i].line,
							p_alloc_lines[i].file);
				p_alloc_lines[i].size -= p_alloc_lines[i].allocs[j].size;
				p_alloc_lines[i].allocs[j] = p_alloc_lines[i].allocs[--p_alloc_lines[i].alloc_count];
				p_alloc_lines[i].freed++;
				return true;
			}
		}
	}
	return false;
}

void p_debug_mem_free(void *buf)
{
	if (p_alloc_mutex != NULL)
		p_alloc_mutex_lock(p_alloc_mutex);
	if (!p_debug_mem_remove(buf))
	{
		uint *X = NULL;
		X[0] = 0;
	}
	(free)(buf);
	if (p_alloc_mutex != NULL)
		p_alloc_mutex_unlock(p_alloc_mutex);
}


void *p_debug_mem_realloc(void *pointer, size_t size, char *file, uint line)
{
	uint i, j, k, move;
	void *pointer2;
	if (pointer == NULL)
		return p_debug_mem_malloc( size, file, line);

	if (p_alloc_mutex != NULL)
		p_alloc_mutex_lock(p_alloc_mutex);
	for (i = 0; i < p_alloc_line_count; i++)
	{
		for (j = 0; j < p_alloc_lines[i].alloc_count; j++)
			if (p_alloc_lines[i].allocs[j].buf == pointer)
				break;
		if (j < p_alloc_lines[i].alloc_count)
			break;
	}
	if (i == p_alloc_line_count)
	{
		p_log_message(P_LOG_ERROR, L"Memory",
				L"Trying to reallocate pointer %p in %s line %u. Pointer has never beein allocated\n", pointer, file,
				line);
		for (i = 0; i < p_alloc_line_count; i++)
		{
			for (j = 0; j < p_alloc_lines[i].alloc_count; j++)
			{
				uint *buf;
				buf = p_alloc_lines[i].allocs[j].buf;
				for (k = 0; k < p_alloc_lines[i].allocs[j].size; k++)
				{
					if (&buf[k] == pointer)
					{
						p_log_message(P_LOG_ERROR, L"Memory",
								L"Trying to reallocate pointer %u bytes (out of %u) in to allocation made in %s on line %u.\n",
								k, p_alloc_lines[i].allocs[j].size, p_alloc_lines[i].file, p_alloc_lines[i].line);
					}
				}
			}
		}
		exit(0);
	}
	move = p_alloc_lines[i].allocs[j].size;

	if (move > size)
		move = size;

	pointer2 = (malloc)(size + P_MEM_OVER_ALLOC);
	if (pointer2 == NULL)
	{
		p_log_message(P_LOG_ERROR, L"Memory",
				L"Realloc returns NULL when trying to allocate %zu bytes at line %u in file %s\n", size, line, file);
		if (p_alloc_mutex != NULL)
			p_alloc_mutex_unlock(p_alloc_mutex);
		p_debug_mem_print(0);
		exit(0);
	}
	for (i = 0; i < size + P_MEM_OVER_ALLOC; i++)
		((uint8_t *)pointer2)[i] = P_MEM_MAGIC_NUMBER + 1;
	memcpy(pointer2, pointer, move);

	p_debug_mem_add(pointer2, size, file, line);
	p_debug_mem_remove(pointer);
	(free)(pointer);

	if (p_alloc_mutex != NULL)
		p_alloc_mutex_unlock(p_alloc_mutex);
	return pointer2;
}

void p_debug_mem_print(uint min_allocs)
{
#ifdef PLATINUM_DEBUG_MEMORY
	uint i;
	if (p_alloc_mutex != NULL)
		p_alloc_mutex_lock(p_alloc_mutex);
	p_log_message(P_LOG_DEBUG, L"Memory",L"----------------------------------------------");
	for (i = 0; i < p_alloc_line_count; i++)
	{
		if (min_allocs < p_alloc_lines[i].alocated)
		{
			p_log_message(P_LOG_DEBUG, L"Memory", L"%s line: %u",p_alloc_lines[i].file, p_alloc_lines[i].line);
			p_log_message(P_LOG_DEBUG, L"Memory", L"    - Bytes allocated: %u", p_alloc_lines[i].size);
			p_log_message(P_LOG_DEBUG, L"Memory", L"    - Allocations:     %u", p_alloc_lines[i].alocated);
			p_log_message(P_LOG_DEBUG, L"Memory", L"    - Frees:           %u", p_alloc_lines[i].freed);
		}
	}
	p_log_message(P_LOG_DEBUG, L"Memory",L"----------------------------------------------");
	if (p_alloc_mutex != NULL)
		p_alloc_mutex_unlock(p_alloc_mutex);
#else
	E_UNUSED(min_allocs);
#endif
}

uint32_t p_debug_mem_consumption(void)
{
#ifdef PLATINUM_DEBUG_MEMORY
	uint i, sum = 0;

	if (p_alloc_mutex != NULL)
		p_alloc_mutex_lock(p_alloc_mutex);
	for (i = 0; i < p_alloc_line_count; i++)
		sum += p_alloc_lines[i].size;
	if (p_alloc_mutex != NULL)
		p_alloc_mutex_unlock(p_alloc_mutex);
	return sum;
#else
	return 0;
#endif
}

void p_debug_mem_reset(void)
{
#ifdef PLATINUM_DEBUG_MEMORY
	uint i;
	if (p_alloc_mutex != NULL)
		p_alloc_mutex_lock(p_alloc_mutex);
	for (i = 0; i < p_alloc_line_count; i++)
		(free)(p_alloc_lines[i].allocs);
	p_alloc_line_count = 0;

	if (p_alloc_mutex != NULL)
		p_alloc_mutex_unlock(p_alloc_mutex);
#endif
}

void exit_crash(uint i)
{
	uint *a = NULL;
	a[0] = i;
}
