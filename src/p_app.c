#include "platinum.h"
#include <locale.h>
#include <stdio.h>

PMutex debug_memory_mutex;

/**
 * p_app_init
 *
 * Generic function that creates an application.
 * splits into platform specific functions
 */
PAppData *p_app_init(PAppRequest app_request)
{
	setlocale(LC_ALL, "");

	debug_memory_mutex = p_mutex_init();
	p_debug_memory_init(&debug_memory_mutex);

	PAppData *app_data = malloc(sizeof *app_data);
	app_data->app_config = app_request.app_config;

	// init window mutex
	app_data->window_mutex = p_mutex_init();

	// create the window array
	app_data->window_data = p_dynarr_init(sizeof (PWindowData *), 1);

	// create the input manager
	//app_data->input_manager = p_event_init();

	// create the vulkan instance
	app_data->graphical_app_data = p_graphics_init(&app_request.graphical_app_request);

#ifdef PLATINUM_PLATFORM_LINUX
	p_linux_app_init(app_data, app_request);
#elif defined PLATINUM_PLATFORM_WINDOWS
	p_windows_app_init(app_data, app_request);
#endif // PLATINUM_PLATFORM_LINUX

	return app_data;
}

/**
 * p_app_deinit
 *
 * Closes the application
 * Takes a PAppData to deconstruct.
 */
void p_app_deinit(PAppData *app_data)
{
#ifdef PLATINUM_PLATFORM_LINUX
	p_linux_app_deinit(app_data);
#elif defined PLATINUM_PLATFORM_WINDOWS
	p_windows_app_deinit(app_data);
#endif // PLATINUM_PLATFORM_LINUX

	while (p_dynarr_count(app_data->window_data) > 0)
	{
		PWindowData *window_data = p_dynarr_get(app_data->window_data, PWindowData *, 0);
		p_window_close(window_data);
		p_thread_join(window_data->event_manager);
		int index = p_dynarr_find(app_data->window_data, &window_data);
		free(window_data->event_calls);
		free(window_data->name);
		free(window_data);
		p_dynarr_remove_unordered(app_data->window_data, index);
	}
	p_dynarr_deinit(app_data->window_data);
	p_mutex_destroy(app_data->window_mutex);

	//p_event_deinit(app_data->input_manager);
	p_graphics_deinit(app_data->graphical_app_data);

	free(app_data);

	p_debug_mem_print(0);
	p_mutex_destroy(debug_memory_mutex);
}
