#include "phantom.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>


PDisplayInfo *p_win32_window_create(PWindowSettings ws)
{
	HWND hwnd;
	hwnd = CreateWindow(ws.name, ws.name,
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			ws.x, ws.y, ws.width, ws.height, 0, 0,
			NULL, 0);

	if (hwnd == NULL) {
		fprintf(stderr, "PHANTOM: ERROR: Cannot open Display!\n");
		exit(1);
	}

	uint class;
	switch (ws.interact_type)
	{
		case P_INTERACT_INPUT:
		break;

		case P_INTERACT_INPUT_OUTPUT:
		default:
		break;
	}

	PDisplayInfo *di = malloc(sizeof *di);
	di->hwnd = hwnd;

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
			//w = DisplayWidth(dpy, scr);
			//h = DisplayHeight(dpy, scr);
		break;
		case P_DISPLAY_FULLSCREEN:
			x = 0;
			y = 0;
			//hw = DisplayWidth(dpy, scr);
			//h = DisplayHeight(dpy, scr);

			p_win32_window_fullscreen(di);
		break;
	}

	return di;
}

void p_win32_window_close(PDisplayInfo *di)
{
}

void p_win32_window_fullscreen(PDisplayInfo *di)
{
}
