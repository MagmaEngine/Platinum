#include "phantom.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libudev.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

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

	//struct udev_monitor *udev_monitor = udev_monitor_new_from_netlink(input_manager->udev, "udev");
	//udev_monitor_filter_add_match_subsystem_devtype(udev_monitor, "input", NULL);
	//udev_monitor_enable_receiving(udev_monitor);
	//e_dynarr_add(input_manager->udev_monitors, udev_monitor);

	//// Create a udev monitor for input devices
	//struct udev_monitor *udev_monitor = udev_monitor_new_from_netlink(udev, "udev");
	//udev_monitor_filter_add_match_subsystem_devtype(udev_monitor, "input", NULL);
	//udev_monitor_enable_receiving(udev_monitor);

	return input_manager;
}

void p_linux_event_deinit(PDeviceManager *input_manager)
{
	//input_manager->udev_monitors = udev_monitor_unref(input_manager->udev_monitor);
	udev_unref(udev);
	free(input_manager);
}
