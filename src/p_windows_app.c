#include "phantom.h"
#include <locale.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

/**
 * p_windows_app_init
 *
 * Creates an application with GUI and input.
 * Takes a PWindowRequest for window creation and returns a PAppData.
 */
PHANTOM_API PAppData *p_windows_app_init(PAppRequest app_request)
{
	setlocale(LC_ALL, "");

	// Makes a console
	AllocConsole();
	FILE* stream;

	// Reopen stdout in binary mode to support Unicode output
	if ((_wfreopen_s(&stream, L"CONOUT$", L"w", stdout) || _wfreopen_s(&stream, L"CONOUT$", L"w", stderr))!= 0)
	{
		perror("Failed to reopen stdout or stderr");
		exit(1);
	}
	_setmode(_fileno(stderr), _O_U16TEXT);
	_setmode(_fileno(stdout), _O_U16TEXT);



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
 * p_windows_app_deinit
 *
 * Closes the application
 * Takes a PAppData to deconstruct.
 */
PHANTOM_API void p_windows_app_deinit(PAppData *app_data)
{
	while (app_data->window_data->num_items > 0)
	{
		PWindowData *window_data = E_DYNARR_GET(app_data->window_data, PWindowData *, 0);
		p_window_close(window_data);
		int index = e_dynarr_find(app_data->window_data, &window_data);
		free(window_data->event_calls);
		free(window_data->name);
		free(window_data);
		e_dynarr_remove_unordered(app_data->window_data, index);
	}
	e_dynarr_deinit(app_data->window_data);
	e_mutex_destroy(app_data->window_mutex);

	p_event_deinit(app_data->input_manager);
	p_vulkan_deinit(app_data->graphical_app_data);

	free(app_data->window_mutex);
	free(app_data);

	//FreeConsole(); // Closes the log console. Also ends the application.
	//Not needed since console closes on its own anyways.
}
