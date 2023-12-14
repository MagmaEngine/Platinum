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
	app_instance->window_settings = malloc(sizeof *app_instance->window_settings);
	app_instance->display_info = malloc(sizeof *app_instance->display_info);

	p_window_create(*window_request, app_instance->display_info, app_instance->window_settings);
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
	p_window_close(app_instance->display_info, app_instance->window_settings);
	p_event_deinit(app_instance->input_manager);
	free(app_instance->display_info);
	free(app_instance->window_settings);
	free(app_instance);
}
