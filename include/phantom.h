#ifndef _PHANTOM_H
#define _PHANTOM_H

#ifndef _UINT
#define _UINT
typedef unsigned int uint;
#endif // _UINT

typedef enum {
	P_DISPLAY_NONE,
	P_DISPLAY_WINDOWED,
	P_DISPLAY_FULLSCREEN,
	P_DISPLAY_WINDOWED_FULLSCREEN,
} PWindowDisplayType;

typedef enum {
	P_INTERACT_NONE,
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
} PWindowSettings;

// X11 systems
#ifdef _PHANTOM_X11
#include "p_x11.h"
#define p_window_create p_x11_window_create
#define p_window_close p_x11_window_close

#endif // _PHANTOM_X11

extern DisplayInfo *p_x11_window_create(PWindowSettings ws);
extern void p_x11_window_close(DisplayInfo *di);

#endif // _PHANTOM_H

