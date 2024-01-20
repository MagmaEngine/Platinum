#ifndef _PHANTOM_DISPLAY_X11_H
#define _PHANTOM_DISPLAY_X11_H

#include "p_api.h"
#include <xcb/xcb.h>
#include <wchar.h>

/**
 * PAtomTypes
 *
 * This enum is used to initialize the display_info atoms
 * for quick lookup later
 */
enum PAtomTypes {
	P_ATOM_UTF8_STRING,
	P_ATOM_NET_WM_NAME,
	P_ATOM_NET_WM_STATE,
	P_ATOM_NET_WM_STATE_FULLSCREEN,
	P_ATOM_NET_WM_DECORATION,
	P_ATOM_NET_WM_DECORATION_ALL,
	P_ATOM_NET_WM_WINDOW_TYPE,
	P_ATOM_NET_WM_WINDOW_TYPE_NORMAL,
	P_ATOM_NET_WM_WINDOW_TYPE_DIALOG,
	P_ATOM_NET_WM_WINDOW_TYPE_DOCK,
	P_ATOM_MOTIF_WM_HINTS,
	P_ATOM_MAX
};

/**
 * PDisplayInfo
 *
 * This struct holds all the low-level display information
 * Values here should never be set directly
 */
struct PDisplayInfo {
	// X info
	xcb_atom_t atoms[P_ATOM_MAX];
	xcb_connection_t *connection;
	xcb_screen_t *screen;
	xcb_window_t window;

	// Window graphics
	xcb_gcontext_t graphics_context;
	xcb_pixmap_t pixmap;
};

// Window
#define p_window_create(p_app_data, p_window_request) \
	p_x11_window_create(p_app_data, p_window_request)
#define p_window_close(p_window_data) \
	p_x11_window_close(p_window_data)
#define p_window_fullscreen(p_window_data) \
	p_x11_window_fullscreen(p_window_data)
#define p_window_docked_fullscreen(p_window_data) \
	p_x11_window_docked_fullscreen(p_window_data)
#define p_window_windowed(p_window_data, x, y, width, height) \
	p_x11_window_windowed(p_window_data, x, y, width, height)
#define p_window_set_dimensions(p_display_info, x, y, width, height) \
	p_x11_window_set_dimensions(p_display_info, x, y, width, height)
#define p_window_set_name(p_display_info, name) \
	p_x11_window_set_name(p_display_info, name)

PHANTOM_API void p_x11_window_create(PAppData *app_data, const PWindowRequest window_request);
PHANTOM_API void p_x11_window_close(PWindowData *window_data);
PHANTOM_API void p_x11_window_fullscreen(PWindowData *window_data);
PHANTOM_API void p_x11_window_docked_fullscreen(PWindowData *window_data);
PHANTOM_API void p_x11_window_windowed(PWindowData *window_data, uint x, uint y, uint width, uint height);
PHANTOM_API void p_x11_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height);
PHANTOM_API void p_x11_window_set_name(PDisplayInfo *display_info, const wchar_t *name);

#endif // _PHANTOM_DISPLAY_X11_H
