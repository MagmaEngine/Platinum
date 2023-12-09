#ifndef _P_X11_INTERNAL
#define _P_X11_INTERNAL

#include <X11/Xlib.h>

typedef struct {
	Display *dpy;
	Window window;
} DisplayInfo;

#endif // _P_X11_INTERNAL
