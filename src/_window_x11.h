#ifndef PLATINUM_INTERNAL_WINDOW_X11_H
#define PLATINUM_INTERNAL_WINDOW_X11_H

#include <xcb/xcb.h>

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

#endif // PLATINUM_INTERNAL_WINDOW_X11_H
