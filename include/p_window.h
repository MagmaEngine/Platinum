#ifndef _PLATINUM_WINDOW_H
#define _PLATINUM_WINDOW_H

#include "enigma.h"
#include "p_graphics.h"
#include "p_util.h"
#include <wchar.h>

// External Forward Declarations
typedef struct PAppData PAppData;
typedef struct PDisplayInfo PDisplayInfo;

// Internal Forward Declarations
typedef struct PWindowRequest PWindowRequest;
typedef struct PEventCalls PEventCalls;

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
 * PWindowData
 *
 * This struct acts as a status showing the window's current data and settings
 * Values here should never be set directly
 */
struct PWindowData{
	wchar_t *name;
	uint x;
	uint y;
	uint width;
	uint height;
	enum PWindowDisplayType display_type;
	enum PWindowInteractType interact_type;
	enum PWindowStatus status;
	PThread event_manager;
	PEventCalls *event_calls;
	PDisplayInfo *display_info;
	PGraphicalDisplayData *graphical_display_data;
};

/**
 * PWindowRequest
 *
 * This struct is used to create a new window with the requested settings
 */
struct PWindowRequest{
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

void p_window_create(PAppData *app_data, const PWindowRequest window_request);
void p_window_close(PWindowData *window_data);
void p_window_fullscreen(PWindowData *window_data);
void p_window_docked_fullscreen(PWindowData *window_data);
void p_window_windowed(PWindowData *window_data, uint x, uint y, uint width, uint height);
void p_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height);
void p_window_set_name(PDisplayInfo *display_info, const wchar_t *name);

#ifdef PLATINUM_DISPLAY_WIN32

void p_win32_window_create(PAppData *app_data, const PWindowRequest window_request);
void p_win32_window_close(PWindowData *window_data);
void p_win32_window_fullscreen(PWindowData *window_data);
void p_win32_window_docked_fullscreen(PWindowData *window_data);
void p_win32_window_windowed(PWindowData *window_data, uint x, uint y, uint width, uint height);
void p_win32_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height);
void p_win32_window_set_name(PDisplayInfo *display_info, const wchar_t *name);

#endif // PLATINUM_DISPLAY_WIN32
#ifdef PLATINUM_DISPLAY_X11

void p_x11_window_create(PAppData *app_data, const PWindowRequest window_request);
void p_x11_window_close(PWindowData *window_data);
void p_x11_window_fullscreen(PWindowData *window_data);
void p_x11_window_docked_fullscreen(PWindowData *window_data);
void p_x11_window_windowed(PWindowData *window_data, uint x, uint y, uint width, uint height);
void p_x11_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height);
void p_x11_window_set_name(PDisplayInfo *display_info, const wchar_t *name);

#endif // PLATINUM_DISPLAY_X11
#ifdef PLATINUM_DISPLAY_WAYLAND

void p_wayland_window_create(PAppData *app_data, const PWindowRequest window_request);
void p_wayland_window_close(PWindowData *window_data);
void p_wayland_window_fullscreen(PWindowData *window_data);
void p_wayland_window_docked_fullscreen(PWindowData *window_data);
void p_wayland_window_windowed(PWindowData *window_data, uint x, uint y, uint width, uint height);
void p_wayland_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height);
void p_wayland_window_set_name(PDisplayInfo *display_info, const wchar_t *name);

#endif // PLATINUM_DISPLAY_WAYLAND

#endif // _PLATINUM_WINDOW_H
