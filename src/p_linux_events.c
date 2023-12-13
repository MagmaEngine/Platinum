#include "phantom.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libudev.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

void p_udev_init(void)
{
	udev_new();

	struct udev *udev = udev_new();
	if (!udev) {
		fprintf(stderr, "Failed to initialize udev\n");
		exit(EXIT_FAILURE);
	}

	// Create a udev monitor for input devices
	struct udev_monitor *udev_monitor = udev_monitor_new_from_netlink(udev, "udev");
	udev_monitor_filter_add_match_subsystem_devtype(udev_monitor, "input", NULL);
	udev_monitor_enable_receiving(udev_monitor);
}

