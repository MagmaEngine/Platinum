#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include "enigma.h"

#ifndef E_MAX_LOG_LENGTH
#define E_MAX_LOG_LENGTH 1024
#endif // E_MAX_LOG_LENGTH

#ifdef ENIGMA_PLATFORM_LINUX
#define E_LOG_COLOR_RESET "\x1b[0m"
#define E_LOG_COLOR_RED "\x1b[31m"
#define E_LOG_COLOR_YELLOW "\x1b[33m"
#define E_LOG_COLOR_GREEN "\x1b[32m"
#define E_LOG_COLOR_BLUE "\x1b[34m"
#elif defined ENIGMA_PLATFORM_WINDOWS
#define E_LOG_COLOR_RESET FOREGROUND_INTENSITY
#define E_LOG_COLOR_RED FOREGROUND_RED
#define E_LOG_COLOR_YELLOW FOREGROUND_RED | FOREGROUND_GREEN
#define E_LOG_COLOR_GREEN FOREGROUND_GREEN
#define E_LOG_COLOR_BLUE FOREGROUND_BLUE
#endif // ENIGMA_PLATFORM_XXXXXX

// Log function
ENIGMA_API void e_log_message(enum ELogLevel level, const wchar_t *channel, const wchar_t *format, ...) {
	va_list args;
	va_start(args, format);

	wchar_t buffer[E_MAX_LOG_LENGTH];
	vswprintf(buffer, sizeof(buffer)/sizeof(buffer[0]), format, args);

	va_end(args);

	wchar_t *log_level;
#ifdef ENIGMA_PLATFORM_LINUX
	char *color;
#elif defined ENIGMA_PLATFORM_WINDOWS
	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD color;
#endif // ENIGMA_PLATFORM_XXXXXX
	switch (level) {
		case E_LOG_DEBUG:
			log_level = L"DEBUG";
			color = E_LOG_COLOR_BLUE;
			break;
		case E_LOG_INFO:
			log_level = L"INFO";
			color = E_LOG_COLOR_GREEN;
			break;
		case E_LOG_WARNING:
			log_level = L"WARNING";
			color = E_LOG_COLOR_YELLOW;
			break;
		case E_LOG_ERROR:
			log_level = L"ERROR";
			color = E_LOG_COLOR_RED;
			break;
		default:
			log_level = L"UNKNOWN";
			color = E_LOG_COLOR_RESET;
			break;
	}

#ifdef ENIGMA_PLATFORM_LINUX
	wprintf(L"%s[%ls] %ls: %ls%s\n", color, log_level, channel, buffer, E_LOG_COLOR_RESET);
#elif defined ENIGMA_PLATFORM_WINDOWS
	SetConsoleTextAttribute(console_handle, color);
	wprintf(L"[%ls] %ls: %ls\n", log_level, channel, buffer);
	SetConsoleTextAttribute(console_handle, E_LOG_COLOR_RESET);
#endif // ENIGMA_PLATFORM_XXXXXX
}

