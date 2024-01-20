#ifndef _PHANTOM_H
#define _PHANTOM_H

#ifdef PHANTOM_BACKEND_VULKAN
#include "p_graphics_vulkan.h"
#endif // PHANTOM_BACKEND_VULKAN

#ifdef PHANTOM_DISPLAY_X11
#include "p_display_x11.h"
#endif // PHANTOM_DISPLAY_X11
#ifdef PHANTOM_DISPLAY_WAYLAND
#include "p_display_wayland.h"
#endif // PHANTOM_DISPLAY_WAYLAND
#ifdef PHANTOM_DISPLAY_WIN32
#include "p_display_win32.h"
#endif // PHANTOM_DISPLAY_WIN32

#ifdef PHANTOM_PLATFORM_LINUX
#include "p_platform_linux.h"
#endif // PHANTOM_PLATFORM_LINUX
#ifdef PHANTOM_PLATFORM_WINDOWS
#include "p_platform_windows.h"
#endif // PHANTOM_PLATFORM_WINDOWS


// Graphics

struct PGraphicalAppRequest {
	bool headless;
};

struct PGraphicalDisplayRequest {
	bool headless;
	bool stereoscopic;
};

struct PGraphicalDevice {
	char *name;
	void *handle; // backend-specific handle (for vulkan its VkPhysicalDevice)
};


// Device Management

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


// Display

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

/**
 * PEventCalls
 *
 * This struct is used to set user-based functionality onto events from the window manager
 */
typedef struct {
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
} PEventCalls;

/**
 * PWindowData
 *
 * This struct acts as a status showing the window's current data and settings
 * Values here should never be set directly
 */
struct PWindowData {
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
	PGraphicalDisplayData *graphical_display_data;
};

/**
 * PWindowRequest
 *
 * This struct is used to create a new window with the requested settings
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
	PGraphicalDisplayRequest graphical_display_request;
};

/**
 * PAppRequest
 *
 * This struct is used to create a new app with the requested settings
 */
struct PAppRequest {
	PGraphicalAppRequest graphical_app_request;
};

/**
 * PAppData
 *
 * This struct is the holds all the information for the app for a GUI to work properly
 */
struct PAppData {
	EDynarr *window_data; // Array of (PWindowData *)
	PDeviceManager *input_manager;
	EMutex *window_mutex;
	PGraphicalAppData *graphical_app_data;
};

#endif // _PHANTOM_H

