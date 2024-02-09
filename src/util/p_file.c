#include <stdio.h>
#include "platinum.h"

/**
 * p_file_exists
 *
 * Returns true if a file exists
 */
bool p_file_exists(const char *filename)
{
	FILE *f;
	f = fopen(filename, "r");
	if (f == NULL)
		return false;
	fclose(f);
	return true;
}

/**
 * p_file_get_size
 *
 * Returns the size of a file in bytes
 */
uint p_file_get_size(const char *filename)
{
	FILE *f;
	f = fopen(filename, "r");
	if (f == NULL)
	{
		p_log_message(P_LOG_WARNING, L"File", L"File %s cannot be read", filename);
		return 0;
	}
	fseek(f, 0, SEEK_END);
	uint size = ftell(f);
	fclose(f);
	return size;
}

/**
 * p_file_write
 *
 * Writes a file
 * returns true on success
 */
bool p_file_write(const char *filename, void *buffer, uint size)
{
	FILE *f;
	f = fopen(filename, "wb");
	if (f == NULL)
	{
		p_log_message(P_LOG_WARNING, L"File", L"File %s cannot be written to", filename);
		return false;
	}
	fwrite(buffer, size, 1, f);
	fclose(f);
	return true;
}

/**
 * p_file_read
 *
 * Reads a file
 */
bool p_file_read(const char *filename, void *buffer, uint size)
{
	FILE *f;
	f = fopen(filename, "rb");
	if (f == NULL)
	{
		p_log_message(P_LOG_WARNING, L"File", L"File %s cannot be read", filename);
		return false;
	}
	fread(buffer, size, 1, f);
	fclose(f);
	return true;
}

