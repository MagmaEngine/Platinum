#ifndef _PHANTOM_H
#define _PHANTOM_H

#include "enigma.h"

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
	P_WINDOW_ALIVE,
	P_WINDOW_CLOSE,
	P_WINDOW_INTERNAL_CLOSE,
	P_WINDOW_MAX
};

typedef struct PAppInstance PAppInstance;
typedef struct PDeviceManager PDeviceManager;
typedef struct PDisplayInfo PDisplayInfo;
typedef struct PEventCalls PEventCalls;
typedef struct PWindowRequest PWindowRequest;
typedef struct PWindowSettings PWindowSettings;
typedef void * PEventArguments;

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
	bool enable_map;
	bool enable_unmap;
	bool enable_destroy;
	void (*expose)(PEventArguments);
	void (*configure)(PEventArguments);
	void (*property)(PEventArguments);
	void (*client)(PEventArguments);
	void (*focus_in)(PEventArguments);
	void (*focus_out)(PEventArguments);
	void (*enter)(PEventArguments);
	void (*leave)(PEventArguments);
	void (*map)(PEventArguments);
	void (*unmap)(PEventArguments);
	void (*destroy)(PEventArguments);
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
 * PAppInstance
 *
 * This struct is the holds all the information for the app for a GUI to work properly
 */
struct PAppInstance {
	EDynarr *window_settings; // Array of (PWindowSettings *)
	PDeviceManager *input_manager;
	EMutex *window_mutex;
};



// X11 systems
#ifdef _PHANTOM_X11

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
	xcb_connection_t *connection;
	xcb_screen_t *screen;
	xcb_window_t window;
	xcb_atom_t atoms[P_ATOM_MAX];
};

#define p_window_create p_x11_window_create
#define p_window_close p_x11_window_close
#define p_window_fullscreen p_x11_window_fullscreen
#define p_window_docked_fullscreen p_x11_window_docked_fullscreen
#define p_window_windowed p_x11_window_windowed
#define p_window_set_dimensions p_x11_window_set_dimensions
#define p_window_set_name p_x11_window_set_name
#define p_window_event_manage p_x11_window_event_manage


void p_x11_window_create(PAppInstance *app_instance, const PWindowRequest window_request);
void p_x11_window_close(PWindowSettings *window_settings);
void p_x11_window_fullscreen(PDisplayInfo *display_info);
void p_x11_window_docked_fullscreen(PDisplayInfo *display_info);
void p_x11_window_windowed(PDisplayInfo *display_info, uint x, uint y, uint width, uint height);
void p_x11_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height);
void p_x11_window_set_name(PDisplayInfo *display_info, wchar_t *name);
EThreadResult p_x11_window_event_manage(EThreadArguments args);

#endif


// Wayland systems
#ifdef _PHANTOM_WAYLAND

#include <wayland-client.h>
struct PDisplayInfo{
};

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
struct PDeviceManager {
	EDynarr *udev_monitors;
	EDynarr *libevs;
	EDynarr *libev_uinputs;
};

#define p_event_init p_linux_event_init
#define p_app_init p_linux_app_init

#define p_event_deinit p_linux_event_deinit
#define p_app_deinit p_linux_app_deinit

PAppInstance *p_linux_app_init(void);
PDeviceManager *p_linux_event_init(void);

void p_linux_app_deinit(PAppInstance *app_instance);
void p_linux_event_deinit(PDeviceManager *input_manager);

#endif


// Windows systems
#ifdef _PHANTOM_WIN32

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
	uint screen_width;
	uint screen_height;
};

#define p_window_create p_win32_window_create
#define p_window_close p_win32_window_close
#define p_window_fullscreen p_win32_window_fullscreen
#define p_window_docked_fullscreen p_win32_window_docked_fullscreen
#define p_window_windowed p_win32_window_windowed
#define p_window_set_dimensions p_win32_window_set_dimensions
#define p_window_set_name p_win32_window_set_name
#define p_window_event_manage p_win32_window_event_manage


void p_win32_window_create(PAppInstance *app_instance, const PWindowRequest window_request);
void p_win32_window_close(PWindowSettings *window_settings);
void p_win32_window_fullscreen(PDisplayInfo *display_info);
void p_win32_window_docked_fullscreen(PDisplayInfo *display_info);
void p_win32_window_windowed(PDisplayInfo *display_info, uint x, uint y, uint width, uint height);
void p_win32_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height);
void p_win32_window_set_name(PDisplayInfo *display_info, wchar_t *name);
EThreadResult p_win32_window_event_manage(EThreadArguments args);


#endif // _PHANTOM_WIN32


// Windows systems
#ifdef _PHANTOM_WINDOWS

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

#define p_event_init p_windows_event_init
#define p_app_init p_windows_app_init

#define p_event_deinit p_windows_event_deinit
#define p_app_deinit p_windows_app_deinit

PAppInstance *p_windows_app_init(void);
PDeviceManager *p_windows_event_init(void);

void p_windows_app_deinit(PAppInstance *app_instance);
void p_windows_event_deinit(PDeviceManager *input_manager);


#endif


#endif // _PHANTOM_H

