#include "platinum.h"
#include <string.h>

/**
 * p_graphics_init
 *
 * Initializes the graphics subsystem.
 * returns a PGraphicalAppData
 */
PGraphicalAppData p_graphics_init(PGraphicalAppRequest *graphical_app_request)
{
#ifdef PLATINUM_GRAPHICS_VULKAN
	return p_graphics_vulkan_init(graphical_app_request);
#endif // PLATINUM_GRAPHICS
}

/**
 * p_graphics_deinit
 *
 * Deinitializes the graphics subsystem.
 */
void p_graphics_deinit(PGraphicalAppData graphical_app_data)
{
#ifdef PLATINUM_GRAPHICS_VULKAN
	p_graphics_vulkan_deinit(graphical_app_data);
#endif // PLATINUM_GRAPHICS
}

/**
 * p_graphics_display_create
 *
 * Creates a display based on the platform
 * and assigns it to window_data
 */
void p_graphics_display_create(PWindowData *window_data, const PGraphicalAppData graphical_app_data,
		const PGraphicalDisplayRequest * const graphical_display_request)
{
#ifdef PLATINUM_GRAPHICS_VULKAN
	p_graphics_vulkan_display_create(window_data, graphical_app_data, graphical_display_request);
#endif // PLATINUM_GRAPHICS
}

/**
 * p_graphics_display_destroy
 *
 * Destroys the display
 * and removes it from window_data
 */
void p_graphics_display_destroy(PGraphicalDisplayData graphical_display_data)
{
#ifdef PLATINUM_GRAPHICS_VULKAN
	p_graphics_vulkan_display_destroy(graphical_display_data);
#endif // PLATINUM_GRAPHICS
}

/**
 * p_graphics_device_set
 *
 * sets the physical device to use in graphical_display_data from physical_device
 */
void p_graphics_device_set(
		PGraphicalDisplayData graphical_display_data,
		const PGraphicalDisplayRequest * const graphical_display_request,
		const PGraphicalDevice physical_device,
		const PGraphicalDisplayData display,
		const PWindowData * const window_data)
{
#ifdef PLATINUM_GRAPHICS_VULKAN
	p_graphics_vulkan_device_set(graphical_display_data, graphical_display_request, physical_device,
			display, window_data);
#endif // PLATINUM_GRAPHICS
}

/**
 * p_graphics_device_auto_pick
 *
 * initializes compatible_devices in graphical_display_data
 * picks the physical device for use with vulkan and returns it
 * returns a PGraphicalDevice
 */
PGraphicalDevice p_graphics_device_auto_pick(
		PGraphicalDisplayData graphical_display_data,
		const PGraphicalAppData graphical_app_data,
		const PGraphicalDisplayRequest * const graphical_display_request)
{
#ifdef PLATINUM_GRAPHICS_VULKAN
	return p_graphics_vulkan_device_auto_pick(graphical_display_data, graphical_app_data,
			graphical_display_request);
#endif // PLATINUM_GRAPHICS
}
