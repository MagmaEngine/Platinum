#include "phantom.h"
#include <stdio.h>
#include <libudev.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

struct udev *udev;

/**
 * p_linux_event_init
 *
 * initializes the event manager.
 * sets up udev_monitors for input devices.
 * returns a PDeviceManager.
 */
PDeviceManager *p_linux_event_init(void)
{
	udev = udev_new();
	if (!udev) {
		fprintf(stderr, "Failed to initialize udev\n");
		exit(EXIT_FAILURE);
	}
	PDeviceManager *input_manager = malloc(sizeof * input_manager);

	return input_manager;
}

void p_linux_event_deinit(PDeviceManager *input_manager)
{
	udev_unref(udev);
	free(input_manager);
}
