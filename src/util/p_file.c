#include <stdio.h>
#include "enigma.h"

/**
 * e_file_exists
 *
 * Returns true if a file exists
 */
ENIGMA_API bool e_file_exists(const char *filename)
{
	FILE *f;
	f = fopen(filename, "r");
	if (f == NULL)
		return false;
	fclose(f);
	return true;
}

/**
 * e_file_get_size
 *
 * Returns the size of a file in bytes
 */
ENIGMA_API uint e_file_get_size(const char *filename)
{
	FILE *f;
	f = fopen(filename, "r");
	if (f == NULL)
	{
		e_log_message(E_LOG_WARNING, L"File", L"File %s cannot be read", filename);
		return 0;
	}
	fseek(f, 0, SEEK_END);
	uint size = ftell(f);
	fclose(f);
	return size;
}

/**
 * e_file_write
 *
 * Writes a file
 * returns true on success
 */
ENIGMA_API bool e_file_write(const char *filename, void *buffer, uint size)
{
	FILE *f;
	f = fopen(filename, "wb");
	if (f == NULL)
	{
		e_log_message(E_LOG_WARNING, L"File", L"File %s cannot be written to", filename);
		return false;
	}
	fwrite(buffer, size, 1, f);
	fclose(f);
	return true;
}

/**
 * e_file_read
 *
 * Reads a file
 */
ENIGMA_API bool e_file_read(const char *filename, void *buffer, uint size)
{
	FILE *f;
	f = fopen(filename, "rb");
	if (f == NULL)
	{
		e_log_message(E_LOG_WARNING, L"File", L"File %s cannot be read", filename);
		return false;
	}
	fread(buffer, size, 1, f);
	fclose(f);
	return true;
}

