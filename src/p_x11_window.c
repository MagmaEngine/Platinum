#include "phantom.h"
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
 * returns a window_settings and display_info associated with the window.
 */
void p_x11_window_create(const PWindowRequest window_request, PDisplayInfo *display_info,
		PWindowSettings *window_settings)
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

	display_info->connection = connection;
	display_info->screen = screen;
	display_info->window = window;

	uint class = P_INTERACT_INPUT_OUTPUT;
	uint border_width = 0;

	window_settings->name = malloc((wcslen(window_request.name)+1) * sizeof(wchar_t));
	wcscpy(window_settings->name, window_request.name);
	window_settings->x = window_request.x;
	window_settings->y = window_request.y;
	window_settings->width = window_request.width;
	window_settings->height = window_request.height;
	window_settings->display_type = window_request.display_type;
	window_settings->interact_type = window_request.interact_type;

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

	// Set display based on type
	switch (window_request.display_type)
	{
		case P_DISPLAY_WINDOWED:
			p_x11_window_windowed(display_info, window_request.x, window_request.y, window_request.width,
					window_request.height);
				// display_info->screen->width_in_pixels/8,
				// display_info->screen->height_in_pixels/8,
				// display_info->screen->width_in_pixels * 0.75,
				// display_info->screen->height_in_pixels * 0.75);
		break;

		case P_DISPLAY_WINDOWED_FULLSCREEN:
			p_x11_window_windowed_fullscreen(display_info);
		break;

		case P_DISPLAY_FULLSCREEN:
			p_x11_window_fullscreen(display_info);
		break;
	}

	return;
}

/**
 * p_x11_window_close
 *
 * closes the window and the connection to the Xserver
 */
void p_x11_window_close(PDisplayInfo *display_info, PWindowSettings *window_settings)
{
	xcb_unmap_window(display_info->connection, display_info->window);
	xcb_disconnect(display_info->connection);
	free(window_settings->name);
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
