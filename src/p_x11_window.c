#include "phantom.h"
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>


PDisplayInfo *p_x11_window_create(PWindowSettings ws)
{
	Display *dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "PHANTOM: ERROR: Cannot open Display!\n");
		exit(1);
	}
	Window parent = DefaultRootWindow(dpy);
	int scr = DefaultScreen(dpy);

	int num_items;
	XVisualInfo *vinfo = XGetVisualInfo(dpy, VisualNoMask, NULL, &num_items);

	uint class;
	switch (ws.interact_type)
	{
		case P_INTERACT_INPUT:
			class = InputOnly;
		break;

		case P_INTERACT_INPUT_OUTPUT:
		default:
			class = InputOutput;
		break;
	}

	uint border_width = 0;
	Window window = XCreateWindow(dpy,
			parent,
			ws.x, ws.y, ws.width, ws.height,
			border_width,
			vinfo->depth,
			class,
			vinfo->visual,
			0, NULL);

	int x, y, w, h;
	switch (ws.display_type)
	{
		case P_DISPLAY_WINDOWED:
			x = ws.x;
			y = ws.y;
			w = ws.width;
			h = ws.height;
		break;
		case P_DISPLAY_WINDOWED_FULLSCREEN:
			x = 0;
			y = 0;
			w = DisplayWidth(dpy, scr);
			h = DisplayHeight(dpy, scr);
		break;
		case P_DISPLAY_FULLSCREEN:
			x = 0;
			y = 0;
			w = DisplayWidth(dpy, scr);
			h = DisplayHeight(dpy, scr);

			p_x11_window_fullscreen(dpy, window, 1);
		break;
	}
	XMoveResizeWindow(dpy, window, x, y, w, h);

	XSetWindowBackground(dpy, window, 0x000000);
	XEvent event;
	XSelectInput(dpy, window, ExposureMask | KeyPressMask);
	XMapWindow(dpy, window);
	while(1)
	{
		XNextEvent(dpy, &event);
	}
	XFree(vinfo);

	PDisplayInfo *di;
	di->dpy = dpy;
	di->window = window;
	return di;
}

void p_x11_window_close(PDisplayInfo *di)
{
	XDestroyWindow(di->dpy, di->window);
	XCloseDisplay(di->dpy);
}

void p_x11_window_fullscreen(Display *dpy, Window win, uint i)
{
	Atom atoms[2] = { XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False), None };
	XChangeProperty(dpy, win, XInternAtom(dpy, "_NET_WM_STATE", False),
			XA_ATOM, 32, PropModeReplace, (unsigned char *)atoms, 1);
}
