#include "platinum.h"
#include <windows.h>

#include "_graphics.h"

// Forward function declarations for internal functions
void _window_close(PAppData *app_data, PWindowData *window_data);

// Internal Enums

// Internal Structs

/**
 * PDisplayInfo
 *
 * This struct holds all the low-level display information
 * Values here should never be set directly
 */
struct PDisplayInfo{
	HWND hwnd;
	HINSTANCE hInstance;
	HBRUSH hBrush;
	wchar_t *class_name;
	uint screen_width;
	uint screen_height;
};


// Internal Variables

static HANDLE window_creation_event;

// Forward function declarations for internal functions
static void _win32_window_close(PAppInstance *app_instance, PWindowData *window_data);
static PThreadResult WINAPI _win32_window_event_manage(PThreadArguments args);

/**
 * WindowProc
 *
 * This internal function gets run by the event manager and responds to different messages
 */
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	void *window_ptr_data = (void *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	PAppInstance *app_instance;
	PWindowData *window_data;

	if (window_ptr_data == NULL)
	{
		p_log_message(P_LOG_WARNING, L"Phantom", L"Window Data not associated yet.");
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	} else {
		window_data = *(PWindowData **)&((char *)window_ptr_data)[sizeof (PAppInstance **)];
		app_instance = ((PAppInstance **)window_ptr_data)[0];
	}

	switch (uMsg)
	{
		// Expose events
		case WM_ERASEBKGND:
		{
			p_log_message(P_LOG_DEBUG, L"Phantom", L"Erase Background Event triggered.");
			if (window_data->event_calls->enable_expose && window_data->event_calls->expose != NULL)
				window_data->event_calls->expose();
			// Handle erase background message to avoid flickering
			return 1;
		}
		case WM_PAINT:
		{
			p_log_message(P_LOG_DEBUG, L"Phantom", L"Paint Event triggered.");
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			// Set the background color to black
			SetBkColor(hdc, RGB(0, 0, 0));

			// Fill the background with the black brush
			FillRect(hdc, &ps.rcPaint, window_data->display_info->hBrush);
			EndPaint(hwnd, &ps);
			if (window_data->event_calls->enable_expose && window_data->event_calls->expose != NULL)
				window_data->event_calls->expose();
			return 0;
		}

		// Configure events
		case WM_DISPLAYCHANGE:
		{
			p_log_message(P_LOG_DEBUG, L"Phantom", L"Display Change Event triggered.");
			window_data->display_info->screen_width = GetSystemMetrics(SM_CXSCREEN);
			window_data->display_info->screen_height = GetSystemMetrics(SM_CYSCREEN);
			if (window_data->event_calls->enable_configure && window_data->event_calls->configure != NULL)
				window_data->event_calls->configure();
			return 0;
		}
		case WM_SIZE:
		{
			p_log_message(P_LOG_DEBUG, L"Phantom", L"Size Event triggered.");
			window_data->width = LOWORD(lParam);
			window_data->height = HIWORD(lParam);
			if (window_data->event_calls->enable_configure && window_data->event_calls->configure != NULL)
				window_data->event_calls->configure();
			InvalidateRect(hwnd, NULL, TRUE);
			return 0;
		}
		case WM_EXITSIZEMOVE:
		{
			p_log_message(P_LOG_DEBUG, L"Phantom", L"Exit Size Move Event triggered.");
			RECT windowRect;
			GetWindowRect(hwnd, &windowRect);

			window_data->x = windowRect.left;
			window_data->y = windowRect.top;

			InvalidateRect(hwnd, NULL, TRUE);

			if (window_data->event_calls->enable_configure && window_data->event_calls->configure != NULL)
				window_data->event_calls->configure();
			return 0;
		}

		// Client message events
		case WM_COPYDATA:
		{
			p_log_message(P_LOG_DEBUG, L"Phantom", L"Copy Data Event triggered.");
			if (window_data->event_calls->enable_client && window_data->event_calls->client != NULL)
				window_data->event_calls->client();
			return 0;
		}

		// Focus in events
		case WM_SETFOCUS:
		{
			p_log_message(P_LOG_DEBUG, L"Phantom", L"Set Focus Event triggered.");
			if (window_data->event_calls->enable_focus_in && window_data->event_calls->focus_in != NULL)
				window_data->event_calls->focus_in();
			return 0;
		}

		// Focus out events
		case WM_KILLFOCUS:
		{
			p_log_message(P_LOG_DEBUG, L"Phantom", L"Kill Focus Event triggered.");
			if (window_data->event_calls->enable_focus_out && window_data->event_calls->focus_out != NULL)
				window_data->event_calls->focus_out();
			return 0;
		}

		// Enter events
		case WM_MOUSEMOVE:
		{
			p_log_message(P_LOG_DEBUG, L"Phantom", L"Mouse Move Event triggered.");
			// TODO: make the event only trigger on mouse enter
			if (window_data->event_calls->enable_enter && window_data->event_calls->enter != NULL)
				window_data->event_calls->enter();
			return 0;
		}

		// Leave events
		case WM_MOUSELEAVE:
		{
			p_log_message(P_LOG_DEBUG, L"Phantom", L"Mouse Leave Event triggered.");
			if (window_data->event_calls->enable_leave && window_data->event_calls->leave != NULL)
				window_data->event_calls->leave();
			return 0;
		}

		// Destroy events
		case WM_DESTROY:
		{
			p_log_message(P_LOG_DEBUG, L"Phantom", L"Destroy Event triggered.");
			DeleteObject(window_data->display_info->hBrush);
			if (window_data->event_calls->enable_destroy && window_data->event_calls->destroy != NULL)
				window_data->event_calls->destroy();
			PostQuitMessage(0);
			p_mutex_lock(app_instance->window_mutex);
			if (window_data->status == P_WINDOW_STATUS_ALIVE)
				window_data->status = P_WINDOW_STATUS_CLOSE;
			p_mutex_unlock(app_instance->window_mutex);
			_window_close(app_instance, window_data);
			return 0;
		}

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

/**
 * p_win32_window_set_dimensions
 *
 * updates window_data with the values provided as x, y, width, and heigth
 */
PLATINUM_API void p_win32_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height)
{
	SetWindowPos(display_info->hwnd, HWND_TOP, x, y, width, height, SWP_FRAMECHANGED);
	UpdateWindow(display_info->hwnd);
}

/**
 * p_win32_window_set_name
 *
 * sets the name of the window
 */
PLATINUM_API void p_win32_window_set_name(PDisplayInfo *display_info, const wchar_t *name)
{
	SetWindowText(display_info->hwnd, name);
}

/**
 * p_win32_window_create
 *
 * creates a window with parameters set from window_request.
 * returns a window_data associated with the window.
 */
PLATINUM_API void p_win32_window_create(PAppInstance *app_instance, const PWindowRequest window_request)
{
	PDisplayInfo *display_info = malloc(sizeof *display_info);
	display_info->hInstance = GetModuleHandle(NULL);
	display_info->screen_width = GetSystemMetrics(SM_CXSCREEN);
	display_info->screen_height = GetSystemMetrics(SM_CYSCREEN);
	display_info->hBrush = CreateSolidBrush(RGB(0, 0, 0));
	display_info->class_name = L"PhantomWindowClass";

	PWindowData *window_data = malloc(sizeof *window_data);
	size_t name_size = (wcslen(window_request.name)) * sizeof(wchar_t);
	window_data->name = malloc(name_size + sizeof(wchar_t));
	wcscpy_s(window_data->name, name_size, window_request.name);
	window_data->x = p_mini(p_maxi(window_request.x, 0), display_info->screen_width);
	window_data->y = p_mini(p_maxi(window_request.y, 0), display_info->screen_height);
	window_data->width = p_mini(p_maxi(window_request.width, 1), display_info->screen_width);
	window_data->height = p_mini(p_maxi(window_request.height, 1), display_info->screen_height);
	window_data->display_type = window_request.display_type;
	window_data->interact_type = window_request.interact_type;
	window_data->display_info = display_info;
	window_data->event_calls = calloc(1, sizeof *window_data->event_calls);
	memcpy(window_data->event_calls, &window_request.event_calls, sizeof *window_data->event_calls);
	window_data->status = P_WINDOW_STATUS_ALIVE;

	// Register the window class
	WNDCLASS window_class = {0};
	window_class.lpfnWndProc = WindowProc;
	window_class.hInstance = display_info->hInstance;
	window_class.lpszClassName = display_info->class_name;
	if (!RegisterClass(&window_class))
	{
		p_log_message(P_LOG_ERROR, L"Phantom", L"Window registration failed...");
		exit(1);
	}

	window_creation_event = CreateEvent(NULL, FALSE, FALSE, NULL);

	// Start the window event manager
	PThreadArguments args = malloc(sizeof (PAppInstance *) + sizeof (PWindowData *));
	memcpy(args, &app_instance, sizeof (PAppInstance **));
	memcpy(&((char *)args)[sizeof (PAppInstance **)], &window_data, sizeof (PWindowData **));
	window_data->event_manager = p_thread_create(_win32_window_event_manage, args);

	DWORD result = WaitForSingleObject(window_creation_event, INFINITE);
	if (result != WAIT_OBJECT_0)
	{
		p_log_message(P_LOG_ERROR, L"Phantom", L"Error waiting for window creation.");
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
PLATINUM_API void p_win32_window_close(PWindowData *window_data)
{
	window_data->status = P_WINDOW_STATUS_INTERNAL_CLOSE;
	PDisplayInfo *display_info = window_data->display_info;
	if (display_info->hwnd != NULL)
		DestroyWindow(display_info->hwnd);
	if (window_data->event_manager != NULL)
		CloseHandle(window_data->event_manager);
}

/**
 * _win32_window_close
 *
 * internal close function that actually closes the window and frees data
 */
void _win32_window_close(PAppInstance *app_instance, PWindowData *window_data)
{
	// If window exists delete it.
	p_mutex_lock(app_instance->window_mutex);
	int index = e_dynarr_find(app_instance->window_data, &window_data);
	if (index == -1)
	{
		p_log_message(P_LOG_ERROR, L"Phantom", L"Window does not exist...");
		exit(1);
	}
	p_graphics_display_destroy(window_data->graphical_display_data);
	free(window_data->display_info);
	if (window_data->status == P_WINDOW_STATUS_CLOSE)
	{
		free(window_data->event_calls);
		free(window_data->name);
		free(window_data);
		p_thread_detach(p_thread_self());
	}
	e_dynarr_remove_unordered(app_instance->window_data, index);
	p_mutex_unlock(app_instance->window_mutex);
}

/**
 * _win32_window_event_manage
 *
 * Windows Specific: actually creates the window because windows is stupid.
 * This function runs in its own thread and manages window manager events
 * returns 0
 */
PThreadResult WINAPI _win32_window_event_manage(PThreadArguments args)
{
	PAppInstance *app_instance = ((PAppInstance **)args)[0];
	PWindowData *window_data = *(PWindowData **)&((char *)args)[sizeof (PAppInstance **)];
	window_data->display_info->hwnd = CreateWindowEx(
			0,
			window_data->display_info->class_name,
			window_data->name,
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			window_data->x, window_data->y, window_data->width, window_data->height,
			NULL,	  // Parent window
			NULL,	  // Menu
			window_data->display_info->hInstance,
			NULL);	 // Additional application data

	if (window_data->display_info->hwnd == NULL) {
		p_log_message(P_LOG_ERROR, L"Phantom", L"Cannot open window");
		exit(1);
	}

	// Set display based on type
	switch (window_data->display_type)
	{
		case P_DISPLAY_WINDOWED:
			p_window_windowed(window_data, window_data->x, window_data->y, window_data->width,
					window_data->height);
		break;

		case P_DISPLAY_DOCKED_FULLSCREEN:
			p_window_docked_fullscreen(window_data);
		break;

		case P_DISPLAY_FULLSCREEN:
			p_window_fullscreen(window_data);
		break;

		case P_DISPLAY_MAX:
			p_log_message(P_LOG_WARNING, L"Phantom", L"P_DISPLAY_MAX is not a valid window type");
			exit(1);
	}

	p_graphics_display_create(window_data, app_data->graphical_app_data, &window_request.graphical_display_request);

	p_mutex_lock(app_instance->window_mutex);
	e_dynarr_add(app_instance->window_data, &window_data);
	p_mutex_unlock(app_instance->window_mutex);

	SetWindowLongPtr(window_data->display_info->hwnd, GWLP_USERDATA, (LONG_PTR)args);
	SetEvent(window_creation_event);

	MSG msg = {0};

	// Run the message loop
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	free(args);
	p_window_close(window_data);
	return 0;
}

/**
 * p_win32_window_fullscreen
 *
 * sets the window in window_data to fullscreen
 */
PLATINUM_API void p_win32_window_fullscreen(PWindowData *window_data)
{
	DEVMODE devMode = {0};
	devMode.dmSize = sizeof(DEVMODE);
	devMode.dmPelsWidth = GetSystemMetrics(SM_CXSCREEN);
	devMode.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN);
	devMode.dmBitsPerPel = 32; // Adjust as needed
	devMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
	ShowWindow(window_data->display_info->hwnd, SW_HIDE);
	if (ChangeDisplaySettings(&devMode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
	{
		SetWindowLong(window_data->display_info->hwnd, GWL_STYLE, WS_POPUP);
		SetWindowPos(window_data->display_info->hwnd, HWND_TOP, 0, 0, devMode.dmPelsWidth, devMode.dmPelsHeight, SWP_FRAMECHANGED);
		window_data->display_type = P_DISPLAY_FULLSCREEN;
	}
	ShowWindow(window_data->display_info->hwnd, SW_SHOWNORMAL);
	UpdateWindow(window_data->display_info->hwnd);
}

/**
 * p_win32_window_docked_fullscreen
 *
 * sets the window in window_data to (borderless/windowed) fullscreen
 */
PLATINUM_API void p_win32_window_docked_fullscreen(PWindowData *window_data)
{
	if (window_data->display_type == P_DISPLAY_FULLSCREEN)
	{
		ShowWindow(window_data->display_info->hwnd, SW_HIDE);
		if (ChangeDisplaySettings(NULL, 0) != DISP_CHANGE_SUCCESSFUL)
			return;
	}
	SetWindowLong(window_data->display_info->hwnd, GWL_STYLE,
			GetWindowLong(window_data->display_info->hwnd, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW);
	SetWindowPos(window_data->display_info->hwnd, HWND_TOP, 0, 0, window_data->display_info->screen_width,
			window_data->display_info->screen_height, SWP_FRAMECHANGED);
	window_data->display_type = P_DISPLAY_DOCKED_FULLSCREEN;

	ShowWindow(window_data->display_info->hwnd, SW_SHOWNORMAL);
	UpdateWindow(window_data->display_info->hwnd);
}

/**
 * p_win32_window_windowed
 *
 * sets the window in window_data to windowed mode and sets the dimensions
 */
PLATINUM_API void p_win32_window_windowed(PWindowData *window_data, uint x, uint y, uint width, uint height)
{
	if (window_data->display_type == P_DISPLAY_FULLSCREEN)
	{
		ShowWindow(window_data->display_info->hwnd, SW_HIDE);
		if (ChangeDisplaySettings(NULL, 0) != DISP_CHANGE_SUCCESSFUL)
			return;
	}
	SetWindowLong(window_data->display_info->hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
	SetWindowPos(window_data->display_info->hwnd, HWND_TOP, x, y, width, height, SWP_FRAMECHANGED);
	window_data->display_type = P_DISPLAY_WINDOWED;
	ShowWindow(window_data->display_info->hwnd, SW_SHOWNORMAL);
	UpdateWindow(window_data->display_info->hwnd);
}

/**
 * p_x11_window_set_graphical_display
 *
 * adds a graphical surface to the window display
 */
void p_win32_window_set_graphical_display(PWindowData *window_data, PGraphicalAppData graphical_app_data,
		PGraphicalDisplayData graphical_display_data)
{
	VkWin32SurfaceCreateInfoKHR vk_surface_create_info;
	vk_surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vk_surface_create_info.hwnd = window_data->display_info->hwnd;
	vk_surface_create_info.hinstance = window_data->display_info->hInstance;
	if (vkCreateWin32SurfaceKHR(vulkan_app_data->instance, &vk_surface_create_info, NULL, vulkan_display_data->surface)
			!= VK_SUCCESS)
	{
		p_log_message(P_LOG_ERROR, L"Vulkan General", L"Failed to create Win32 surface!");
		exit(1);
	}
}
