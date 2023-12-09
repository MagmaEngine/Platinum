#ifndef _PHANTOM_H
#define _PHANTOM_H

#include "enigma.h"

extern void ph_x11_window_create(uint x, uint y, uint w, uint h);
extern void ph_x11_window_close(void);

#ifdef _PHANTOM_X11

#define ph_window_create ph_x11_window_create
#define ph_window_close ph_x11_window_close

#endif // _PHANTOM_X11

#endif // _PHANTOM_H

