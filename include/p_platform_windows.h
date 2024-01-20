#ifndef _PHANTOM_PLATFORM_WINDOWS_H
#define _PHANTOM_PLATFORM_WINDOWS_H

#include "enigma.h"
#include "p_api.h"

// App
#define p_app_init(p_app_request) p_windows_app_init(p_app_request)
#define p_app_deinit(p_app_data) p_windows_app_deinit(p_app_data)

PHANTOM_API PAppData *p_windows_app_init(PAppRequest app_request);
PHANTOM_API void p_windows_app_deinit(PAppData *app_data);

// Event
#define p_event_init() p_windows_event_init()
#define p_event_deinit(input_manager) p_windows_event_deinit(input_manager)

PDeviceManager *p_windows_event_init(void);
void p_windows_event_deinit(PDeviceManager *input_manager);

#endif // _PHANTOM_PLATFORM_WINDOWS_H
