#include "phantom.h"
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


DisplayInfo *p_x11_window_create(PWindowSettings ws)
{
	Display *dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "PHANTOM: ERROR: Cannot open Display!\n");
		exit(1);
	}
	Window parent = XDefaultRootWindow(dpy);

	int num_items;
	XVisualInfo *vinfo = XGetVisualInfo(dpy, VisualNoMask, NULL, &num_items);

	Window window = XCreateWindow(dpy,
			parent,
			ws.x, ws.y, ws.width, ws.height,
			vinfo->depth, 0,
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

	DisplayInfo *di;
	di->dpy = dpy;
	di->window = window;
	return di;
}


void p_x11_window_close(DisplayInfo *di)
{
	XDestroyWindow(di->dpy, di->window);
	XCloseDisplay(di->dpy);
}
