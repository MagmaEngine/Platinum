#include "phantom.h"
#include <windows.h>

static HANDLE window_creation_event;

void _win32_window_close(PAppInstance *app_instance, PWindowSettings *window_settings);

/**
 * WindowProc
 *
 * This internal function gets run by the event manager and responds to different messages
 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	void *window_data = (void *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	PAppInstance *app_instance;
	PWindowSettings *window_settings;

	if (window_data == NULL)
	{
		e_log_message(E_LOG_WARNING, L"Phantom", L"Window Data not associated yet.");
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	} else {
		window_settings = *(PWindowSettings **)&((char *)window_data)[sizeof (PAppInstance **)];
		app_instance = ((PAppInstance **)window_data)[0];
	}

	switch (uMsg)
	{
		// Expose events
		case WM_ERASEBKGND:
		{
			e_log_message(E_LOG_DEBUG, L"Phantom", L"Erase Background Event triggered.");
			if (window_settings->event_calls->enable_expose && window_settings->event_calls->expose != NULL)
				window_settings->event_calls->expose();
			// Handle erase background message to avoid flickering
			return 1;
		}
		case WM_PAINT:
		{
			e_log_message(E_LOG_DEBUG, L"Phantom", L"Paint Event triggered.");
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			// Set the background color to black
			SetBkColor(hdc, RGB(0, 0, 0));

			// Fill the background with the black brush
			FillRect(hdc, &ps.rcPaint, window_settings->display_info->hBrush);
			EndPaint(hwnd, &ps);
			if (window_settings->event_calls->enable_expose && window_settings->event_calls->expose != NULL)
				window_settings->event_calls->expose();
			return 0;
		}

		// Configure events
		case WM_DISPLAYCHANGE:
		{
			e_log_message(E_LOG_DEBUG, L"Phantom", L"Display Change Event triggered.");
			window_settings->display_info->screen_width = GetSystemMetrics(SM_CXSCREEN);
			window_settings->display_info->screen_height = GetSystemMetrics(SM_CYSCREEN);
			if (window_settings->event_calls->enable_configure && window_settings->event_calls->configure != NULL)
				window_settings->event_calls->configure();
			return 0;
		}
		case WM_SIZE:
		{
			e_log_message(E_LOG_DEBUG, L"Phantom", L"Size Event triggered.");
			window_settings->width = LOWORD(lParam);
			window_settings->height = HIWORD(lParam);
			if (window_settings->event_calls->enable_configure && window_settings->event_calls->configure != NULL)
				window_settings->event_calls->configure();
			InvalidateRect(hwnd, NULL, TRUE);
			return 0;
		}
		case WM_EXITSIZEMOVE:
		{
			e_log_message(E_LOG_DEBUG, L"Phantom", L"Exit Size Move Event triggered.");
			RECT windowRect;
			GetWindowRect(hwnd, &windowRect);

			window_settings->x = windowRect.left;
			window_settings->y = windowRect.top;

			InvalidateRect(hwnd, NULL, TRUE);

			if (window_settings->event_calls->enable_configure && window_settings->event_calls->configure != NULL)
				window_settings->event_calls->configure();
			return 0;
		}

		// Client message events
		case WM_COPYDATA:
		{
			e_log_message(E_LOG_DEBUG, L"Phantom", L"Copy Data Event triggered.");
			if (window_settings->event_calls->enable_client && window_settings->event_calls->client != NULL)
				window_settings->event_calls->client();
			return 0;
		}

		// Focus in events
		case WM_SETFOCUS:
		{
			e_log_message(E_LOG_DEBUG, L"Phantom", L"Set Focus Event triggered.");
			if (window_settings->event_calls->enable_focus_in && window_settings->event_calls->focus_in != NULL)
				window_settings->event_calls->focus_in();
			return 0;
		}

		// Focus out events
		case WM_KILLFOCUS:
		{
			e_log_message(E_LOG_DEBUG, L"Phantom", L"Kill Focus Event triggered.");
			if (window_settings->event_calls->enable_focus_out && window_settings->event_calls->focus_out != NULL)
				window_settings->event_calls->focus_out();
			return 0;
		}

		// Enter events
		case WM_MOUSEMOVE:
		{
			e_log_message(E_LOG_DEBUG, L"Phantom", L"Mouse Move Event triggered.");
			// TODO: make the event only trigger on mouse enter
			if (window_settings->event_calls->enable_enter && window_settings->event_calls->enter != NULL)
				window_settings->event_calls->enter();
			return 0;
		}

		// Leave events
		case WM_MOUSELEAVE:
		{
			e_log_message(E_LOG_DEBUG, L"Phantom", L"Mouse Leave Event triggered.");
			if (window_settings->event_calls->enable_leave && window_settings->event_calls->leave != NULL)
				window_settings->event_calls->leave();
			return 0;
		}

		// Destroy events
		case WM_DESTROY:
		{
			e_log_message(E_LOG_DEBUG, L"Phantom", L"Destroy Event triggered.");
			DeleteObject(window_settings->display_info->hBrush);
			if (window_settings->event_calls->enable_destroy && window_settings->event_calls->destroy != NULL)
				window_settings->event_calls->destroy();
			PostQuitMessage(0);
			e_mutex_lock(app_instance->window_mutex);
			if (window_settings->status == P_WINDOW_STATUS_ALIVE)
				window_settings->status = P_WINDOW_STATUS_CLOSE;
			e_mutex_unlock(app_instance->window_mutex);
			_win32_window_close(app_instance, window_settings);
			return 0;
		}

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

/**
 * p_win32_window_set_dimensions
 *
 * updates window_settings with the values provided as x, y, width, and heigth
 */
PHANTOM_API void p_win32_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height)
{
	SetWindowPos(display_info->hwnd, HWND_TOP, x, y, width, height, SWP_FRAMECHANGED);
	UpdateWindow(display_info->hwnd);
}

/**
 * p_win32_window_set_name
 *
 * sets the name of the window
 */
PHANTOM_API void p_win32_window_set_name(PDisplayInfo *display_info, const wchar_t *name)
{
	SetWindowText(display_info->hwnd, name);
}

/**
 * p_win32_window_create
 *
 * creates a window with parameters set from window_request.
 * returns a window_settings associated with the window.
 */
PHANTOM_API void p_win32_window_create(PAppInstance *app_instance, const PWindowRequest window_request)
{
	PDisplayInfo *display_info = malloc(sizeof *display_info);
	display_info->hInstance = GetModuleHandle(NULL);
	display_info->screen_width = GetSystemMetrics(SM_CXSCREEN);
	display_info->screen_height = GetSystemMetrics(SM_CYSCREEN);
	display_info->hBrush = CreateSolidBrush(RGB(0, 0, 0));
	display_info->class_name = L"PhantomWindowClass";

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
	window_settings->status = P_WINDOW_STATUS_ALIVE;

	// Register the window class
	WNDCLASS window_class = {0};
	window_class.lpfnWndProc = WindowProc;
	window_class.hInstance = display_info->hInstance;
	window_class.lpszClassName = display_info->class_name;
	if (!RegisterClass(&window_class))
	{
		e_log_message(E_LOG_ERROR, L"Phantom", L"Window registration failed...");
		exit(1);
	}

	window_creation_event = CreateEvent(NULL, FALSE, FALSE, NULL);

	// Start the window event manager
	EThreadArguments args = malloc(sizeof (PAppInstance *) + sizeof (PWindowSettings *));
	memcpy(args, &app_instance, sizeof (PAppInstance **));
	memcpy(&((char *)args)[sizeof (PAppInstance **)], &window_settings, sizeof (PWindowSettings **));
	window_settings->event_manager = e_thread_create(p_window_event_manage, args);

	DWORD result = WaitForSingleObject(window_creation_event, INFINITE);
	if (result != WAIT_OBJECT_0)
	{
		e_log_message(E_LOG_ERROR, L"Phantom", L"Error waiting for window creation.");
		exit(1);
	}
	CloseHandle(window_creation_event);
	InvalidateRect(display_info->hwnd, NULL, TRUE);
}

/**
 * p_win32_window_close
 *
 * sends a signal to close the window and the event manager
 */
PHANTOM_API void p_win32_window_close(PWindowSettings *window_settings)
{
	window_settings->status = P_WINDOW_STATUS_INTERNAL_CLOSE;
	PDisplayInfo *display_info = window_settings->display_info;
	if (display_info->hwnd != NULL)
		DestroyWindow(display_info->hwnd);
	if (window_settings->event_manager != NULL)
		CloseHandle(window_settings->event_manager);
}

/**
 * _win32_window_close
 *
 * internal close function that actually closes the window and frees data
 */
void _win32_window_close(PAppInstance *app_instance, PWindowSettings *window_settings)
{
	// If window exists delete it.
	e_mutex_lock(app_instance->window_mutex);
	int index = e_dynarr_contains(app_instance->window_settings, &window_settings);
	if (index == -1)
	{
		e_log_message(E_LOG_ERROR, L"Phantom", L"Window does not exist...");
		exit(1);
	}
	free(window_settings->display_info);
	if (window_settings->status == P_WINDOW_STATUS_CLOSE)
	{
		free(window_settings->event_calls);
		free(window_settings->name);
		free(window_settings);
		e_thread_detach(e_thread_self());
	}
	e_dynarr_remove_unordered(app_instance->window_settings, index);
	e_mutex_unlock(app_instance->window_mutex);
}

/**
 * p_win32_window_event_manage
 *
 * Windows Specific: actually creates the window because windows is stupid.
 * This function runs in its own thread and manages window manager events
 * returns 0
 */
EThreadResult WINAPI p_win32_window_event_manage(EThreadArguments args)
{
	PAppInstance *app_instance = ((PAppInstance **)args)[0];
	PWindowSettings *window_settings = *(PWindowSettings **)&((char *)args)[sizeof (PAppInstance **)];
	window_settings->display_info->hwnd = CreateWindowEx(
			0,
			window_settings->display_info->class_name,
			window_settings->name,
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			window_settings->x, window_settings->y, window_settings->width, window_settings->height,
			NULL,	  // Parent window
			NULL,	  // Menu
			window_settings->display_info->hInstance,
			NULL);	 // Additional application data

	if (window_settings->display_info->hwnd == NULL) {
		e_log_message(E_LOG_ERROR, L"Phantom", L"Cannot open window");
		exit(1);
	}

	// Set display based on type
	switch (window_settings->display_type)
	{
		case P_DISPLAY_WINDOWED:
			p_window_windowed(window_settings, window_settings->x, window_settings->y, window_settings->width,
					window_settings->height);
		break;

		case P_DISPLAY_DOCKED_FULLSCREEN:
			p_window_docked_fullscreen(window_settings);
		break;

		case P_DISPLAY_FULLSCREEN:
			p_window_fullscreen(window_settings);
		break;

		case P_DISPLAY_MAX:
			e_log_message(E_LOG_WARNING, L"Phantom", L"P_DISPLAY_MAX is not a valid window type");
			exit(1);
	}

	e_mutex_lock(app_instance->window_mutex);
	e_dynarr_add(app_instance->window_settings, &window_settings);
	e_mutex_unlock(app_instance->window_mutex);

	SetWindowLongPtr(window_settings->display_info->hwnd, GWLP_USERDATA, (LONG_PTR)args);
	SetEvent(window_creation_event);

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

/**
 * p_win32_window_fullscreen
 *
 * sets the window in window_settings to fullscreen
 */
PHANTOM_API void p_win32_window_fullscreen(PWindowSettings *window_settings)
{
	DEVMODE devMode = {0};
	devMode.dmSize = sizeof(DEVMODE);
	devMode.dmPelsWidth = GetSystemMetrics(SM_CXSCREEN);
	devMode.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN);
	devMode.dmBitsPerPel = 32; // Adjust as needed
	devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
	ShowWindow(window_settings->display_info->hwnd, SW_HIDE);
	if (ChangeDisplaySettings(&devMode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
	{
		SetWindowLong(window_settings->display_info->hwnd, GWL_STYLE, WS_POPUP);
		SetWindowPos(window_settings->display_info->hwnd, HWND_TOP, 0, 0, devMode.dmPelsWidth, devMode.dmPelsHeight, SWP_FRAMECHANGED);
		window_settings->display_type = P_DISPLAY_FULLSCREEN;
	}
	ShowWindow(window_settings->display_info->hwnd, SW_SHOWNORMAL);
	UpdateWindow(window_settings->display_info->hwnd);
}

/**
 * p_win32_window_docked_fullscreen
 *
 * sets the window in window_settings to (borderless/windowed) fullscreen
 */
PHANTOM_API void p_win32_window_docked_fullscreen(PWindowSettings *window_settings)
{
	if (window_settings->display_type == P_DISPLAY_FULLSCREEN)
	{
		ShowWindow(window_settings->display_info->hwnd, SW_HIDE);
		if (ChangeDisplaySettings(NULL, 0) != DISP_CHANGE_SUCCESSFUL)
			return;
	}
	SetWindowLong(window_settings->display_info->hwnd, GWL_STYLE,
			GetWindowLong(window_settings->display_info->hwnd, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW);
	SetWindowPos(window_settings->display_info->hwnd, HWND_TOP, 0, 0, window_settings->display_info->screen_width,
			window_settings->display_info->screen_height, SWP_FRAMECHANGED);
	window_settings->display_type = P_DISPLAY_DOCKED_FULLSCREEN;

	ShowWindow(window_settings->display_info->hwnd, SW_SHOWNORMAL);
	UpdateWindow(window_settings->display_info->hwnd);
}

/**
 * p_win32_window_windowed
 *
 * sets the window in window_settings to windowed mode and sets the dimensions
 */
PHANTOM_API void p_win32_window_windowed(PWindowSettings *window_settings, uint x, uint y, uint width, uint height)
{
	if (window_settings->display_type == P_DISPLAY_FULLSCREEN)
	{
		ShowWindow(window_settings->display_info->hwnd, SW_HIDE);
		if (ChangeDisplaySettings(NULL, 0) != DISP_CHANGE_SUCCESSFUL)
			return;
	}
	SetWindowLong(window_settings->display_info->hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
	SetWindowPos(window_settings->display_info->hwnd, HWND_TOP, x, y, width, height, SWP_FRAMECHANGED);
	window_settings->display_type = P_DISPLAY_WINDOWED;
	ShowWindow(window_settings->display_info->hwnd, SW_SHOWNORMAL);
	UpdateWindow(window_settings->display_info->hwnd);
}
