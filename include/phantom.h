#ifndef _PHANTOM_H
#define _PHANTOM_H

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

typedef struct {
	char *name;
	uint x;
	uint y;
	uint width;
	uint height;
	PWindowDisplayType display_type;
	PWindowInteractType interact_type;
	uint framerate;
} PWindowSettings;

// X11 systems
#ifdef _PHANTOM_X11

#include "p_x11.h"

#define p_window_create p_x11_window_create
#define p_window_close p_x11_window_close
#define p_window_fullscreen p_x11_window_fullscreen

PDisplayInfo *p_x11_window_create(PWindowSettings ws);
void p_x11_window_close(PDisplayInfo *di);
void p_x11_window_fullscreen(PDisplayInfo *di);

#endif

// Wayland systems
#ifdef _PHANTOM_WAYLAND

#define p_window_create p_wayland_window_create
#define p_window_close p_wayland_window_close
#define p_window_fullscreen p_wayland_window_fullscreen

PDisplayInfo *p_wayland_window_create(PWindowSettings ws);
void p_wayland_window_close(PDisplayInfo *di);
void p_wayland_window_fullscreen(PDisplayInfo *di);

#endif // _PHANTOM_WAYLAND

// Windows systems
#ifdef _PHANTOM_WIN32

#include "p_win32.h"

#define p_window_create p_win32_window_create
#define p_window_close p_win32_window_close
#define p_window_fullscreen p_win32_window_fullscreen

PDisplayInfo *p_win32_window_create(PWindowSettings ws);
void p_win32_window_close(PDisplayInfo *di);
void p_win32_window_fullscreen(PDisplayInfo *di);

#endif // _PHANTOM_WIN32



#endif // _PHANTOM_H

