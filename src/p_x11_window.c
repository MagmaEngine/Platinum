#include "phantom.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <xcb/xcb.h>

PDisplayInfo *p_x11_window_create(PWindowSettings *ws)
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

	PDisplayInfo *di = malloc(sizeof *di);
	di->connection = connection;
	di->screen = screen;
	di->window = window;

	// convert game engine vars to xcb vars
	uint class;
	switch (ws->interact_type)
	{
		case P_INTERACT_INPUT:
			class = XCB_WINDOW_CLASS_INPUT_ONLY;
		break;

		case P_INTERACT_INPUT_OUTPUT:
			class = XCB_WINDOW_CLASS_INPUT_OUTPUT;
		break;
	}

	// creates the window
	uint border_width = 0;
	xcb_create_window(
			connection,
			XCB_COPY_FROM_PARENT,
			window,
			screen->root,
			ws->x, ws->y, ws->width, ws->height,
			border_width,
			class,
			screen->root_visual,
			XCB_CW_BACK_PIXEL, &screen->black_pixel);

	// Set display based on type
	switch (ws->display_type)
	{
		case P_DISPLAY_WINDOWED:
			p_x11_window_windowed(di, ws);
		break;

		case P_DISPLAY_WINDOWED_FULLSCREEN:
			p_x11_window_windowed_fullscreen(di, ws);
		break;

		case P_DISPLAY_FULLSCREEN:
			p_x11_window_fullscreen(di, ws);
		break;
	}

	return di;
}

void p_x11_window_close(PDisplayInfo *di)
{
    xcb_unmap_window(di->connection, di->window);
	xcb_disconnect(di->connection);
	free(di);
}

void p_x11_window_fullscreen(PDisplayInfo *di, PWindowSettings *ws)
{
	xcb_intern_atom_cookie_t cookie_type = xcb_intern_atom(di->connection, 0, 19, "_NET_WM_WINDOW_TYPE");
	xcb_intern_atom_reply_t *reply_type = xcb_intern_atom_reply(di->connection, cookie_type, NULL);
	xcb_intern_atom_cookie_t cookie_normal = xcb_intern_atom(di->connection, 0, 26, "_NET_WM_WINDOW_TYPE_NORMAL");
	xcb_intern_atom_reply_t *reply_normal = xcb_intern_atom_reply(di->connection, cookie_normal, NULL);

	xcb_intern_atom_cookie_t cookie_state = xcb_intern_atom(di->connection, 0, 13, "_NET_WM_STATE");
	xcb_intern_atom_reply_t *reply_state = xcb_intern_atom_reply(di->connection, cookie_state, NULL);
	xcb_intern_atom_cookie_t cookie_fullscreen = xcb_intern_atom(di->connection, 0, 24, "_NET_WM_STATE_FULLSCREEN");
	xcb_intern_atom_reply_t *reply_fullscreen = xcb_intern_atom_reply(di->connection, cookie_fullscreen, NULL);

	// set normal fullscreen window
	if (reply_type && reply_normal && reply_state && reply_fullscreen) {
		xcb_change_property(di->connection, XCB_PROP_MODE_REPLACE, di->window, reply_type->atom,
				XCB_ATOM_ATOM, 32, 1, &(reply_normal->atom));
		free(reply_type);
		free(reply_normal);
        xcb_change_property(di->connection, XCB_PROP_MODE_REPLACE, di->window, reply_state->atom,
				XCB_ATOM_ATOM, 32, 1, &(reply_fullscreen->atom));
		free(reply_state);
		free(reply_fullscreen);
		ws->display_type = P_DISPLAY_FULLSCREEN;
	} else {
		fprintf(stderr, "Unable to make fullscreen\n");
		exit(1);
	}
    xcb_unmap_window(di->connection, di->window);
	xcb_map_window(di->connection, di->window);
	xcb_flush(di->connection);
}

void p_x11_window_windowed_fullscreen(PDisplayInfo *di, PWindowSettings *ws)
{
	xcb_intern_atom_cookie_t cookie_type = xcb_intern_atom(di->connection, 0, 19, "_NET_WM_WINDOW_TYPE");
	xcb_intern_atom_reply_t *reply_type = xcb_intern_atom_reply(di->connection, cookie_type, NULL);
	xcb_intern_atom_cookie_t cookie_dialog = xcb_intern_atom(di->connection, 0, 26, "_NET_WM_WINDOW_TYPE_DIALOG");
	xcb_intern_atom_reply_t *reply_dialog = xcb_intern_atom_reply(di->connection, cookie_dialog, NULL);

	xcb_intern_atom_cookie_t cookie_state = xcb_intern_atom(di->connection, 0, 13, "_NET_WM_STATE");
	xcb_intern_atom_reply_t *reply_state = xcb_intern_atom_reply(di->connection, cookie_state, NULL);

	// set dialog window
	if (reply_type && reply_dialog && reply_state) {
		xcb_delete_property(di->connection, di->window, reply_state->atom);
		xcb_change_property(di->connection, XCB_PROP_MODE_REPLACE, di->window, reply_type->atom,
				XCB_ATOM_ATOM, 32, 1, &(reply_dialog->atom));
		free(reply_type);
		free(reply_dialog);
		free(reply_state);
		ws->display_type = P_DISPLAY_WINDOWED_FULLSCREEN;
		ws->x = 0;
		ws->y = 0;
		ws->width = di->screen->width_in_pixels;
		ws->height = di->screen->height_in_pixels;

		// resize
		uint32_t params[4] = {ws->x, ws->y, ws->width, ws->height};
		xcb_configure_window(di->connection, di->window,
				XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
				params);
	} else {
		fprintf(stderr, "Unable to make fullscreen\n");
		exit(1);
	}
    xcb_unmap_window(di->connection, di->window);
	xcb_map_window(di->connection, di->window);
	xcb_flush(di->connection);
}

void p_x11_window_windowed(PDisplayInfo *di, PWindowSettings *ws)
{
	xcb_intern_atom_cookie_t cookie_type = xcb_intern_atom(di->connection, 0, 19, "_NET_WM_WINDOW_TYPE");
	xcb_intern_atom_reply_t *reply_type = xcb_intern_atom_reply(di->connection, cookie_type, NULL);
	xcb_intern_atom_cookie_t cookie_normal = xcb_intern_atom(di->connection, 0, 26, "_NET_WM_WINDOW_TYPE_NORMAL");
	xcb_intern_atom_reply_t *reply_normal = xcb_intern_atom_reply(di->connection, cookie_normal, NULL);

	xcb_intern_atom_cookie_t cookie_state = xcb_intern_atom(di->connection, 0, 13, "_NET_WM_STATE");
	xcb_intern_atom_reply_t *reply_state = xcb_intern_atom_reply(di->connection, cookie_state, NULL);

	// set normal window
	if (reply_type && reply_normal && reply_state) {
		xcb_delete_property(di->connection, di->window, reply_state->atom);
		xcb_change_property(di->connection, XCB_PROP_MODE_REPLACE, di->window, reply_type->atom,
				XCB_ATOM_ATOM, 32, 1, &(reply_normal->atom));
		free(reply_type);
		free(reply_normal);
		free(reply_state);
		ws->display_type = P_DISPLAY_WINDOWED;
		ws->x = di->screen->width_in_pixels/4;
		ws->y = di->screen->height_in_pixels/4;
		ws->width = di->screen->width_in_pixels/2;
		ws->height = di->screen->height_in_pixels/2;

		// resize
		uint32_t params[4] = {ws->x, ws->y, ws->width, ws->height};
		xcb_configure_window(di->connection, di->window,
				XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
				params);
	} else {
		fprintf(stderr, "Unable to make fullscreen\n");
		exit(1);
	}
    xcb_unmap_window(di->connection, di->window);
	xcb_map_window(di->connection, di->window);
	xcb_flush(di->connection);
}
