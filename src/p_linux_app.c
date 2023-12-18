#include "phantom.h"
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

/**
 * p_linux_app_init
 *
 * Creates an application with GUI and input.
 * Takes a PWindowRequest for window creation and returns a PAppInstance.
 */
PAppInstance *p_linux_app_init(void)
{
	// Set locale to same as system
	setlocale(LC_ALL, "");

	PAppInstance *app_instance = malloc(sizeof *app_instance);


	app_instance->window_mutex = malloc(sizeof *app_instance->window_mutex);
	if (mtx_init(app_instance->window_mutex, mtx_plain) != thrd_success)
	{
		fprintf(stderr, "Error initializing window mutex...\n");
		exit(1);
	}


	// create the window array
	app_instance->window_settings = e_dynarr_init(sizeof (PWindowSettings *), 1);

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
	while (app_instance->window_settings->num_items > 0)
	{
		int result;
		PWindowSettings *window_settings = *((PWindowSettings **)app_instance->window_settings->arr);
		window_settings->deinit = true;
		p_window_close(app_instance, window_settings);
		if (thrd_join(*window_settings->event_manager, &result) != thrd_success)
		{
			fprintf(stderr, "Error joining window event manager...\n");
			exit(1);
		}
		free(window_settings->event_calls);
		free(window_settings->event_manager);
		free(window_settings->name);
		free(window_settings);
		e_dynarr_remove_unordered_ptr(app_instance->window_settings, window_settings);
	}
	e_dynarr_deinit(app_instance->window_settings);
	//free(window_settings);

	p_event_deinit(app_instance->input_manager);
	mtx_destroy(app_instance->window_mutex);
	free(app_instance->window_mutex);
	free(app_instance);
}
