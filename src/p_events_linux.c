#include "platinum.h"
#include <libudev.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

struct udev *udev;

///**
// * p_linux_event_init
// *
// * initializes the event manager.
// * sets up udev_monitors for input devices.
// * returns a PDeviceManager.
// */
//PDeviceManager *p_linux_event_init(void)
//{
//	udev = udev_new();
//	if (!udev) {
//		p_log_message(P_LOG_ERROR, L"Phantom", L"Failed to initialize udev");
//		exit(EXIT_FAILURE);
//	}
//	PDeviceManager *input_manager = malloc(sizeof * input_manager);
//
//	return input_manager;
//}
//
///**
// * p_linux_event_deinit
// *
// * deinitializes the event manager.
// * frees input device stuff
// */
//void p_linux_event_deinit(PDeviceManager *input_manager)
//{
//	udev_unref(udev);
//	free(input_manager);
//}
