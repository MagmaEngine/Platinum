#include "phantom.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>


//PDisplayInfo *p_win32_window_create(PWindowSettings ws)
void p_win32_window_create(PAppInstance *app_instance, const PWindowRequest window_request)
{
	PDisplayInfo *display_info = malloc(sizeof *display_info);
	
	PWindowSettings *window_settings = malloc(sizeof *window_settings);
	window_settings->name = malloc((wcslen(window_request.name)+1) * sizeof(wchar_t));
	wcscpy_s(window_settings->name, window_request.name);
	window_settings->x = e_mini(e_maxi(window_request.x, 0), screen->width_in_pixels);
	window_settings->y = e_mini(e_maxi(window_request.y, 0), screen->height_in_pixels);
	window_settings->width = e_mini(e_maxi(window_request.width, 1), screen->width_in_pixels);
	window_settings->height = e_mini(e_maxi(window_request.height, 1), screen->width_in_pixels);
	window_settings->display_type = window_request.display_type;
	window_settings->interact_type = window_request.interact_type;
	window_settings->display_info = display_info;
	window_settings->event_calls = calloc(1, sizeof *window_settings->event_calls);
	memcpy(window_settings->event_calls, &window_request.event_calls, sizeof *window_settings->event_calls);
	window_settings->deinit = false;
	
	
	window_settings->display_info->hwnd = CreateWindow(window_settings->name, window_settings->name,
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			window_settings->x, window_settings->y, window_settings->width, window_settings->height, 0, 0,
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
