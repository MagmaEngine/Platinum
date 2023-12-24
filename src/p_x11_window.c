#include "phantom.h"
#include <enigma.h>
#include <threads.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <xcb/xcb.h>

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
		e_log_message(E_LOG_ERROR, L"Phantom", L"Could not get atom reply...");
		exit(1);
	}
	xcb_atom_t atom = reply->atom;
	free(reply);
	return atom;
}

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
 * p_x11_window_set_name
 *
 * sets the name of the window
 */
void p_x11_window_set_name(PDisplayInfo *display_info, const wchar_t *name)
{
	// convert name to char*
	size_t utf8_size = wcstombs(NULL, name, 0);
	char *utf8_name = malloc(utf8_size + 1);
	wcstombs(utf8_name, name, utf8_size);
	utf8_name[utf8_size] = '\0';

	xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window, XCB_ATOM_WM_NAME,
			display_info->atoms[P_ATOM_UTF8_STRING], 8, utf8_size, utf8_name);

	xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window,
			display_info->atoms[P_ATOM_NET_WM_NAME], display_info->atoms[P_ATOM_UTF8_STRING], 8, utf8_size, utf8_name);

	free(utf8_name);
}

/**
 * p_x11_window_set_icon
 *
 * sets the icon of the window
 */
void p_x11_window_set_icon(PDisplayInfo *display_info, uint32_t *icon)
{
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
		e_log_message(E_LOG_ERROR, L"Phantom", L"Unable to open the connection to the X server");
		exit(1);
	}

	xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
	xcb_window_t window = xcb_generate_id(connection);

	PDisplayInfo *display_info = malloc(sizeof *display_info);
	display_info->connection = connection;
	display_info->screen = screen;
	display_info->window = window;
	display_info->atoms[P_ATOM_UTF8_STRING] = p_x11_generate_atom(display_info->connection, "UTF8_STRING");
	display_info->atoms[P_ATOM_NET_WM_NAME] = p_x11_generate_atom(display_info->connection, "_NET_WM_NAME");
	display_info->atoms[P_ATOM_NET_WM_STATE] = p_x11_generate_atom(display_info->connection, "_NET_WM_STATE");
	display_info->atoms[P_ATOM_NET_WM_STATE_FULLSCREEN] = p_x11_generate_atom(display_info->connection,
			"_NET_WM_STATE_FULLSCREEN");
	display_info->atoms[P_ATOM_NET_WM_DECORATION] = p_x11_generate_atom(display_info->connection,
			"_NET_WM_DECORATION");
	display_info->atoms[P_ATOM_NET_WM_DECORATION_ALL] = p_x11_generate_atom(display_info->connection,
			"_NET_WM_DECORATION_ALL");
	display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE] = p_x11_generate_atom(display_info->connection,
			"_NET_WM_WINDOW_TYPE");
	display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE_NORMAL] = p_x11_generate_atom(display_info->connection,
			"_NET_WM_WINDOW_TYPE_NORMAL");
	display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE_DIALOG] = p_x11_generate_atom(display_info->connection,
			"_NET_WM_WINDOW_TYPE_DIALOG");
	display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE_DOCK] = p_x11_generate_atom(display_info->connection,
			"_NET_WM_WINDOW_TYPE_DOCK");
	display_info->atoms[P_ATOM_MOTIF_WM_HINTS] = p_x11_generate_atom(display_info->connection,
			"_MOTIF_WM_HINTS");

	for (uint i = 0; i < P_ATOM_MAX; i++)
	{
		if (display_info->atoms[i] == XCB_ATOM_NONE)
		{
			e_log_message(E_LOG_ERROR, L"Phantom", L"XCB atoms not initialized. Aborting...");
			exit(1);
		}
	}

	uint class = P_INTERACT_INPUT_OUTPUT;
	uint border_width = 0;

	PWindowSettings *window_settings = malloc(sizeof *window_settings);
	window_settings->name = malloc((wcslen(window_request.name)+1) * sizeof(wchar_t));
	wcscpy(window_settings->name, window_request.name);
	window_settings->x = e_mini(e_maxi(window_request.x, 0), screen->width_in_pixels);
	window_settings->y = e_mini(e_maxi(window_request.y, 0), screen->height_in_pixels);
	window_settings->width = e_mini(e_maxi(window_request.width, 1), screen->width_in_pixels);
	window_settings->height = e_mini(e_maxi(window_request.height, 1), screen->width_in_pixels);
	window_settings->display_type = window_request.display_type;
	window_settings->interact_type = window_request.interact_type;
	window_settings->display_info = display_info;
	window_settings->event_calls = calloc(1, sizeof *window_settings->event_calls);
	memcpy(window_settings->event_calls, &window_request.event_calls, sizeof *window_settings->event_calls);
	window_settings->status = P_WINDOW_ALIVE;

	// creates the window
	xcb_create_window(
			connection,
			XCB_COPY_FROM_PARENT,
			window,
			screen->root,
			window_settings->x, window_settings->y, window_settings->width, window_settings->height,
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

	p_window_set_name(display_info, window_settings->name);

	// Set display based on type
	switch (window_request.display_type)
	{
		case P_DISPLAY_WINDOWED:
			p_window_windowed(window_settings, window_request.x, window_request.y, window_request.width,
					window_request.height);
		break;

		case P_DISPLAY_DOCKED_FULLSCREEN:
			p_window_docked_fullscreen(window_settings);
		break;

		case P_DISPLAY_FULLSCREEN:
			p_window_fullscreen(window_settings);
		break;

		case P_DISPLAY_MAX:
			e_log_message(E_LOG_WARNING, L"Phantom", L"P_DISPLAY_MAX is not a valid window type...");
			exit(1);
	}

	e_mutex_lock(app_instance->window_mutex);
	e_dynarr_add(app_instance->window_settings, &window_settings);
	e_mutex_unlock(app_instance->window_mutex);

	// Start the window event manager
	EThreadArguments args = malloc(sizeof (PAppInstance *) + sizeof (PWindowSettings *));
	memcpy(args, &app_instance, sizeof (PAppInstance **));
	memcpy(&((char *)args)[sizeof (PAppInstance **)], &window_settings, sizeof (PWindowSettings **));
	window_settings->event_manager = e_thread_create(p_window_event_manage, args);
}

/**
 * p_x11_window_close
 *
 * sends a signal to close the window and the connection to the Xserver
 */
void p_x11_window_close(PWindowSettings *window_settings)
{
	window_settings->status = P_WINDOW_INTERNAL_CLOSE;
	xcb_destroy_window(window_settings->display_info->connection, window_settings->display_info->window);
	xcb_flush(window_settings->display_info->connection);
}


/**
 * _x11_window_close
 *
 * internal close function that actually closes the window and frees data
 */
void _x11_window_close(PAppInstance *app_instance, PWindowSettings *window_settings)
{
	// If window exists delete it.
	e_mutex_lock(app_instance->window_mutex);
	int index = e_dynarr_contains(app_instance->window_settings, &window_settings);
	if (index == -1)
	{
		e_log_message(E_LOG_ERROR, L"Phantom", L"Window does not exist...");
		exit(1);
	}
	PDisplayInfo *display_info = window_settings->display_info;
	xcb_unmap_window(display_info->connection, display_info->window);
	xcb_disconnect(display_info->connection);
	free(window_settings->display_info);
	if (window_settings->status == P_WINDOW_CLOSE)
	{
		free(window_settings->event_calls);
		free(window_settings->name);
		free(window_settings);
		e_thread_detach(e_thread_self());
	}
	e_dynarr_remove_unordered(app_instance->window_settings, index);
	e_mutex_unlock(app_instance->window_mutex);
}

/**
 * p_x11_window_event_manage
 *
 * This function runs in its own thread and manages window manager events
 * returns NULL
 */
EThreadResult p_x11_window_event_manage(EThreadArguments args)
{
	// unpack args
	PAppInstance *app_instance = ((PAppInstance **)args)[0];
	PWindowSettings *window_settings = *(PWindowSettings **)&((char *)args)[sizeof (PAppInstance **)];
	//PWindowSettings *window_settings = ((PWindowSettings **)args)[1];
	PDisplayInfo *display_info = window_settings->display_info;
	PEventCalls *event_calls = window_settings->event_calls;


	while (window_settings->status == P_WINDOW_ALIVE) {
		xcb_generic_event_t *event = xcb_wait_for_event(display_info->connection);
		if (!event)
		{
			e_log_message(E_LOG_WARNING, L"Phantom", L"Event was null...");
			e_mutex_lock(app_instance->window_mutex);
			if (window_settings->status == P_WINDOW_ALIVE)
				window_settings->status = P_WINDOW_CLOSE;
			e_mutex_unlock(app_instance->window_mutex);
			_x11_window_close(app_instance, window_settings);
			break;
		}
		switch (event->response_type & ~0x80)
		{
			// TODO: experiment with capturing mouse and keyboard
			case XCB_EXPOSE:
			{
				e_log_message(E_LOG_DEBUG, L"Phantom", L"Expose Event triggered");
				xcb_expose_event_t *expose_event = (xcb_expose_event_t *)event;
				E_UNUSED(expose_event);
				// TODO: redraw (only new part?)
				if (event_calls->enable_expose && event_calls->expose != NULL)
					event_calls->expose();
				break;
			}
			case XCB_CONFIGURE_NOTIFY:
			{
				e_log_message(E_LOG_DEBUG, L"Phantom", L"Configure Notify Event triggered");
				xcb_configure_notify_event_t *config_notify_event = (xcb_configure_notify_event_t *)event;
				window_settings->x = config_notify_event->x;
				window_settings->y = config_notify_event->y;
				window_settings->width = config_notify_event->width;
				window_settings->height = config_notify_event->height;
				if (event_calls->enable_configure && event_calls->configure != NULL)
					event_calls->configure();
				break;
			}
			case XCB_PROPERTY_NOTIFY:
			{
				e_log_message(E_LOG_DEBUG, L"Phantom", L"Property Notify Event triggered");
				xcb_property_notify_event_t *property_notify_event = (xcb_property_notify_event_t *)event;
				E_UNUSED(property_notify_event);

				xcb_get_property_cookie_t property_cookie_state = xcb_get_property(display_info->connection, 0,
						display_info->window, display_info->atoms[P_ATOM_NET_WM_STATE], XCB_ATOM_ANY, 0, 1024);
				xcb_get_property_reply_t *property_reply_state = xcb_get_property_reply(display_info->connection,
						property_cookie_state, NULL);

				xcb_get_property_cookie_t property_cookie_type = xcb_get_property(display_info->connection, 0,
						display_info->window, display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE], XCB_ATOM_ANY, 0,
						1024);
				xcb_get_property_reply_t *property_reply_type = xcb_get_property_reply(display_info->connection,
						property_cookie_type, NULL);

				if (property_reply_state && property_reply_type)
				{
					xcb_atom_t *state_state = (xcb_atom_t *)xcb_get_property_value(property_reply_state);
					xcb_atom_t *state_type = (xcb_atom_t *)xcb_get_property_value(property_reply_type);
					int num_state_atoms = xcb_get_property_value_length(property_reply_state) / sizeof(xcb_atom_t);
					int num_type_atoms = xcb_get_property_value_length(property_reply_type) / sizeof(xcb_atom_t);

					// Check what atoms are present
					bool is_fullscreen = false;
					bool is_normal = false;
					bool is_dock = false;
					for (int i = 0; i < num_state_atoms; i++)
						is_fullscreen |= (state_state[i] == display_info->atoms[P_ATOM_NET_WM_STATE_FULLSCREEN]);
					for (int i = 0; i < num_type_atoms; i++)
					{
						is_dock |= (state_type[i] == display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE_DOCK]);
						is_normal |= (state_type[i] == display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE_NORMAL]);
					}

					free(property_reply_state);
					free(property_reply_type);

					if (is_fullscreen)
						window_settings->display_type = P_DISPLAY_FULLSCREEN;
					else if (is_normal)
						window_settings->display_type = P_DISPLAY_WINDOWED;
					else if (is_dock)
						window_settings->display_type = P_DISPLAY_DOCKED_FULLSCREEN;
				}

				if (event_calls->enable_property && event_calls->property != NULL)
					event_calls->property();
				break;
			}
			case XCB_CLIENT_MESSAGE:
			{
				e_log_message(E_LOG_DEBUG, L"Phantom", L"Client Message Event triggered");
				xcb_client_message_event_t *message_event = (xcb_client_message_event_t *)event;
				E_UNUSED(message_event);
				if (event_calls->enable_client && event_calls->client != NULL)
					event_calls->client();
				break;
			}
			case XCB_FOCUS_IN:
			{
				e_log_message(E_LOG_DEBUG, L"Phantom", L"Focus In Event triggered");
				xcb_focus_in_event_t *focus_in_event = (xcb_focus_in_event_t *)event;
				E_UNUSED(focus_in_event);
				if (event_calls->enable_focus_in && event_calls->focus_in != NULL)
					event_calls->focus_in();
				break;
			}
			case XCB_FOCUS_OUT:
			{
				e_log_message(E_LOG_DEBUG, L"Phantom", L"Focus Out Event triggered");
				xcb_focus_out_event_t *focus_out_event = (xcb_focus_out_event_t *)event;
				E_UNUSED(focus_out_event);
				if (event_calls->enable_focus_out && event_calls->focus_out != NULL)
					event_calls->focus_out();
				break;
			}
			case XCB_ENTER_NOTIFY:
			{
				e_log_message(E_LOG_DEBUG, L"Phantom", L"Enter Notify Event triggered");
				xcb_enter_notify_event_t *enter_notify_event = (xcb_enter_notify_event_t *)event;
				E_UNUSED(enter_notify_event);
				if (event_calls->enable_enter && event_calls->enter != NULL)
					event_calls->enter();
				break;
			}
			case XCB_LEAVE_NOTIFY:
			{
				e_log_message(E_LOG_DEBUG, L"Phantom", L"Leave Notify Event triggered");
				xcb_leave_notify_event_t *leave_notify_event = (xcb_leave_notify_event_t *)event;
				E_UNUSED(leave_notify_event);
				if (event_calls->enable_leave && event_calls->leave != NULL)
					event_calls->leave();
				break;
			}
			case XCB_DESTROY_NOTIFY:
			{
				e_log_message(E_LOG_DEBUG, L"Phantom", L"Destroy Notify Event triggered");
				xcb_destroy_notify_event_t *destroy_notify_event = (xcb_destroy_notify_event_t *)event;
				E_UNUSED(destroy_notify_event);
				// TODO: handle errors

				if (event_calls->enable_destroy && event_calls->destroy != NULL)
					event_calls->destroy();

				e_mutex_lock(app_instance->window_mutex);
				if (window_settings->status == P_WINDOW_ALIVE)
					window_settings->status = P_WINDOW_CLOSE;
				e_mutex_unlock(app_instance->window_mutex);
				_x11_window_close(app_instance, window_settings);

				break;
			}
			default:
			{
				e_log_message(E_LOG_DEBUG, L"Phantom", L"Unknown Event triggered.");
				break;
			}
		}
		free(event);
	}
	free(args);
	return NULL;
}

/**
 * p_x11_window_fullscreen
 *
 * sets the window in window_settings to fullscreen
 */
void p_x11_window_fullscreen(PWindowSettings *window_settings)
{
	PDisplayInfo *display_info = window_settings->display_info;

	// set normal fullscreen window
	xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window,
			display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE], XCB_ATOM_ATOM, 32, 1,
			&display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE_NORMAL]);
	xcb_client_message_event_t ev_type;
	ev_type.response_type = XCB_CLIENT_MESSAGE;
	ev_type.format = 32;
	ev_type.sequence = 0;
	ev_type.window = display_info->window;
	ev_type.type = display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE];
	ev_type.data.data32[0] = 1;
	ev_type.data.data32[1] = display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE_NORMAL];
	ev_type.data.data32[2] = 0;
	xcb_send_event(display_info->connection, 0, display_info->screen->root, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
			(char*)&ev_type);

	xcb_unmap_window(display_info->connection, display_info->window);
	xcb_map_window(display_info->connection, display_info->window);

	xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window,
			display_info->atoms[P_ATOM_NET_WM_STATE], XCB_ATOM_ATOM, 32, 1,
			&display_info->atoms[P_ATOM_NET_WM_STATE_FULLSCREEN]);
	xcb_client_message_event_t ev_state;
	ev_state.response_type = XCB_CLIENT_MESSAGE;
	ev_state.format = 32;
	ev_state.sequence = 0;
	ev_state.window = display_info->window;
	ev_state.type = display_info->atoms[P_ATOM_NET_WM_STATE];
	ev_state.data.data32[0] = 1;
	ev_state.data.data32[1] = display_info->atoms[P_ATOM_NET_WM_STATE_FULLSCREEN];
	ev_state.data.data32[2] = 0;
	xcb_send_event(display_info->connection, 0, display_info->screen->root, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
			(char*)&ev_state);

	xcb_flush(display_info->connection);
}

/**
 * p_x11_window_docked_fullscreen
 *
 * sets the window in window_settings to (borderless/windowed) fullscreen
 */
void p_x11_window_docked_fullscreen(PWindowSettings *window_settings)
{
	PDisplayInfo *display_info = window_settings->display_info;
	xcb_unmap_window(display_info->connection, display_info->window);
	xcb_flush(display_info->connection);

	// set windowed fullscreen window
	xcb_delete_property(display_info->connection, display_info->window, display_info->atoms[P_ATOM_NET_WM_STATE]);
	xcb_client_message_event_t ev_state;
	ev_state.response_type = XCB_CLIENT_MESSAGE;
	ev_state.format = 32;
	ev_state.sequence = 0;
	ev_state.window = display_info->window;
	ev_state.type = display_info->atoms[P_ATOM_NET_WM_STATE];
	ev_state.data.data32[0] = 0;
	ev_state.data.data32[1] = display_info->atoms[P_ATOM_NET_WM_STATE_FULLSCREEN];
	ev_state.data.data32[2] = 0;
	xcb_send_event(display_info->connection, 0, display_info->screen->root, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
			(char*)&ev_state);

	xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window,
			display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE], XCB_ATOM_ATOM, 32, 1,
			&display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE_DOCK]);
	xcb_client_message_event_t ev_type;
	ev_type.response_type = XCB_CLIENT_MESSAGE;
	ev_type.format = 32;
	ev_type.sequence = 0;
	ev_type.window = display_info->window;
	ev_type.type = display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE];
	ev_type.data.data32[0] = 1;
	ev_type.data.data32[1] = display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE_DOCK];
	ev_type.data.data32[2] = 0;
	xcb_send_event(display_info->connection, 0, display_info->screen->root, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
			(char*)&ev_type);

	uint x = 0, y = 0, width = display_info->screen->width_in_pixels, height = display_info->screen->height_in_pixels;
	p_x11_window_set_dimensions(display_info, x, y, width, height);
	xcb_configure_notify_event_t configure_event;
	configure_event.response_type = XCB_CONFIGURE_NOTIFY;
	configure_event.event = display_info->window;
	configure_event.window = display_info->window;
	configure_event.above_sibling = XCB_NONE;
	configure_event.x = x;
	configure_event.y = y;
	configure_event.width = width;
	configure_event.height = height;
	configure_event.border_width = 0;
	xcb_send_event(display_info->connection, 0, display_info->screen->root, XCB_EVENT_MASK_STRUCTURE_NOTIFY,
			(const char *)&configure_event);

	xcb_map_window(display_info->connection, display_info->window);
	xcb_flush(display_info->connection);
}

/**
 * p_x11_window_windowed
 *
 * sets the window in window_settings to windowed mode and sets the dimensions
 */
void p_x11_window_windowed(PWindowSettings *window_settings, uint x, uint y, uint width, uint height)
{
	PDisplayInfo *display_info = window_settings->display_info;

	// set normal window
	xcb_delete_property(display_info->connection, display_info->window, display_info->atoms[P_ATOM_NET_WM_STATE]);
	xcb_client_message_event_t ev_state;
	ev_state.response_type = XCB_CLIENT_MESSAGE;
	ev_state.format = 32;
	ev_state.sequence = 0;
	ev_state.window = display_info->window;
	ev_state.type = display_info->atoms[P_ATOM_NET_WM_STATE];
	ev_state.data.data32[0] = 0;
	ev_state.data.data32[1] = display_info->atoms[P_ATOM_NET_WM_STATE_FULLSCREEN];
	ev_state.data.data32[2] = 0;
	xcb_send_event(display_info->connection, 0, display_info->screen->root, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
			(char*)&ev_state);

	xcb_change_property(display_info->connection, XCB_PROP_MODE_REPLACE, display_info->window,
			display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE], XCB_ATOM_ATOM, 32, 1,
			&display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE_NORMAL]);
	xcb_client_message_event_t ev_type;
	ev_type.response_type = XCB_CLIENT_MESSAGE;
	ev_type.format = 32;
	ev_type.sequence = 0;
	ev_type.window = display_info->window;
	ev_type.type = display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE];
	ev_type.data.data32[0] = 1;
	ev_type.data.data32[1] = display_info->atoms[P_ATOM_NET_WM_WINDOW_TYPE_NORMAL];
	ev_type.data.data32[2] = 0;
	xcb_send_event(display_info->connection, 0, display_info->screen->root, XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
			(char*)&ev_type);

	p_x11_window_set_dimensions(display_info, x, y, width, height);
	xcb_configure_notify_event_t configure_event;
	configure_event.response_type = XCB_CONFIGURE_NOTIFY;
	configure_event.event = display_info->window;
	configure_event.window = display_info->window;
	configure_event.above_sibling = XCB_NONE;
	configure_event.x = x;
	configure_event.y = y;
	configure_event.width = width;
	configure_event.height = height;
	configure_event.border_width = 0;
	xcb_send_event(display_info->connection, 0, display_info->screen->root, XCB_EVENT_MASK_STRUCTURE_NOTIFY,
			(const char *)&configure_event);

	xcb_unmap_window(display_info->connection, display_info->window);
	xcb_flush(display_info->connection);
	xcb_map_window(display_info->connection, display_info->window);
	xcb_flush(display_info->connection);
}
