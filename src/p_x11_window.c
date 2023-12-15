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
 * TODO: change PWindowSettings only as an event
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

	uint window_alive = 1;

	while (window_alive) {
		xcb_generic_event_t *event = xcb_wait_for_event(display_info->connection);

		if (event)
		{
			switch (event->response_type & ~0x80) {
				case XCB_EXPOSE:
				{
					xcb_expose_event_t *expose_event = (xcb_expose_event_t *)event;
					// TODO: redraw (only new part?)
					printf("EVENT: XCB_EXPOSE\n");
					break;
				}
				case XCB_CONFIGURE_NOTIFY:
				{
					xcb_configure_notify_event_t *config_event = (xcb_configure_notify_event_t *)event;
					// TODO: Update window_settings
					printf("EVENT: XCB_CONFIGURE_NOTIFY\n");
					break;
				}
				case XCB_PROPERTY_NOTIFY:
				{
					xcb_property_notify_event_t *property_event = (xcb_property_notify_event_t *)event;
					// TODO: Update window_settings
					printf("EVENT: XCB_PROPERTY_NOTIFY\n");
					break;
				}
				case XCB_CLIENT_MESSAGE:
				{
					xcb_client_message_event_t *message_event = (xcb_client_message_event_t *)event;
					// TODO:
					printf("EVENT: XCB_CLIENT_MESSAGE\n");
					break;
				}
				case XCB_FOCUS_IN:
				{
					xcb_focus_in_event_t *focus_in_event = (xcb_focus_in_event_t *)event;
					printf("EVENT: XCB_FOCUS_IN\n");
					// TODO:
					break;
				}
				case XCB_FOCUS_OUT:
				{
					xcb_focus_out_event_t *focus_out_event = (xcb_focus_out_event_t *)event;
					printf("EVENT: XCB_FOCUS_OUT\n");
					// TODO:
					break;
				}
				case XCB_ENTER_NOTIFY:
				{
					xcb_enter_notify_event_t *enter_notify_event = (xcb_enter_notify_event_t *)event;
					// TODO:
					printf("EVENT: XCB_ENTER_NOTIFY\n");
					break;
				}
				case XCB_LEAVE_NOTIFY:
				{
					xcb_leave_notify_event_t *leave_notify_event = (xcb_leave_notify_event_t *)event;
					// TODO:
					printf("EVENT: XCB_LEAVE_NOTIFY\n");
					break;
				}
				case XCB_MAP_NOTIFY:
				{
					xcb_map_notify_event_t *map_notify_event = (xcb_map_notify_event_t *)event;
					// TODO: mimimize/maximize? tell engine to go active?
					printf("EVENT: XCB_MAP_NOTIFY\n");
					break;
				}
				case XCB_UNMAP_NOTIFY:
				{
					xcb_unmap_notify_event_t *unmap_notify_event = (xcb_unmap_notify_event_t *)event;
					// TODO: mimimize/maximize? tell engine to go dormant?
					printf("EVENT: XCB_UNMAP_NOTIFY\n");
					break;
				}
				case XCB_DESTROY_NOTIFY:
				{
					xcb_destroy_notify_event_t *destroy_event = (xcb_destroy_notify_event_t *)event;
					window_alive = 0;
					// TODO: handle errors
					printf("EVENT: XCB_DESTROY_NOTIFY\n");
					break;
				}
				// ... other cases for different event types ...
			}
		} else {
			window_alive = 0;
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
	xcb_intern_atom_cookie_t cookie_type = xcb_intern_atom(display_info->connection, 0, 19, "_NET_WM_WINDOW_TYPE");
	xcb_intern_atom_reply_t *reply_type = xcb_intern_atom_reply(display_info->connection, cookie_type, NULL);
	xcb_intern_atom_cookie_t cookie_type_normal = xcb_intern_atom(display_info->connection, 0, 26,
			"_NET_WM_WINDOW_TYPE_NORMAL");
	xcb_intern_atom_reply_t *reply_type_normal = xcb_intern_atom_reply(display_info->connection,
			cookie_type_normal, NULL);

	xcb_intern_atom_cookie_t cookie_state = xcb_intern_atom(display_info->connection, 0, 13, "_NET_WM_STATE");
	xcb_intern_atom_reply_t *reply_state = xcb_intern_atom_reply(display_info->connection, cookie_state, NULL);
	xcb_intern_atom_cookie_t cookie_state_fullscreen = xcb_intern_atom(display_info->connection, 0, 24,
			"_NET_WM_STATE_FULLSCREEN");
	xcb_intern_atom_reply_t *reply_state_fullscreen = xcb_intern_atom_reply(display_info->connection,
			cookie_state_fullscreen, NULL);

	// set normal fullscreen window
	if (reply_type && reply_type_normal && reply_state && reply_state_fullscreen) {
		xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window, reply_type->atom,
				XCB_ATOM_ATOM, 32, 1, &(reply_type_normal->atom));
		free(reply_type);
		free(reply_type_normal);
		xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window, reply_state->atom,
				XCB_ATOM_ATOM, 32, 1, &(reply_state_fullscreen->atom));
		free(reply_state);
		free(reply_state_fullscreen);
	} else {
		fprintf(stderr, "Unable to make fullscreen\n");
		exit(1);
	}
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
	xcb_intern_atom_cookie_t cookie_type = xcb_intern_atom(display_info->connection, 0, 19, "_NET_WM_WINDOW_TYPE");
	xcb_intern_atom_reply_t *reply_type = xcb_intern_atom_reply(display_info->connection, cookie_type, NULL);
	xcb_intern_atom_cookie_t cookie_type_normal = xcb_intern_atom(display_info->connection, 0, 26,
			"_NET_WM_WINDOW_TYPE_NORMAL");
	xcb_intern_atom_reply_t *reply_type_normal = xcb_intern_atom_reply(display_info->connection, cookie_type_normal,
			NULL);

	xcb_intern_atom_cookie_t cookie_decor = xcb_intern_atom(display_info->connection, 0, 17, "_NET_WM_DECORATION");
	xcb_intern_atom_reply_t *reply_decor = xcb_intern_atom_reply(display_info->connection, cookie_decor, NULL);
	xcb_intern_atom_cookie_t cookie_decor_all = xcb_intern_atom(display_info->connection, 0, 21,
			"_NET_WM_DECORATION_ALL");
	xcb_intern_atom_reply_t *reply_decor_all = xcb_intern_atom_reply(display_info->connection, cookie_decor_all, NULL);

	xcb_intern_atom_cookie_t cookie_state = xcb_intern_atom(display_info->connection, 0, 13, "_NET_WM_STATE");
	xcb_intern_atom_reply_t *reply_state = xcb_intern_atom_reply(display_info->connection, cookie_state, NULL);

	// set dialog window
	if (reply_type && reply_type_normal && reply_state && reply_decor && reply_decor_all) {
		xcb_delete_property(display_info->connection, display_info->window, reply_state->atom);
		xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window, reply_type->atom,
				XCB_ATOM_ATOM, 32, 1, &(reply_type_normal->atom));
		xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window, reply_decor->atom,
				XCB_ATOM_CARDINAL, 32, 1, &reply_decor_all->atom);
		free(reply_type);
		free(reply_type_normal);
		free(reply_state);
		free(reply_decor);
		free(reply_decor_all);
		p_x11_window_set_dimensions(display_info, 0, 0, display_info->screen->width_in_pixels,
				display_info->screen->height_in_pixels);

	} else {
		fprintf(stderr, "Unable to make windowed fullscreen\n");
		exit(1);
	}
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
	xcb_intern_atom_cookie_t cookie_type = xcb_intern_atom(display_info->connection, 0, 19, "_NET_WM_WINDOW_TYPE");
	xcb_intern_atom_reply_t *reply_type = xcb_intern_atom_reply(display_info->connection, cookie_type, NULL);
	xcb_intern_atom_cookie_t cookie_type_normal = xcb_intern_atom(display_info->connection, 0, 26,
			"_NET_WM_WINDOW_TYPE_NORMAL");
	xcb_intern_atom_reply_t *reply_type_normal = xcb_intern_atom_reply(display_info->connection, cookie_type_normal,
			NULL);

	xcb_intern_atom_cookie_t cookie_state = xcb_intern_atom(display_info->connection, 0, 13, "_NET_WM_STATE");
	xcb_intern_atom_reply_t *reply_state = xcb_intern_atom_reply(display_info->connection, cookie_state, NULL);

	xcb_intern_atom_cookie_t cookie_decor = xcb_intern_atom(display_info->connection, 0, 17, "_NET_WM_DECORATION");
	xcb_intern_atom_reply_t *reply_decor = xcb_intern_atom_reply(display_info->connection, cookie_decor, NULL);


	// set normal window
	if (reply_type && reply_type_normal && reply_state) {
		xcb_delete_property(display_info->connection, display_info->window, reply_state->atom);
		xcb_delete_property(display_info->connection, display_info->window, reply_decor->atom);
		xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window, reply_type->atom,
				XCB_ATOM_ATOM, 32, 1, &(reply_type_normal->atom));
		free(reply_type);
		free(reply_type_normal);
		free(reply_state);
		free(reply_decor);

		p_x11_window_set_dimensions(display_info, x, y, width, height);

	} else {
		fprintf(stderr, "Unable to make windowed\n");
		exit(1);
	}
	xcb_unmap_window(display_info->connection, display_info->window);
	xcb_map_window(display_info->connection, display_info->window);
	xcb_flush(display_info->connection);
}
