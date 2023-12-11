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

	PDisplayInfo *di = malloc(sizeof *di);
	di->hwnd = hwnd;

	return di;
}

void p_win32_window_close(PDisplayInfo *di)
{
}

void p_win32_window_fullscreen(PDisplayInfo *di)
{
}
