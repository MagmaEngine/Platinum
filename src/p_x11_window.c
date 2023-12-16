#include "phantom.h"
#include "enigma.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <xcb/xcb.h>

/**
 * p_x11_window_set_dimensions
 *
 * updates window_settings with the values provided as x, y, width, and heigth
 * and submits a request to the WM to resize the window
 */
void p_x11_window_set_dimensions(PDisplayInfo *display_info, uint x, uint y, uint width, uint height)
{
	// resize
	uint32_t params[4] = {x, y, width, height};
	xcb_configure_window(display_info->connection, display_info->window,
			XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
			params);
}

/**
 * p_x11_generate_atom
 *
 * generates an xcb atom and returns it.
 * Returns XCB_ATOM_NONE if atom cannot be found.
 */
xcb_atom_t p_x11_generate_atom(xcb_connection_t *connection, const char *atom_name)
{
	xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, 0, strlen(atom_name), atom_name);
	xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, NULL);
	if (!reply) {
		fprintf(stderr, "Could not get atom reply...");
		exit(1);
	}
	xcb_atom_t atom = reply->atom;
	free(reply);
	return atom;
}


/**
 * p_x11_window_create
 *
 * creates a window with parameters set from window_request.
 * returns a window_settings associated with the window.
 */
void p_x11_window_create(PAppInstance *app_instance, const PWindowRequest window_request)
{
	// Create an XCB connection and window
	xcb_connection_t *connection = xcb_connect(NULL, NULL);

	// Check if the connection was successful
	if (xcb_connection_has_error(connection))
	{
		fprintf(stderr, "Unable to open the connection to the X server\n");
		exit(1);
	}

	xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
	xcb_window_t window = xcb_generate_id(connection);

	PDisplayInfo *display_info = malloc(sizeof *display_info);
	display_info->connection = connection;
	display_info->screen = screen;
	display_info->window = window;

	uint class = P_INTERACT_INPUT_OUTPUT;
	uint border_width = 0;

	PWindowSettings *window_settings = malloc(sizeof *window_settings);
	window_settings->name = malloc((wcslen(window_request.name)+1) * sizeof(wchar_t));
	wcscpy(window_settings->name, window_request.name);
	window_settings->x = window_request.x;
	window_settings->y = window_request.y;
	window_settings->width = window_request.width;
	window_settings->height = window_request.height;
	window_settings->display_type = window_request.display_type;
	window_settings->interact_type = window_request.interact_type;
	window_settings->display_info = display_info;
	window_settings->event_calls = calloc(1, sizeof *window_settings->event_calls);
	memcpy(window_settings->event_calls, &window_request.event_calls, sizeof *window_settings->event_calls);

	// creates the window
	xcb_create_window(
			connection,
			XCB_COPY_FROM_PARENT,
			window,
			screen->root,
			window_request.x, window_request.y, window_request.width, window_request.height,
			border_width,
			class,
			screen->root_visual,
			XCB_CW_BACK_PIXEL, &screen->black_pixel);

	// Listen different events in the window
	// PROPERTY_CHANGE:		When window properties change (windowed, fullscreen, windowed fullscreen)
	// RESIZE_REDIRECT:		Requested resizing of window from WM
	// EXPOSURE:			When a previously obscured part of window becomes visible
	// STRUCTURE_NOTIFY:	When the window is resized, mapped, or moved
	// SUBSTRUCTURE_NOTIFY:	Like STRUCTURE, but for subwindows
	// FOCUS_CHANGE:		When the window gains or loses input focus
	uint32_t event_mask =
		XCB_EVENT_MASK_PROPERTY_CHANGE |
		//XCB_EVENT_MASK_RESIZE_REDIRECT |
		XCB_EVENT_MASK_LEAVE_WINDOW |
		XCB_EVENT_MASK_ENTER_WINDOW |
		XCB_EVENT_MASK_EXPOSURE |
		XCB_EVENT_MASK_STRUCTURE_NOTIFY |
		XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
		XCB_EVENT_MASK_FOCUS_CHANGE;
	xcb_change_window_attributes(connection, window, XCB_CW_EVENT_MASK, &event_mask);

	// Set display based on type
	switch (window_request.display_type)
	{
		case P_DISPLAY_WINDOWED:
			p_x11_window_windowed(display_info, window_request.x, window_request.y, window_request.width,
					window_request.height);
		break;

		case P_DISPLAY_WINDOWED_FULLSCREEN:
			p_x11_window_windowed_fullscreen(display_info);
		break;

		case P_DISPLAY_FULLSCREEN:
			p_x11_window_fullscreen(display_info);
		break;
	}

	e_dynarr_add(app_instance->window_settings, window_settings);

	// TODO: make wrapper for pthreads
	// Start the window event manager
	pthread_t thread_xcb_event_manager;
	pthread_create(&thread_xcb_event_manager, NULL, p_x11_window_event_manage, (void *)window_settings);
	//pthread_detach(thread_xcb_event_manager);
	pthread_join(thread_xcb_event_manager, NULL);
}


/**
 * p_x11_window_close
 *
 * closes the window and the connection to the Xserver
 */
void p_x11_window_close(PAppInstance *app_instance, PWindowSettings *window_settings)
{
	// Find the matching window and close it
	int index = -1;
	for(uint i = 0; i < app_instance->window_settings->num_items; i++)
	{
		if (&((PWindowSettings *)app_instance->window_settings->arr)[i] == window_settings)
		{
			index = i;
			break;
		}
	}
	// If window matches delete it.
	if (index > -1)
	{
		PDisplayInfo *display_info = window_settings->display_info;
		xcb_unmap_window(display_info->connection, display_info->window);
		xcb_disconnect(display_info->connection);
		free(window_settings->display_info);
		free(window_settings->event_calls);
		free(window_settings->name);
		e_dynarr_remove_unordered(app_instance->window_settings, index);
	}
}


/**
 * p_x11_window_event_manage
 *
 * This function runs in its own thread and manages window manager events
 * Right now args is useless
 */
void *p_x11_window_event_manage(void *args)
{
	// unpack args
	PWindowSettings *window_settings = (PWindowSettings *)args;
	PDisplayInfo *display_info = window_settings->display_info;
	PEventCalls *event_calls = window_settings->event_calls;

	bool window_alive = true;

	while (window_alive) {
		xcb_generic_event_t *event = xcb_wait_for_event(display_info->connection);

		if (event)
		{
			switch (event->response_type & ~0x80) {
				// TODO: experiment with capturing mouse and keyboard
				case XCB_EXPOSE:
				{
					xcb_expose_event_t *expose_event = (xcb_expose_event_t *)event;
					// TODO: redraw (only new part?)
					if (event_calls->enable_expose && event_calls->expose != NULL)
						event_calls->expose(expose_event);
					break;
				}
				case XCB_CONFIGURE_NOTIFY:
				{
					xcb_configure_notify_event_t *config_notify_event = (xcb_configure_notify_event_t *)event;
					// TODO: Update window_settings
					if (event_calls->enable_configure && event_calls->configure != NULL)
						event_calls->configure(config_notify_event);
					break;
				}
				case XCB_PROPERTY_NOTIFY:
				{
					xcb_property_notify_event_t *property_notify_event = (xcb_property_notify_event_t *)event;

					xcb_atom_t atom_state = p_x11_generate_atom(display_info->connection, "_NET_WM_STATE");
					xcb_atom_t atom_fullscreen = p_x11_generate_atom(display_info->connection, "_NET_WM_STATE_FULLSCREEN");

					//enum PWindowDisplayType display_type;
					if (property_notify_event->atom == atom_state)
					{
						// Handle _NET_WM_STATE property change
						printf("_NET_WM_STATE property changed\n");
					}
					xcb_get_property_cookie_t cookie =
						xcb_get_property(display_info->connection, 0, display_info->window, atom_state, XCB_ATOM_ANY, 0,
								1024);
					xcb_get_property_reply_t *reply =
						xcb_get_property_reply(display_info->connection, cookie, NULL);

					if (reply)
					{
						xcb_atom_t *state = (xcb_atom_t *)xcb_get_property_value(reply);
						int num_atoms = xcb_get_property_value_length(reply) / sizeof(xcb_atom_t);
						// Check if the _NET_WM_STATE_FULLSCREEN atom is present
						for (int i = 0; i < num_atoms; i++) {
							if (state[i] == atom_fullscreen) {
								// Fullscreen is set
								//display_type = P_DISPLAY_FULLSCREEN;
								break;
							}
						}
						free(reply);
					}

					// Set Fullscreen status
					//window_settings->display_type = display_type;

					if (event_calls->enable_property && event_calls->property != NULL)
						event_calls->property(property_notify_event);
					break;
				}
				case XCB_CLIENT_MESSAGE:
				{
					xcb_client_message_event_t *message_event = (xcb_client_message_event_t *)event;
					// TODO:
					if (event_calls->enable_client && event_calls->client != NULL)
						event_calls->client(message_event);
					break;
				}
				case XCB_FOCUS_IN:
				{
					xcb_focus_in_event_t *focus_in_event = (xcb_focus_in_event_t *)event;
					// TODO:
					if (event_calls->enable_focus_in && event_calls->focus_in != NULL)
						event_calls->focus_in(focus_in_event);
					break;
				}
				case XCB_FOCUS_OUT:
				{
					xcb_focus_out_event_t *focus_out_event = (xcb_focus_out_event_t *)event;
					// TODO:
					if (event_calls->enable_focus_out && event_calls->focus_out != NULL)
						event_calls->focus_out(focus_out_event);
					break;
				}
				case XCB_ENTER_NOTIFY:
				{
					xcb_enter_notify_event_t *enter_notify_event = (xcb_enter_notify_event_t *)event;
					// TODO:
					if (event_calls->enable_enter && event_calls->enter != NULL)
						event_calls->enter(enter_notify_event);
					break;
				}
				case XCB_LEAVE_NOTIFY:
				{
					xcb_leave_notify_event_t *leave_notify_event = (xcb_leave_notify_event_t *)event;
					// TODO:
					if (event_calls->enable_leave && event_calls->leave != NULL)
						event_calls->leave(leave_notify_event);
					break;
				}
				case XCB_MAP_NOTIFY:
				{
					xcb_map_notify_event_t *map_notify_event = (xcb_map_notify_event_t *)event;
					// TODO: mimimize/maximize? tell engine to go active?
					if (event_calls->enable_map && event_calls->map != NULL)
						event_calls->map(map_notify_event);
					break;
				}
				case XCB_UNMAP_NOTIFY:
				{
					xcb_unmap_notify_event_t *unmap_notify_event = (xcb_unmap_notify_event_t *)event;
					// TODO: mimimize/maximize? tell engine to go dormant?
					if (event_calls->enable_unmap && event_calls->unmap != NULL)
						event_calls->unmap(unmap_notify_event);
					break;
				}
				case XCB_DESTROY_NOTIFY:
				{
					xcb_destroy_notify_event_t *destroy_notify_event = (xcb_destroy_notify_event_t *)event;
					window_alive = false;
					// TODO: handle errors
					if (event_calls->enable_destroy && event_calls->destroy != NULL)
						event_calls->destroy(destroy_notify_event);
					break;
				}
			}
		} else {
			window_alive = false;
		}
		free(event);
	}
	free(window_settings);
	return NULL;
}


/**
 * p_x11_window_fullscreen
 *
 * sets the window in display_info to fullscreen
 * TODO: listen for request granted and set PWindowSettings accordingly
 */
void p_x11_window_fullscreen(PDisplayInfo *display_info)
{

	xcb_atom_t atom_type = p_x11_generate_atom(display_info->connection, "_NET_WM_WINDOW_TYPE");
	xcb_atom_t atom_normal = p_x11_generate_atom(display_info->connection, "_NET_WM_WINDOW_TYPE_NORMAL");

	xcb_atom_t atom_state = p_x11_generate_atom(display_info->connection, "_NET_WM_STATE");
	xcb_atom_t atom_fullscreen = p_x11_generate_atom(display_info->connection, "_NET_WM_STATE_FULLSCREEN");

	if (atom_state == XCB_ATOM_NONE || atom_fullscreen == XCB_ATOM_NONE || atom_type == XCB_ATOM_NONE ||
			atom_normal == XCB_ATOM_NONE)
	{
		fprintf(stderr, "Unable to make fullscreen\n");
		exit(1);
	}

	// set normal fullscreen window
	xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window, atom_type,
			XCB_ATOM_ATOM, 32, 1, &atom_normal);
	xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window, atom_state,
			XCB_ATOM_ATOM, 32, 1, &atom_fullscreen);

	xcb_unmap_window(display_info->connection, display_info->window);
	xcb_map_window(display_info->connection, display_info->window);
	xcb_flush(display_info->connection);
}

/**
 * p_x11_window_windowed_fullscreen
 *
 * sets the window in display_info to (borderless/windowed) fullscreen
 * TODO: listen for request granted and set PWindowSettings accordingly
 */
void p_x11_window_windowed_fullscreen(PDisplayInfo *display_info)
{
	xcb_atom_t atom_type = p_x11_generate_atom(display_info->connection, "_NET_WM_WINDOW_TYPE");
	xcb_atom_t atom_normal = p_x11_generate_atom(display_info->connection, "_NET_WM_WINDOW_TYPE_NORMAL");
	xcb_atom_t atom_decor = p_x11_generate_atom(display_info->connection, "_NET_WM_DECORATION");
	xcb_atom_t atom_decor_all = p_x11_generate_atom(display_info->connection, "_NET_WM_DECORATION_ALL");
	xcb_atom_t atom_state = p_x11_generate_atom(display_info->connection, "_NET_WM_STATE");

	if (atom_decor == XCB_ATOM_NONE || atom_decor_all == XCB_ATOM_NONE || atom_type == XCB_ATOM_NONE ||
			atom_normal == XCB_ATOM_NONE || atom_state == XCB_ATOM_NONE)
	{
		fprintf(stderr, "Unable to make windowed fullscreen\n");
		exit(1);
	}

	// set windowed fullscreen window
	xcb_delete_property(display_info->connection, display_info->window, atom_state);
	xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window, atom_type,
			XCB_ATOM_ATOM, 32, 1, &atom_normal);
	xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window, atom_decor,
			XCB_ATOM_ATOM, 32, 1, &atom_decor_all);
	p_x11_window_set_dimensions(display_info, 0, 0, display_info->screen->width_in_pixels,
			display_info->screen->height_in_pixels);

	xcb_unmap_window(display_info->connection, display_info->window);
	xcb_map_window(display_info->connection, display_info->window);
	xcb_flush(display_info->connection);
}

/**
 * p_x11_window_windowed
 *
 * sets the window in display_info to windowed mode and sets the dimensions
 * TODO: listen for request granted and set PWindowSettings accordingly
 */
void p_x11_window_windowed(PDisplayInfo *display_info, uint x, uint y, uint width, uint height)
{
	xcb_atom_t atom_type = p_x11_generate_atom(display_info->connection, "_NET_WM_WINDOW_TYPE");
	xcb_atom_t atom_normal = p_x11_generate_atom(display_info->connection, "_NET_WM_WINDOW_TYPE_NORMAL");
	xcb_atom_t atom_state = p_x11_generate_atom(display_info->connection, "_NET_WM_STATE");
	xcb_atom_t atom_decor = p_x11_generate_atom(display_info->connection, "_NET_WM_DECORATION");

	// set normal window
	if (atom_type == XCB_ATOM_NONE || atom_normal == XCB_ATOM_NONE || atom_state == XCB_ATOM_NONE ||
			atom_decor == XCB_ATOM_NONE)
	{
		fprintf(stderr, "Unable to make windowed\n");
		exit(1);
	}
	xcb_delete_property(display_info->connection, display_info->window, atom_state);
	xcb_delete_property(display_info->connection, display_info->window, atom_decor);
	xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window, atom_type,
			XCB_ATOM_ATOM, 32, 1, &atom_normal);

	p_x11_window_set_dimensions(display_info, x, y, width, height);

	xcb_unmap_window(display_info->connection, display_info->window);
	xcb_map_window(display_info->connection, display_info->window);
	xcb_flush(display_info->connection);
}
