#ifndef _P_X11_INTERNAL
#define _P_X11_INTERNAL

#include <X11/Xlib.h>

typedef struct {
	Display *dpy;
	Window window;
} PDisplayInfo;

#endif // _P_X11_INTERNAL
