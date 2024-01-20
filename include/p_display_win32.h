#ifndef _PHANTOM_DISPLAY_WIN32_H
#define _PHANTOM_DISPLAY_WIN32_H

#include "p_api.h"
#include <windows.h>
#include <wchar.h>

/**
 * PDisplayInfo
 *
 * This struct holds all the low-level display information
 * Values here should never be set directly
 */
struct PDisplayInfo{
	HWND hwnd;
	HINSTANCE hInstance;
	HBRUSH hBrush;
	wchar_t *class_name;
	uint screen_width;
	uint screen_height;
};

// Window
#define p_window_create(p_app_data, p_window_request) \
	p_win32_window_create(p_app_data, p_window_request)
#define p_window_close(p_window_data) \
	p_win32_window_close(p_window_data)
#define p_window_fullscreen(p_window_data) \
	p_win32_window_fullscreen(p_window_data)
#define p_window_docked_fullscreen(p_window_data) \
	p_win32_window_docked_fullscreen(p_window_data)
#define p_window_windowed(p_window_data, x, y, width, height) \
	p_win32_window_windowed(p_window_data, x, y, width, height)
#define p_window_set_dimensions(p_display_info, x, y, width, height) \
	p_win32_window_set_dimensions(p_display_info, x, y, width, height)
#define p_window_set_name(p_display_info, name) \
	p_win32_window_set_name(p_display_info, name)

PHANTOM_API void p_win32_window_create(PAppData *app_data, const PWindowRequest window_request);
PHANTOM_API void p_win32_window_close(PWindowData *window_data);
PHANTOM_API void p_win32_window_fullscreen(PWindowData *window_data);
PHANTOM_API void p_win32_window_docked_fullscreen(PWindowData *window_data);
PHANTOM_API void p_win32_window_windowed(PWindowData *window_data, uint x, uint y, uint width, uint height);
PHANTOM_API void p_win32_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height);
PHANTOM_API void p_win32_window_set_name(PDisplayInfo *display_info, const wchar_t *name);

#endif // _PHANTOM_DISPLAY_WIN32_H
