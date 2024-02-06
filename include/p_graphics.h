#ifndef _PLATINUM_GRAPHICS_H
#define _PLATINUM_GRAPHICS_H

#include "enigma.h"

// External Forward Declarations
typedef struct PWindowData PWindowData;

// Internal Forward Declarations
typedef struct PGraphicalAppRequest PGraphicalAppRequest;
typedef struct PGraphicalDisplayRequest PGraphicalDisplayRequest;
typedef struct PGraphicalDevice PGraphicalDevice;

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


#ifdef PLATINUM_BACKEND_VULKAN

#include <vulkan/vulkan.h>

/**
 * PVulkanAppRequest
 *
 * This struct is made with PGraphicalAppRequest
 */
typedef struct {
	EDynarr *required_extensions; // contains char *
	EDynarr *required_layers; // contains char *
} PVulkanAppRequest;

/**
 * PVulkanDisplayRequest
 *
 * This struct is used to create a new window surface with the requested settings
 */
typedef struct {
	bool stereoscopic;
	VkBool32 require_present;

	VkQueueFlags required_queue_flags;
	VkPhysicalDeviceFeatures required_features;
	EDynarr *required_extensions; // contains char *
	EDynarr *required_layers; // contains char *
} PVulkanDisplayRequest;

/**
 * PVulkanSwapchainSupport
 *
 * This struct is used to hold the capabilities of a vulkan swap chain
 */
typedef struct {
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR *format; // pointer because NULL if none
	VkPresentModeKHR *present_mode; // pointer because NULL if none
} PVulkanSwapchainSupport;

/**
 * PVulkanQueueFamilyInfo
 *
 * This struct is used to store information about the queue family
 */
typedef struct {
	VkQueueFlags flags;
	VkBool32 exists;
	uint32_t index;
	VkQueue queue;
} PVulkanQueueFamilyInfo;

enum PVulkanQueueType {
	P_VULKAN_QUEUE_TYPE_GRAPHICS,
	P_VULKAN_QUEUE_TYPE_PRESENT,
	P_VULKAN_QUEUE_TYPE_COMPUTE,
	P_VULKAN_QUEUE_TYPE_TRANSFER,
	P_VULKAN_QUEUE_TYPE_SPARSE,
	P_VULKAN_QUEUE_TYPE_PROTECTED,
	P_VULKAN_QUEUE_TYPE_VIDEO_DECODE,
	P_VULKAN_QUEUE_TYPE_OPTICAL_FLOW,
	P_VULKAN_QUEUE_TYPE_MAX
};

/**
 * PVulkanDisplayData
 *
 * This struct contains data relevant to a vulkan display
 */
typedef struct {
	VkSurfaceKHR surface;
	VkPhysicalDevice current_physical_device;
	VkDevice logical_device;
	EDynarr *compatible_devices; // contains VkPhysicalDevice
	PVulkanQueueFamilyInfo queue_family_infos[P_VULKAN_QUEUE_TYPE_MAX];
	VkInstance instance; // non-malloced pointer to PVulkanAppData->instance
	VkSwapchainKHR swapchain;
	VkExtent2D swapchain_extent;
	VkFormat swapchain_format;
	EDynarr *swapchain_images; // contains VkImage
	EDynarr *swapchain_image_views; // contains VkImageView

	EDynarr *shaders; // contains VkShaderModule. TODO: move this to renderer
} PVulkanDisplayData;

/**
 * PVulkanAppData
 *
 * This struct is the holds all the information for the app for vulkan to work properly
 */
typedef struct {
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
} PVulkanAppData;

#endif // PLATINUM_BACKEND_VULKAN


// Vulkan specific implementation
typedef VkSurfaceKHR PGraphicalDisplay;
typedef PVulkanAppData PGraphicalAppData;
typedef PVulkanDisplayData PGraphicalDisplayData;
#define p_graphics_init(p_graphical_app_request) \
	p_vulkan_init(p_graphical_app_request)
#define p_graphics_deinit(p_graphical_app_data) \
	p_vulkan_deinit(p_graphical_app_data)
#define p_graphics_display_create(p_window_data, p_graphical_app_data, p_graphical_display_request) \
	p_vulkan_display_create(p_window_data, p_graphical_app_data, p_graphical_display_request)
#define p_graphics_display_destroy(p_graphical_display_data) \
	p_vulkan_display_destroy(p_graphical_display_data)
#define p_graphics_device_set(p_vulkan_display_data, p_vulkan_display_request, p_graphical_device, \
		p_graphical_display, p_window_data) \
	p_vulkan_device_set(p_vulkan_display_data, p_vulkan_display_request, p_graphical_device, p_graphical_display, \
			p_window_data)
#define p_graphics_device_auto_pick(p_vulkan_display_data, p_vulkan_app_data, p_vulkan_display_request, \
		p_graphical_display) \
	p_vulkan_device_auto_pick(p_vulkan_display_data, p_vulkan_app_data, p_vulkan_display_request, p_graphical_display)


#ifdef PLATINUM_BACKEND_VULKAN

PGraphicalAppData *p_vulkan_init(PGraphicalAppRequest *graphical_app_request);

void p_vulkan_deinit(PGraphicalAppData *vulkan_app_data);

void p_vulkan_display_create(PWindowData *window_data, const PGraphicalAppData * const vulkan_app_data,
		const PGraphicalDisplayRequest * const vulkan_display_request);

void p_vulkan_display_destroy(PGraphicalDisplayData *vulkan_display_data);

void p_vulkan_device_set(
		PGraphicalDisplayData *graphical_display_data,
		const PGraphicalDisplayRequest * const graphical_display_request,
		const PGraphicalDevice physical_device,
		const PGraphicalDisplay display,
		const PWindowData * const window_data);

PGraphicalDevice p_vulkan_device_auto_pick(
		PGraphicalDisplayData *graphical_display_data,
		const PGraphicalAppData * const graphical_app_data,
		const PGraphicalDisplayRequest * const graphical_display_request,
		const PGraphicalDisplay display);

#endif // PLATINUM_BACKEND_VULKAN

#endif // _PLATINUM_GRAPHICS_H
