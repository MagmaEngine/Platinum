#include "phantom.h"
#include <locale.h>

EMutex debug_memory_mutex;

/**
 * p_linux_app_init
 *
 * Creates an application with GUI and input.
 * Takes a PWindowRequest for window creation and returns a PAppInstance.
 */
PHANTOM_API PAppInstance *p_linux_app_init(void)
{
	setlocale(LC_ALL, "");

	e_mutex_init(&debug_memory_mutex);
	e_debug_memory_init(&debug_memory_mutex);

	PAppInstance *app_instance = malloc(sizeof *app_instance);

	// init window mutex
	app_instance->window_mutex = malloc(sizeof *app_instance->window_mutex);
	e_mutex_init(app_instance->window_mutex);

	// create the window array
	app_instance->window_data = e_dynarr_init(sizeof (PWindowData *), 1);

	// create the input manager
	app_instance->input_manager = p_event_init();

	// create the vulkan instance
	PVulkanAppRequest *vulkan_request_app = p_vulkan_request_app_create();
	app_instance->vulkan_data_app = p_vulkan_init(vulkan_request_app);
	p_vulkan_request_app_destroy(vulkan_request_app);

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
	while (app_instance->window_data->num_items > 0)
	{
		PWindowData *window_data = E_DYNARR_GET(app_instance->window_data, PWindowData *, 0);
		p_window_close(window_data);
		e_thread_join(window_data->event_manager);
		int index = e_dynarr_find(app_instance->window_data, &window_data);
		free(window_data->event_calls);
		free(window_data->name);
		free(window_data);
		e_dynarr_remove_unordered(app_instance->window_data, index);
	}
	e_dynarr_deinit(app_instance->window_data);
	e_mutex_destroy(app_instance->window_mutex);

	p_event_deinit(app_instance->input_manager);
	p_vulkan_deinit(app_instance->vulkan_data_app);

	free(app_instance->window_mutex);
	free(app_instance);

	e_debug_mem_print(0);
	e_mutex_destroy(&debug_memory_mutex);
}
