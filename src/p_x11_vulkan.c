#include "phantom.h"

/**
 * p_x11_vulkan_init
 *
 * Initializes vulkan on X11 via xcb.
 * returns pointer to a VkInstance.
 */
VkInstance *p_x11_vulkan_init(void)
{
	// create vulkan instance
	VkApplicationInfo vk_app_info;
	vk_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vk_app_info.pApplicationName = "Phantom";
	vk_app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	vk_app_info.pEngineName = "No Engine";
	vk_app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	vk_app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo vk_instance_create_info;
	vk_instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vk_instance_create_info.pApplicationInfo = &vk_app_info;

	// vulkan extensions to use
	const char *const vulkan_enabled_extensions[] = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_XCB_SURFACE_EXTENSION_NAME,
		//VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
	vk_instance_create_info.enabledExtensionCount = sizeof(vulkan_enabled_extensions) / sizeof(vulkan_enabled_extensions[0]);
	vk_instance_create_info.ppEnabledExtensionNames = vulkan_enabled_extensions;

	// vulkan layers to use
	vk_instance_create_info.enabledLayerCount = 0;

	VkInstance *vk_instance = malloc(sizeof *vk_instance);
	VkResult result = vkCreateInstance(&vk_instance_create_info, NULL, vk_instance);
	if (result != VK_SUCCESS)
	{
		e_log_message(E_LOG_ERROR, L"Phantom", L"Error initializing Vulkan.");
		exit(1);
	}

	return vk_instance;
}

/**
 * p_x11_vulkan_deinit
 *
 * Deinitializes vulkan on X11
 */
void p_x11_vulkan_deinit(VkInstance *vk_instance)
{
	free(vk_instance);
}
