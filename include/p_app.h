#ifndef _PLATINUM_APP_H
#define _PLATINUM_APP_H

#include "p_graphics.h"
#include "p_util.h"
#include "p_window.h"

// Internal Forward Declarations
typedef struct PAppConfig PAppConfig;
typedef struct PAppRequest PAppRequest;


/**
 * PAppConfig
 *
 * This struct is used to create a new app with the requested settings
 */
struct PAppConfig {
	char *shader_path;
};

/**
 * PAppRequest
 *
 * This struct is used to create a new app with the requested settings
 */
struct PAppRequest {
	PGraphicalAppRequest graphical_app_request;
	PAppConfig *app_config;
};

/**
 * PAppData
 *
 * This struct is the holds all the information for the app for a GUI to work properly
 */
struct PAppData {
	PAppConfig *app_config;
	PDynarr *window_data; // Array of (PWindowData *)
	//PDeviceManager *input_manager;
	PMutex window_mutex;
	PGraphicalAppData graphical_app_data;
};


PAppData *p_app_init(PAppRequest app_request);
void p_app_deinit(PAppData *app_data);


#ifdef PLATINUM_PLATFORM_LINUX

void p_linux_app_init(PAppData *app_data, PAppRequest app_request);
void p_linux_app_deinit(PAppData *app_data);

//PDeviceManager *p_linux_event_init(void);
//void p_linux_event_deinit(PDeviceManager *input_manager);

#elif defined PLATINUM_PLATFORM_WINDOWS

void p_windows_app_init(PAppData *app_data, PAppRequest app_request);
void p_windows_app_deinit(PAppData *app_data);

//PDeviceManager *p_windows_event_init(void);
//void p_windows_event_deinit(PDeviceManager *input_manager);

#endif // PLATINUM_PLATFORM_WINDOWS

#endif // _PLATINUM_APP_H
