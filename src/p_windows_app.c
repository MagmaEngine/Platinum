#include "phantom.h"
#include <locale.h>
#include <stdio.h>

/**
 * p_windows_app_init
 *
 * Creates an application with GUI and input.
 * Takes a PWindowRequest for window creation and returns a PAppInstance.
 */
PHANTOM_API PAppInstance *p_windows_app_init(void)
{
	setlocale(LC_ALL, "");

	// Makes a console
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	PAppInstance *app_instance = malloc(sizeof *app_instance);

	app_instance->window_mutex = malloc(sizeof *app_instance->window_mutex);
	e_mutex_init(app_instance->window_mutex);

	// create the window array
	app_instance->window_settings = e_dynarr_init(sizeof (PWindowSettings *), 1);

	app_instance->input_manager = p_event_init();
	return app_instance;
}

/**
 * p_windows_app_deinit
 *
 * Closes the application
 * Takes a PAppInstance to deconstruct.
 */
PHANTOM_API void p_windows_app_deinit(PAppInstance *app_instance)
{
	while (app_instance->window_settings->num_items > 0)
	{
		PWindowSettings *window_settings = *((PWindowSettings **)app_instance->window_settings->arr);
		p_window_close(window_settings);
		int index = e_dynarr_find(app_instance->window_settings, &window_settings);
		free(window_settings->event_calls);
		free(window_settings->name);
		free(window_settings);
		e_dynarr_remove_unordered(app_instance->window_settings, index);
	}
	e_dynarr_deinit(app_instance->window_settings);

	//p_event_deinit(app_instance->input_manager);
	e_mutex_destroy(app_instance->window_mutex);
	free(app_instance->window_mutex);
	free(app_instance);

	//FreeConsole(); // Closes the log console. Also ends the application.
	//Not needed since console closes on its own anyways.
}
