#ifndef _PHANTOM_H
#define _PHANTOM_H

#ifdef PHANTOM_PLATFORM_WINDOWS
#ifdef _PHANTOM_INTERNAL
#define PHANTOM_API __declspec(dllexport)
#else
#define PHANTOM_API __declspec(dllimport)
#endif // _PHANTOM_INTERNAL
#elif defined PHANTOM_PLATFORM_LINUX
#ifdef _PHANTOM_INTERNAL
#define PHANTOM_API __attribute__((visibility("default")))
#else
#define PHANTOM_API
#endif
#endif // PHANTOM_PLATFORM_XXXXXX

#include "enigma.h"
#include <vulkan/vulkan.h>

#ifndef _UINT
#define _UINT
typedef unsigned int uint;
#endif // _UINT

enum PWindowDisplayType {
	P_DISPLAY_WINDOWED,
	P_DISPLAY_FULLSCREEN,
	P_DISPLAY_DOCKED_FULLSCREEN,
	P_DISPLAY_MAX
};

enum PWindowInteractType {
	P_INTERACT_INPUT,
	P_INTERACT_INPUT_OUTPUT,
	P_INTERACT_MAX
};

enum PWindowStatus {
	P_WINDOW_STATUS_ALIVE,
	P_WINDOW_STATUS_CLOSE,
	P_WINDOW_STATUS_INTERNAL_CLOSE,
	P_WINDOW_STATUS_MAX
};

typedef struct PAppInstance PAppInstance;
typedef struct PDeviceManager PDeviceManager;
typedef struct PDisplayInfo PDisplayInfo;
typedef struct PEventCalls PEventCalls;
typedef struct PWindowRequest PWindowRequest;
typedef struct PWindowSettings PWindowSettings;
typedef struct PVulkanData PVulkanData;

/**
 * PEventCalls
 *
 * This struct is used to set user-based functionality onto events from the window manager
 */
struct PEventCalls {
	bool enable_expose;
	bool enable_configure;
	bool enable_property;
	bool enable_client;
	bool enable_focus_in;
	bool enable_focus_out;
	bool enable_enter;
	bool enable_leave;
	bool enable_destroy;
	void (*expose)(void);
	void (*configure)(void);
	void (*property)(void);
	void (*client)(void);
	void (*focus_in)(void);
	void (*focus_out)(void);
	void (*enter)(void);
	void (*leave)(void);
	void (*destroy)(void);
};

/**
 * PWindowSettings
 *
 * This struct acts as a status showing the window's current settings
 * Values here should never be set directly
 */
struct PWindowSettings {
	wchar_t *name;
	uint x;
	uint y;
	uint width;
	uint height;
	enum PWindowDisplayType display_type;
	enum PWindowInteractType interact_type;
	enum PWindowStatus status;
	EThread event_manager;
	PEventCalls *event_calls;
	PDisplayInfo *display_info;
};

/**
 * PWindowRequest
 *
 * This struct is used to create a new window with the requested settings
 * Values here should never be set directly
 */
struct PWindowRequest {
	wchar_t *name;
	uint x;
	uint y;
	uint width;
	uint height;
	enum PWindowDisplayType display_type;
	enum PWindowInteractType interact_type;
	PEventCalls event_calls;
};

/**
 * PVulkanData
 *
 * This struct contains data relevant to vulkan
 */
struct PVulkanData {
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
};

/**
 * PAppInstance
 *
 * This struct is the holds all the information for the app for a GUI to work properly
 */
struct PAppInstance {
	EDynarr *window_settings; // Array of (PWindowSettings *)
	PDeviceManager *input_manager;
	EMutex *window_mutex;
	PVulkanData * vulkan_data;
};


// Vulkan
PVulkanData *p_vulkan_init(void);
void p_vulkan_deinit(PVulkanData *vulkan_data);
void p_vulkan_list_available_extensions(void);


// X11 systems
#ifdef PHANTOM_DISPLAY_X11

#include <xcb/xcb.h>

// MUTEX for window-based operations

/**
 * PAtomTypes
 *
 * This enum is used to initialize the display_info atoms
 * for quick lookup later
 */
enum PAtomTypes {
	P_ATOM_UTF8_STRING,
	P_ATOM_NET_WM_NAME,
	P_ATOM_NET_WM_STATE,
	P_ATOM_NET_WM_STATE_FULLSCREEN,
	P_ATOM_NET_WM_DECORATION,
	P_ATOM_NET_WM_DECORATION_ALL,
	P_ATOM_NET_WM_WINDOW_TYPE,
	P_ATOM_NET_WM_WINDOW_TYPE_NORMAL,
	P_ATOM_NET_WM_WINDOW_TYPE_DIALOG,
	P_ATOM_NET_WM_WINDOW_TYPE_DOCK,
	P_ATOM_MOTIF_WM_HINTS,
	P_ATOM_MAX
};

/**
 * PDisplayInfo
 *
 * This struct holds all the low-level display information
 * Values here should never be set directly
 */
struct PDisplayInfo {
	// X info
	xcb_atom_t atoms[P_ATOM_MAX];
	xcb_connection_t *connection;
	xcb_screen_t *screen;
	xcb_window_t window;

	// Window graphics
	xcb_gcontext_t graphics_context;
	xcb_pixmap_t pixmap;
};

// Window
#define p_window_create p_x11_window_create
#define p_window_close p_x11_window_close
#define p_window_fullscreen p_x11_window_fullscreen
#define p_window_docked_fullscreen p_x11_window_docked_fullscreen
#define p_window_windowed p_x11_window_windowed
#define p_window_set_dimensions p_x11_window_set_dimensions
#define p_window_set_name p_x11_window_set_name

PHANTOM_API void p_x11_window_create(PAppInstance *app_instance, const PWindowRequest window_request);
PHANTOM_API void p_x11_window_close(PWindowSettings *window_settings);
PHANTOM_API void p_x11_window_fullscreen(PWindowSettings *window_settings);
PHANTOM_API void p_x11_window_docked_fullscreen(PWindowSettings *window_settings);
PHANTOM_API void p_x11_window_windowed(PWindowSettings *window_settings, uint x, uint y, uint width, uint height);
PHANTOM_API void p_x11_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height);
PHANTOM_API void p_x11_window_set_name(PDisplayInfo *display_info, const wchar_t *name);

#endif


// Wayland systems
#ifdef PHANTOM_DISPLAY_WAYLAND

#include <wayland-client.h>
struct PDisplayInfo{
};

// Window
#define p_window_create p_wayland_window_create
#define p_window_close p_wayland_window_close
#define p_window_fullscreen p_wayland_window_fullscreen
#define p_window_docked_fullscreen p_wayland_window_docked_fullscreen
#define p_window_windowed p_wayland_window_windowed
#define p_window_set_dimensions p_wayland_window_set_dimensions
#define p_window_set_name p_wayland_window_set_name

PHANTOM_API void p_wayland_window_create(PAppInstance *app_instance, const PWindowRequest window_request);
PHANTOM_API void p_wayland_window_close(PWindowSettings *window_settings);
PHANTOM_API void p_wayland_window_fullscreen(PWindowSettings *window_settings);
PHANTOM_API void p_wayland_window_docked_fullscreen(PWindowSettings *window_settings);
PHANTOM_API void p_wayland_window_windowed(PWindowSettings *window_settings, uint x, uint y, uint width, uint height);
PHANTOM_API void p_wayland_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height);
PHANTOM_API void p_wayland_window_set_name(PDisplayInfo *display_info, const wchar_t *name);

// Vulkan
#define p_vulkan_init p_wayland_vulkan_init
#define p_vulkan_deinit p_wayland_vulkan_deinit

VkInstance *p_wayland_vulkan_init(void);
void p_wayland_vulkan_deinit(VkInstance *vk_instance);

#endif // PHANTOM_DISPLAY_WAYLAND


// Linux systems
#ifdef PHANTOM_PLATFORM_LINUX

/**
 * PDeviceManager
 *
 * This struct holds all the device handles and event monitors
 * TODO: use me
 */
struct PDeviceManager {
	EDynarr *udev_monitors;
	EDynarr *libevs;
	EDynarr *libev_uinputs;
};

// App
#define p_app_init p_linux_app_init
#define p_app_deinit p_linux_app_deinit

PHANTOM_API PAppInstance *p_linux_app_init(void);
PHANTOM_API void p_linux_app_deinit(PAppInstance *app_instance);

// Event
#define p_event_init p_linux_event_init
#define p_event_deinit p_linux_event_deinit

PDeviceManager *p_linux_event_init(void);
void p_linux_event_deinit(PDeviceManager *input_manager);

#endif


// Windows systems
#ifdef PHANTOM_DISPLAY_WIN32

#include <windows.h>

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

// Window
#define p_window_create p_win32_window_create
#define p_window_close p_win32_window_close
#define p_window_fullscreen p_win32_window_fullscreen
#define p_window_docked_fullscreen p_win32_window_docked_fullscreen
#define p_window_windowed p_win32_window_windowed
#define p_window_set_dimensions p_win32_window_set_dimensions
#define p_window_set_name p_win32_window_set_name

PHANTOM_API void p_win32_window_create(PAppInstance *app_instance, const PWindowRequest window_request);
PHANTOM_API void p_win32_window_close(PWindowSettings *window_settings);
PHANTOM_API void p_win32_window_fullscreen(PWindowSettings *window_settings);
PHANTOM_API void p_win32_window_docked_fullscreen(PWindowSettings *window_settings);
PHANTOM_API void p_win32_window_windowed(PWindowSettings *window_settings, uint x, uint y, uint width, uint height);
PHANTOM_API void p_win32_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height);
PHANTOM_API void p_win32_window_set_name(PDisplayInfo *display_info, const wchar_t *name);

// Vulkan
#define p_vulkan_init p_win32_vulkan_init
#define p_vulkan_deinit p_win32_vulkan_deinit

VkInstance *p_win32_vulkan_init(void);
void p_win32_vulkan_deinit(VkInstance *vk_instance);

#endif // PHANTOM_DISPLAY_WIN32


// Windows systems
#ifdef PHANTOM_PLATFORM_WINDOWS

/**
 * PDeviceManager
 *
 * This struct holds all the device handles and event monitors
 * TODO: use me
 */
struct PDeviceManager {
	EDynarr *udev_monitors;
	EDynarr *libevs;
	EDynarr *libev_uinputs;
};

// App
#define p_app_init p_windows_app_init
#define p_app_deinit p_windows_app_deinit

PHANTOM_API PAppInstance *p_windows_app_init(void);
PHANTOM_API void p_windows_app_deinit(PAppInstance *app_instance);

// Event
#define p_event_init p_windows_event_init
#define p_event_deinit p_windows_event_deinit

PDeviceManager *p_windows_event_init(void);
void p_windows_event_deinit(PDeviceManager *input_manager);

#endif


#endif // _PHANTOM_H

