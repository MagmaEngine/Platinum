#ifndef _P_X11_INTERNAL
#define _P_X11_INTERNAL

#include <xcb/xcb.h>

typedef struct {
	xcb_connection_t *connection;
	xcb_screen_t *screen;
	xcb_window_t window;
} PDisplayInfo;

#endif // _P_X11_INTERNAL
