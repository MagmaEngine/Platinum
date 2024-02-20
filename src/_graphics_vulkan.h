#ifndef PLATINUM_GRAPHICS_VULKAN_H
#define PLATINUM_GRAPHICS_VULKAN_H

#include <vulkan/vulkan.h>
#include <enigma.h>

#ifdef PLATINUM_DISPLAY_X11
#include <xcb/xcb.h>
#include <vulkan/vulkan_xcb.h>
#elif defined PLATINUM_DISPLAY_WIN32
#include <vulkan/vulkan_win32.h>
#endif // PLATINUM_DISPLAY

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
 * PGraphicalDisplayData
 *
 * This struct contains data relevant to a vulkan display
 */
struct PGraphicalDisplayData{
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
	EDynarr *swapchain_framebuffers; // contains VkFramebuffer
	VkRenderPass render_pass;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;

	EDynarr *shaders; // contains VkShaderModule. TODO: move this to renderer
};

/**
 * PVulkanAppData
 *
 * This struct is the holds all the information for the app for vulkan to work properly
 */
struct PGraphicalAppData {
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
};


#endif // PLATINUM_GRAPHICS_VULKAN_H
