#include "phantom.h"
#include <windows.h>
#include <stdio.h>

HBRUSH hBrush;
void _win32_window_close(PAppInstance *app_instance, PWindowSettings *window_settingsi);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	void *window_data = (void *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (uMsg)
	{
		case WM_DESTROY:
			DeleteObject(hBrush);
			PostQuitMessage(0);
			PAppInstance *app_instance = ((PAppInstance **)window_data)[0];
			PWindowSettings *window_settings = *(PWindowSettings **)&((char *)window_data)[sizeof (PAppInstance **)];
			e_mutex_lock(app_instance->window_mutex);
			if (window_settings->status == P_WINDOW_ALIVE)
				window_settings->status = P_WINDOW_CLOSE;
			e_mutex_unlock(app_instance->window_mutex);
			_win32_window_close(app_instance, window_settings);
			return 0;
		case WM_ERASEBKGND:
			// Handle erase background message to avoid flickering
			return 1;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			// Set the background color to black
			SetBkColor(hdc, RGB(0, 0, 0));

			// Create a black brush
			if (hBrush == NULL) {
				hBrush = CreateSolidBrush(RGB(0, 0, 0));
			}

			// Fill the background with the black brush
			FillRect(hdc, &ps.rcPaint, hBrush);
			EndPaint(hwnd, &ps);
		}
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

void p_win32_window_create(PAppInstance *app_instance, const PWindowRequest window_request)
{
	PDisplayInfo *display_info = malloc(sizeof *display_info);
	display_info->hInstance = GetModuleHandleW(NULL);
	display_info->screen_width = GetSystemMetrics(SM_CXSCREEN);
	display_info->screen_height = GetSystemMetrics(SM_CYSCREEN);

	PWindowSettings *window_settings = malloc(sizeof *window_settings);
	window_settings->name = malloc((wcslen(window_request.name)+1) * sizeof(wchar_t));
	wcscpy(window_settings->name, window_request.name);
	window_settings->x = e_mini(e_maxi(window_request.x, 0), display_info->screen_width);
	window_settings->y = e_mini(e_maxi(window_request.y, 0), display_info->screen_height);
	window_settings->width = e_mini(e_maxi(window_request.width, 1), display_info->screen_width);
	window_settings->height = e_mini(e_maxi(window_request.height, 1), display_info->screen_height);
	window_settings->display_type = window_request.display_type;
	window_settings->interact_type = window_request.interact_type;
	window_settings->display_info = display_info;
	window_settings->event_calls = calloc(1, sizeof *window_settings->event_calls);
	memcpy(window_settings->event_calls, &window_request.event_calls, sizeof *window_settings->event_calls);
	window_settings->status = P_WINDOW_ALIVE;

	// Register the window class
	WNDCLASSW windowClass = {0};
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = display_info->hInstance;
	windowClass.lpszClassName = L"MyWindowClass";
	RegisterClassW(&windowClass);

	display_info->hwnd = CreateWindowExW(
			0,
			L"MyWindowClass",
			window_settings->name,
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			window_settings->x, window_settings->y, window_settings->width, window_settings->height,
			NULL,	  // Parent window
			NULL,	  // Menu
			display_info->hInstance,
			NULL);	 // Additional application data

	if (display_info->hwnd == NULL) {
		fprintf(stderr, "PHANTOM: ERROR: Cannot open Display!\n");
		exit(1);
	}

	// Set display based on type
	switch (window_request.display_type)
	{
		case P_DISPLAY_WINDOWED:
			p_window_windowed(display_info, window_request.x, window_request.y, window_request.width,
					window_request.height);
		break;

		case P_DISPLAY_DOCKED_FULLSCREEN:
			p_window_docked_fullscreen(display_info);
		break;

		case P_DISPLAY_FULLSCREEN:
			p_window_fullscreen(display_info);
		break;

		case P_DISPLAY_MAX:
			fprintf(stderr, "P_DISPLAY_MAX is not a valid window type...\n");
			exit(1);
	}

	e_mutex_lock(app_instance->window_mutex);
	e_dynarr_add(app_instance->window_settings, &window_settings);
	e_mutex_unlock(app_instance->window_mutex);

	// Display the window
	ShowWindow(display_info->hwnd, SW_SHOWNORMAL);
	UpdateWindow(display_info->hwnd);

	// Start the window event manager
	EThreadArguments args = malloc(sizeof (PAppInstance *) + sizeof (PWindowSettings *));
	memcpy(args, &app_instance, sizeof (PAppInstance **));
	memcpy(&((char *)args)[sizeof (PAppInstance **)], &window_settings, sizeof (PWindowSettings **));
	window_settings->event_manager = e_thread_create(p_window_event_manage, args);

	SetWindowLongPtr(display_info->hwnd, GWLP_USERDATA, (LONG_PTR)args);
}

void p_win32_window_close(PWindowSettings *window_settings)
{
	window_settings->status = P_WINDOW_INTERNAL_CLOSE;
	PDisplayInfo *display_info = window_settings->display_info;
	if (display_info->hwnd != NULL)
		DestroyWindow(display_info->hwnd);
	if (window_settings->event_manager != NULL)
		CloseHandle(window_settings->event_manager);
}

void _win32_window_close(PAppInstance *app_instance, PWindowSettings *window_settings)
{
	// If window exists delete it.
	e_mutex_lock(app_instance->window_mutex);
	int index = e_dynarr_contains(app_instance->window_settings, &window_settings);
	if (index == -1)
	{
		fprintf(stderr, "Window does not exist...\n");
		exit(1);
	}
	free(window_settings->display_info);
	if (window_settings->status == P_WINDOW_CLOSE)
	{
		free(window_settings->event_calls);
		free(window_settings->name);
		free(window_settings);
		e_thread_detach(e_thread_self());
	}
	e_dynarr_remove_unordered(app_instance->window_settings, index);
	e_mutex_unlock(app_instance->window_mutex);
}

EThreadResult p_win32_window_event_manage(EThreadArguments args)
{
	PAppInstance *app_instance = ((PAppInstance **)args)[0];
	PWindowSettings *window_settings = *(PWindowSettings **)&((char *)args)[sizeof (PAppInstance **)];

	MSG msg = {0};

	// Run the message loop
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	free(args);
	p_win32_window_close(window_settings);
	return 0;
}

void p_win32_window_fullscreen(PDisplayInfo *display_info)
{
	SetWindowLong(display_info->hwnd, GWL_STYLE, GetWindowLong(display_info->hwnd, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW);
	SetWindowPos(display_info->hwnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
}

void p_win32_window_docked_fullscreen(PDisplayInfo *display_info)
{
}

void p_win32_window_windowed(PDisplayInfo *display_info, uint x, uint y, uint width, uint height)
{
}
