#ifndef _PHANTOM_H
#define _PHANTOM_H

#include "enigma.h"

#ifndef _UINT
#define _UINT
typedef unsigned int uint;
#endif // _UINT

typedef enum {
	P_DISPLAY_WINDOWED,
	P_DISPLAY_FULLSCREEN,
	P_DISPLAY_WINDOWED_FULLSCREEN,
} PWindowDisplayType;

typedef enum {
	P_INTERACT_INPUT,
	P_INTERACT_INPUT_OUTPUT,
} PWindowInteractType;

/**
 * PWindowSettings
 *
 * This struct acts as a status showing the window's current settings
 * Values here should never be set directly
 */
typedef struct {
	wchar_t *name;
	uint x;
	uint y;
	uint width;
	uint height;
	PWindowDisplayType display_type;
	PWindowInteractType interact_type;
} PWindowSettings;

/**
 * PWindowRequest
 *
 * This struct is used to create a new window with the requested settings
 * Values here should never be set directly
 */
typedef struct {
	wchar_t *name;
	uint x;
	uint y;
	uint width;
	uint height;
	PWindowDisplayType display_type;
	PWindowInteractType interact_type;
} PWindowRequest;


// X11 systems
#ifdef _PHANTOM_X11

#include <xcb/xcb.h>

/**
 * PDisplayInfo
 *
 * This struct holds all the low-level display information
 * Values here should never be set directly
 */
typedef struct {
	xcb_connection_t *connection;
	xcb_screen_t *screen;
	xcb_window_t window;
} PDisplayInfo;

#define p_window_create p_x11_window_create
#define p_window_close p_x11_window_close
#define p_window_fullscreen p_x11_window_fullscreen
#define p_window_windowed_fullscreen p_x11_window_windowed_fullscreen
#define p_window_windowed p_x11_window_windowed

void p_x11_window_create(const PWindowRequest window_request, PDisplayInfo *display_info,
		PWindowSettings *window_settings);
void p_x11_window_close(PDisplayInfo *display_info, PWindowSettings *window_settings);
void p_x11_window_fullscreen(PDisplayInfo *display_info);
void p_x11_window_windowed_fullscreen(PDisplayInfo *display_info);
void p_x11_window_windowed(PDisplayInfo *display_info, uint x, uint y, uint width, uint height);
void p_x11_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height);

#endif


// Wayland systems
#ifdef _PHANTOM_WAYLAND

#include <wayland-client.h>
typedef struct {
} PDisplayInfo;

#define p_window_create p_wayland_window_create
#define p_window_close p_wayland_window_close
#define p_window_fullscreen p_wayland_window_fullscreen

PDisplayInfo *p_wayland_window_create(char *name, PWindowDisplayType *display_type);
void p_wayland_window_close(PDisplayInfo *display_info);
void p_wayland_window_fullscreen(PDisplayInfo *display_info, PWindowSettings *window_settings);
void p_wayland_window_windowed_fullscreen(PDisplayInfo *display_info, PWindowSettings *window_settings);
void p_wayland_window_windowed(PDisplayInfo *display_info, PWindowSettings *window_settings);

#endif // _PHANTOM_WAYLAND


// Linux systems
#ifdef _PHANTOM_LINUX

/**
 * PDeviceManager
 *
 * This struct holds all the device handles and event monitors
 * TODO: use me
 */
typedef struct {
	EDynarr udevs;
	EDynarr udev_monitors;
	EDynarr libevs;
	EDynarr libev_uinputs;
} PDeviceManager;

/**
 * PAppInstance
 *
 * This struct is the holds all the information for the app for a GUI to work properly
 * TODO: use me
 */
typedef struct {
	PWindowSettings *window_settings;
	PDisplayInfo *display_info;
	PDeviceManager input_manager;
} PAppInstance;

#define p_event_init p_linux_event_init
#define p_app_init p_linux_app_init

#define p_event_deinit p_linux_event_deinit
#define p_app_deinit p_linux_app_deinit

PAppInstance *p_linux_app_init(PWindowRequest *window_request);
PDeviceManager p_linux_event_init(void);

void p_linux_app_deinit(PAppInstance *app_instance);
void p_linux_event_deinit(PDeviceManager input_manager);

#endif


// Windowindow_settings systems
#ifdef _PHANTOM_WIN32

#include <windowindow_settings.h>
typedef struct {
	HWND hwnd;
} PDisplayInfo;

#define p_window_create p_win32_window_create
#define p_window_close p_win32_window_close
#define p_window_fullscreen p_win32_window_fullscreen

PDisplayInfo *p_win32_window_create(char *name, PWindowDisplayType *display_type);
void p_win32_window_close(PDisplayInfo *display_info);
void p_win32_window_fullscreen(PDisplayInfo *display_info, PWindowSettings *window_settings);
void p_win32_window_windowed_fullscreen(PDisplayInfo *display_info, PWindowSettings *window_settings);
void p_win32_window_windowed(PDisplayInfo *display_info, PWindowSettings *window_settings);

#endif // _PHANTOM_WIN32


// Windowindow_settings systems
#ifdef _PHANTOM_WINDOWS

typedef struct {
} PGameInstance;

#endif


#endif // _PHANTOM_H

