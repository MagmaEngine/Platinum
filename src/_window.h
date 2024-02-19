#ifndef PLATINUM_INTERNAL_WINDOW_H
#define PLATINUM_INTERNAL_WINDOW_H

#ifdef PLATINUM_DISPLAY_X11
#include "_window_x11.h"
#elif defined PLATINUM_DISPLAY_WAYLAND
#include "_window_wayland.h"
#elif defined PLATINUM_DISPLAY_WIN32
#include "_window_win32.h"
#endif // PLATINUM_DISPLAY

#endif // PLATINUM_INTERNAL_WINDOW_H
