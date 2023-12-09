#include "phantom.h"
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

Display *dpy;
Window window;

void ph_x11_window_create(uint x, uint y, uint w, uint h)
{
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "PHANTOM: ERROR: Cannot open Display!\n");
		exit(1);
	}
	Window parent = XDefaultRootWindow(dpy);

	int num_items;
	XVisualInfo *vinfo = XGetVisualInfo(dpy, VisualNoMask, NULL, &num_items);

	XSetWindowAttributes attributes = {0};
	window = XCreateWindow(dpy,
			parent,
			x, y, w, h,
			0, 0,
			InputOutput,
			vinfo->visual,
			0, NULL);
	XEvent event;
	XSelectInput(dpy, window, ExposureMask | KeyPressMask);
	XMapWindow(dpy, window);
	while(1)
	{
		XNextEvent(dpy, &event);
	}
	XFree(vinfo);
}


void ph_x11_window_close(void)
{
	XDestroyWindow(dpy, window);
	XCloseDisplay(dpy);
}
