#ifndef _PLATINUM_GRAPHICS_H
#define _PLATINUM_GRAPHICS_H

#include <enigma.h>

// External Forward Declarations
typedef struct PWindowData PWindowData;

// Internal Forward Declarations
typedef struct PGraphicalAppRequest PGraphicalAppRequest;
typedef struct PGraphicalDisplayRequest PGraphicalDisplayRequest;
typedef struct PGraphicalDevice PGraphicalDevice;

typedef struct PGraphicalAppData *PGraphicalAppData;
typedef struct PGraphicalDisplayData *PGraphicalDisplayData;

struct PGraphicalAppRequest {
	bool headless;
};

struct PGraphicalDisplayRequest {
	bool headless;
	bool stereoscopic;
};

struct PGraphicalDevice {
	char *name;
	void *handle; // backend-specific handle (for vulkan its VkPhysicalDevice)
};


PGraphicalAppData p_graphics_init(PGraphicalAppRequest *graphical_app_request);
void p_graphics_deinit(PGraphicalAppData graphical_app_data);
void p_graphics_display_create(PWindowData *window_data, const PGraphicalAppData graphical_app_data,
		const PGraphicalDisplayRequest * const graphical_display_request);
void p_graphics_display_destroy(PGraphicalDisplayData graphical_display_data);
void p_graphics_device_set(
		PGraphicalDisplayData graphical_display_data,
		const PGraphicalDisplayRequest * const graphical_display_request,
		const PGraphicalDevice physical_device,
		const PGraphicalDisplayData display,
		const PWindowData * const window_data);
PGraphicalDevice p_graphics_device_auto_pick(
		PGraphicalDisplayData graphical_display_data,
		const PGraphicalAppData graphical_app_data,
		const PGraphicalDisplayRequest * const graphical_display_request);



// Vulkan specific implementation
#ifdef PLATINUM_GRAPHICS_VULKAN

PGraphicalAppData p_graphics_vulkan_init(PGraphicalAppRequest *graphical_app_request);

void p_graphics_vulkan_deinit(PGraphicalAppData vulkan_app_data);

void p_graphics_vulkan_display_create(PWindowData *window_data, const PGraphicalAppData vulkan_app_data,
		const PGraphicalDisplayRequest * const vulkan_display_request);

void p_graphics_vulkan_display_destroy(PGraphicalDisplayData vulkan_display_data);

void p_graphics_vulkan_device_set(
		PGraphicalDisplayData graphical_display_data,
		const PGraphicalDisplayRequest * const graphical_display_request,
		const PGraphicalDevice physical_device,
		const PGraphicalDisplayData display,
		const PWindowData * const window_data);

PGraphicalDevice p_graphics_vulkan_device_auto_pick(
		PGraphicalDisplayData graphical_display_data,
		const PGraphicalAppData graphical_app_data,
		const PGraphicalDisplayRequest * const graphical_display_request);

#endif // PLATINUM_GRAPHICS_VULKAN

#endif // _PLATINUM_GRAPHICS_H
