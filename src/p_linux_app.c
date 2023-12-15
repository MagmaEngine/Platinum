#include "phantom.h"
#include <stdlib.h>

/**
 * p_linux_app_init
 *
 * Creates an application with GUI and input.
 * Takes a PWindowRequest for window creation and returns a PAppInstance.
 */
PAppInstance *p_linux_app_init(PWindowRequest *window_request)
{
	PAppInstance *app_instance = malloc(sizeof *app_instance);

	// create a window
	app_instance->window_settings = e_dynarr_init(sizeof (PWindowSettings), 1);

	p_window_create(app_instance, *window_request);

	// create the input manager
	app_instance->input_manager = p_event_init();
	return app_instance;
}

/**
 * p_linux_app_deinit
 *
 * Closes the application
 * Takes a PAppInstance to deconstruct.
 */
void p_linux_app_deinit(PAppInstance *app_instance)
{
	// close all windows
	while(app_instance->window_settings->num_items > 0)
	{
		p_window_close(app_instance, (PWindowSettings *)app_instance->window_settings->arr);
	}
	e_dynarr_deinit(app_instance->window_settings);
	p_event_deinit(app_instance->input_manager);
	free(app_instance);
}
