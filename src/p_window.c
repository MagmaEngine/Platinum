#include "platinum.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

// Forward function declarations for internal functions

#ifdef PLATINUM_DISPLAY_WAYLAND
void _wayland_window_close(PWindowData *window_data);
#elif defined PLATINUM_DISPLAY_X11
void _x11_window_close(PWindowData *window_data);
#elif defined PLATINUM_DISPLAY_WIN32
void _win32_window_close(PWindowData *window_data);
#endif // PLATINUM_DISPLAY

/**
 * p_window_create
 *
 * creates a window with parameters set from window_request.
 * adds the window_data associated with the window to app_data.
 */
void p_window_create(PAppData *app_data, const PWindowRequest window_request)
{
#ifdef PLATINUM_DISPLAY_WAYLAND
	p_wayland_window_create(app_data, window_request);
#elif defined PLATINUM_DISPLAY_X11
	p_x11_window_create(app_data, window_request);
#elif defined PLATINUM_DISPLAY_WIN32
	p_win32_window_create(app_data, window_request);
#endif // PLATINUM_PLATFORM
}

/**
 * p_window_close
 *
 * sends a signal to close the window and the connection to the Xserver
 */
void p_window_close(PWindowData *window_data)
{
	window_data->status = P_WINDOW_STATUS_INTERNAL_CLOSE;
#ifdef PLATINUM_DISPLAY_WAYLAND
	p_wayland_window_close(window_data);
#elif defined PLATINUM_DISPLAY_X11
	p_x11_window_close(window_data);
#elif defined PLATINUM_DISPLAY_WIN32
	p_win32_window_close(window_data);
#endif // PLATINUM_PLATFORM
}


/**
 * _window_close
 *
 * internal close function that actually closes the window and frees data
 */
void _window_close(PAppData *app_data, PWindowData *window_data)
{
	// If window exists delete it.
	p_mutex_lock(app_data->window_mutex);
	int index = p_dynarr_find(app_data->window_data, &window_data);
	if (index == -1)
	{
		p_log_message(P_LOG_ERROR, L"Platinum", L"Window does not exist...");
		exit(1);
	}
	p_graphics_display_destroy(window_data->graphical_display_data);

#ifdef PLATINUM_DISPLAY_WAYLAND
	_wayland_window_close(window_data);
#elif defined PLATINUM_DISPLAY_X11
	_x11_window_close(window_data);
#elif defined PLATINUM_DISPLAY_WIN32
	_win32_window_close(window_data);
#endif // PLATINUM_PLATFORM

	free(window_data->display_info);
	if (window_data->status == P_WINDOW_STATUS_CLOSE)
	{
		free(window_data->event_calls);
		free(window_data->name);
		free(window_data);
		p_thread_detach(p_thread_self());
	}
	p_dynarr_remove_unordered(app_data->window_data, index);
	p_mutex_unlock(app_data->window_mutex);
}

/**
 * p_window_fullscreen
 *
 * sets the window in window_data to fullscreen
 */
void p_window_fullscreen(PWindowData *window_data)
{
#ifdef PLATINUM_DISPLAY_WAYLAND
	p_wayland_window_fullscreen(window_data);
#elif defined PLATINUM_DISPLAY_X11
	p_x11_window_fullscreen(window_data);
#elif defined PLATINUM_DISPLAY_WIN32
	p_win32_window_fullscreen(window_data);
#endif // PLATINUM_PLATFORM

}

/**
 * p_window_docked_fullscreen
 *
 * sets the window in window_data to (borderless/windowed) fullscreen
 */
void p_window_docked_fullscreen(PWindowData *window_data)
{
#ifdef PLATINUM_DISPLAY_WAYLAND
	p_wayland_window_docked_fullscreen(window_data);
#elif defined PLATINUM_DISPLAY_X11
	p_x11_window_docked_fullscreen(window_data);
#elif defined PLATINUM_DISPLAY_WIN32
	p_win32_window_docked_fullscreen(window_data);
#endif // PLATINUM_PLATFORM
}

/**
 * p_window_windowed
 *
 * sets the window in window_data to windowed mode and sets the dimensions
 */
void p_window_windowed(PWindowData *window_data, uint x, uint y, uint width, uint height)
{
#ifdef PLATINUM_DISPLAY_WAYLAND
	p_wayland_window_windowed(window_data, x, y, width, height);
#elif defined PLATINUM_DISPLAY_X11
	p_x11_window_windowed(window_data, x, y, width, height);
#elif defined PLATINUM_DISPLAY_WIN32
	p_win32_window_windowed(window_data, x, y, width, height);
#endif // PLATINUM_PLATFORM
}

/**
 * p_window_set_dimensions
 *
 * updates window_data with the values provided as x, y, width, and heigth
 * and submits a request to the WM to resize the window
 */
void p_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height)
{
#ifdef PLATINUM_DISPLAY_WAYLAND
	p_wayland_window_set_dimensions(display_info, x, y, width, height);
#elif defined PLATINUM_DISPLAY_X11
	p_x11_window_set_dimensions(display_info, x, y, width, height);
#elif defined PLATINUM_DISPLAY_WIN32
	p_win32_window_set_dimensions(display_info, x, y, width, height);
#endif // PLATINUM_PLATFORM
}

/**
 * p_window_set_name
 *
 * sets the name of the window
*/
void p_window_set_name(PDisplayInfo *display_info, const wchar_t *name)
{
#ifdef PLATINUM_DISPLAY_WAYLAND
	p_wayland_window_set_name(display_info, name);
#elif defined PLATINUM_DISPLAY_X11
	p_x11_window_set_name(display_info, name);
#elif defined PLATINUM_DISPLAY_WIN32
	p_win32_window_set_name(display_info, name);
#endif // PLATINUM_PLATFORM
}

/**
 * p_window_set_icon
 *
 * sets the icon of the window
 */
void p_window_set_icon(PDisplayInfo *display_info, uint32_t *icon)
{
	// TODO: implement me
//#ifdef PLATINUM_DISPLAY_WAYLAND
//	p_wayland_window_set_icon(display_info, icon);
//#elif defined PLATINUM_DISPLAY_X11
//	p_x11_window_set_icon(display_info, icon);
//#elif defined PLATINUM_DISPLAY_WIN32
//	p_win32_window_set_icon(display_info, icon);
//#endif // PLATINUM_PLATFORM
}


/**
 * p_window_set_graphical_display
 *
 * adds a graphical surface to the window
 */
void p_window_set_graphical_display(PWindowData *window_data, PGraphicalAppData graphical_app_data,
		PGraphicalDisplayData graphical_display_data)
{
#ifdef PLATINUM_DISPLAY_WAYLAND
	p_wayland_window_set_graphical_display(window_data, graphical_app_data, graphical_display_data);
#elif defined PLATINUM_DISPLAY_X11
	p_x11_window_set_graphical_display(window_data, graphical_app_data, graphical_display_data);
#elif defined PLATINUM_DISPLAY_WIN32
	p_win32_window_set_graphical_display(window_data, graphical_app_data, graphical_display_data);
#endif // PLATINUM_PLATFORM
}
