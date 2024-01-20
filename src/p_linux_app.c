#include "phantom.h"
#include <locale.h>

EMutex debug_memory_mutex;

/**
 * p_linux_app_init
 *
 * Creates an application with GUI and input.
 * Takes a PWindowRequest for window creation and returns a PAppData.
 */
PHANTOM_API PAppData *p_linux_app_init(PAppRequest app_request)
{
	setlocale(LC_ALL, "");

	e_mutex_init(&debug_memory_mutex);
	e_debug_memory_init(&debug_memory_mutex);

	PAppData *app_data = malloc(sizeof *app_data);

	// init window mutex
	app_data->window_mutex = malloc(sizeof *app_data->window_mutex);
	e_mutex_init(app_data->window_mutex);

	// create the window array
	app_data->window_data = e_dynarr_init(sizeof (PWindowData *), 1);

	// create the input manager
	app_data->input_manager = p_event_init();

	// create the vulkan instance
	app_data->graphical_app_data = p_graphics_init(&app_request.graphical_app_request);

	return app_data;
}

/**
 * p_linux_app_deinit
 *
 * Closes the application
 * Takes a PAppData to deconstruct.
 */
PHANTOM_API void p_linux_app_deinit(PAppData *app_data)
{
	while (app_data->window_data->num_items > 0)
	{
		PWindowData *window_data = E_DYNARR_GET(app_data->window_data, PWindowData *, 0);
		p_window_close(window_data);
		e_thread_join(window_data->event_manager);
		int index = e_dynarr_find(app_data->window_data, &window_data);
		free(window_data->event_calls);
		free(window_data->name);
		free(window_data);
		e_dynarr_remove_unordered(app_data->window_data, index);
	}
	e_dynarr_deinit(app_data->window_data);
	e_mutex_destroy(app_data->window_mutex);

	p_event_deinit(app_data->input_manager);
	p_graphics_deinit(app_data->graphical_app_data);

	free(app_data->window_mutex);
	free(app_data);

	e_debug_mem_print(0);
	e_mutex_destroy(&debug_memory_mutex);
}
