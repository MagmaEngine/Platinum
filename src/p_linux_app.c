#include "phantom.h"
#include <locale.h>

/**
 * p_linux_app_init
 *
 * Creates an application with GUI and input.
 * Takes a PWindowRequest for window creation and returns a PAppInstance.
 */
PHANTOM_API PAppInstance *p_linux_app_init(void)
{
	// Set locale to same as system
	setlocale(LC_ALL, "");

	PAppInstance *app_instance = malloc(sizeof *app_instance);


	//init window mutex
	app_instance->window_mutex = malloc(sizeof *app_instance->window_mutex);
	e_mutex_init(app_instance->window_mutex);

	// create the window array
	app_instance->window_settings = e_dynarr_init(sizeof (PWindowSettings *), 1);

	// create the input manager
	app_instance->input_manager = p_event_init();

	// create the vulkan instance
	app_instance->vulkan_data = p_vulkan_init();

	return app_instance;
}

/**
 * p_linux_app_deinit
 *
 * Closes the application
 * Takes a PAppInstance to deconstruct.
 */
PHANTOM_API void p_linux_app_deinit(PAppInstance *app_instance)
{
	while (app_instance->window_settings->num_items > 0)
	{
		PWindowSettings *window_settings = *((PWindowSettings **)app_instance->window_settings->arr);
		p_window_close(window_settings);
		e_thread_join(window_settings->event_manager);
		int index = e_dynarr_find(app_instance->window_settings, &window_settings);
		free(window_settings->event_calls);
		free(window_settings->name);
		free(window_settings);
		e_dynarr_remove_unordered(app_instance->window_settings, index);
	}
	e_dynarr_deinit(app_instance->window_settings);
	e_mutex_destroy(app_instance->window_mutex);

	p_event_deinit(app_instance->input_manager);
	p_vulkan_deinit(app_instance->vulkan_data);

	free(app_instance->window_mutex);
	free(app_instance);
}
