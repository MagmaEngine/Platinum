#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include "platinum.h"

#ifndef P_LOG_MAX_LENGTH
#define P_LOG_MAX_LENGTH 1024
#endif // P_LOG_MAX_LENGTH

#ifdef PLATINUM_PLATFORM_LINUX

#define P_LOG_COLOR_RESET "\x1b[0m"
#define P_LOG_COLOR_RED "\x1b[31m"
#define P_LOG_COLOR_YELLOW "\x1b[33m"
#define P_LOG_COLOR_GREEN "\x1b[32m"
#define P_LOG_COLOR_BLUE "\x1b[34m"

#elif defined PLATINUM_PLATFORM_WINDOWS

#define P_LOG_COLOR_RESET FOREGROUND_INTENSITY
#define P_LOG_COLOR_RED FOREGROUND_RED
#define P_LOG_COLOR_YELLOW FOREGROUND_RED | FOREGROUND_GREEN
#define P_LOG_COLOR_GREEN FOREGROUND_GREEN
#define P_LOG_COLOR_BLUE FOREGROUND_BLUE

#endif // PLATINUM_PLATFORM

// Log function
void p_log_message(enum PLogLevel level, const wchar_t *channel, const wchar_t *format, ...) {
	va_list args;
	va_start(args, format);

	wchar_t buffer[P_LOG_MAX_LENGTH];
	vswprintf(buffer, sizeof(buffer)/sizeof(buffer[0]), format, args);

	va_end(args);

	wchar_t *log_level;
#ifdef PLATINUM_PLATFORM_LINUX

	char *color;

#elif defined PLATINUM_PLATFORM_WINDOWS

	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD color;

#endif // PLATINUM_PLATFORM
	switch (level) {
		case P_LOG_DEBUG:
			log_level = L"DEBUG";
			color = P_LOG_COLOR_BLUE;
			break;
		case P_LOG_INFO:
			log_level = L"INFO";
			color = P_LOG_COLOR_GREEN;
			break;
		case P_LOG_WARNING:
			log_level = L"WARNING";
			color = P_LOG_COLOR_YELLOW;
			break;
		case P_LOG_ERROR:
			log_level = L"ERROR";
			color = P_LOG_COLOR_RED;
			break;
		default:
			log_level = L"UNKNOWN";
			color = P_LOG_COLOR_RESET;
			break;
	}

#ifdef PLATINUM_PLATFORM_LINUX
	wprintf(L"%s[%ls] %ls: %ls%s\n", color, log_level, channel, buffer, P_LOG_COLOR_RESET);
#elif defined PLATINUM_PLATFORM_WINDOWS
	SetConsoleTextAttribute(console_handle, color);
	wprintf(L"[%ls] %ls: %ls\n", log_level, channel, buffer);
	SetConsoleTextAttribute(console_handle, P_LOG_COLOR_RESET);
#endif // PLATINUM_PLATFORM
}

