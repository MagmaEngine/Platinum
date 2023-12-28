#include "phantom.h"
#include <string.h>
#include <vulkan/vulkan_xcb.h>

/**
 * p_vulkan_check_validation_layers
 *
 * should only be called when PHANTOM_DEBUG_VULKAN is set.
 * it checks if the desired validation layers exist
 * if not it returns an error
 */
static void p_vulkan_check_validation_layers(const char *const vulkan_enabled_layers[], uint vulkan_enabled_layer_count)
{
	uint32_t vulkan_available_layer_count;
	vkEnumerateInstanceLayerProperties(&vulkan_available_layer_count, NULL);
	e_log_message(E_LOG_INFO, L"Vulkan", L"Number of layers: %i", vulkan_available_layer_count);

	VkLayerProperties *vulkan_available_layers = malloc(vulkan_available_layer_count * sizeof(*vulkan_available_layers));
	vkEnumerateInstanceLayerProperties(&vulkan_available_layer_count, vulkan_available_layers);

	for (uint32_t i = 0; i < vulkan_available_layer_count; i++) {
		e_log_message(E_LOG_DEBUG, L"Vulkan", L"Supported layers: %s", vulkan_available_layers[i].layerName);
	}

	// Check if layer exists
	for (uint i = 0; i < vulkan_enabled_layer_count; i++) {
		bool layer_found = false;
		for (uint j = 0; j < vulkan_available_layer_count; j++) {
			if (strcmp(vulkan_enabled_layers[i], vulkan_available_layers[j].layerName) == 0) {
				layer_found = true;
				break;
			}
		}
		if (!layer_found) {
			e_log_message(E_LOG_ERROR, L"Vulkan", L"Validation layer %s not found", vulkan_enabled_layers[i]);
			exit(1);
		}
	}
	free(vulkan_available_layers);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL _vulkan_debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
{
	E_UNUSED(pUserData);

	wchar_t *log_channel;
	switch (messageType)
	{
		case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
			log_channel = L"Vulkan General";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
			log_channel = L"Vulkan Validation";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
			log_channel = L"Vulkan Performance";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
			log_channel = L"Vulkan Device Address Binding";
			break;
	}

	enum ELogLevel log_level;
	switch (messageSeverity)
	{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			log_level = E_LOG_DEBUG;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			log_level = E_LOG_INFO;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			log_level = E_LOG_WARNING;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			log_level = E_LOG_ERROR;
			break;
		// this one should never happen
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
			log_level = E_LOG_MAX;
			break;
	}
	e_log_message(log_level, log_channel, L"%s", pCallbackData->pMessage);
	return VK_FALSE;
}

static VkResult p_vulkan_create_debug_utils_messenger(
		VkInstance vk_instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(vk_instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != NULL)
		return func(vk_instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void p_vulkan_destroy_debug_utils_messenger(VkInstance vk_instance, VkDebugUtilsMessengerEXT vulkan_debug_messenger,
		const VkAllocationCallbacks* pAllocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != NULL)
		func(vk_instance, vulkan_debug_messenger, pAllocator);
}

/**
 * p_vulkan_list_available_extensions
 *
 * lists the extensions available on vulkan
 */
void p_vulkan_list_available_extensions(void)
{
	// Check availible extensions
	uint32_t vulkan_available_extension_count;
	vkEnumerateInstanceExtensionProperties(NULL, &vulkan_available_extension_count, NULL);
	e_log_message(E_LOG_INFO, L"Vulkan", L"Number of extensions: %i", vulkan_available_extension_count);

	VkExtensionProperties *vulkan_available_extensions = malloc(vulkan_available_extension_count * sizeof(VkExtensionProperties));
	vkEnumerateInstanceExtensionProperties(NULL, &vulkan_available_extension_count, vulkan_available_extensions);
	for (uint32_t i = 0; i < vulkan_available_extension_count; i++)
	{
		e_log_message(E_LOG_DEBUG, L"Vulkan", L"Supported extension: %s", vulkan_available_extensions[i].extensionName);
	}
	free(vulkan_available_extensions);

}

/**
 * p_vulkan_init
 *
 * Initializes vulkan
 */
PVulkanData *p_vulkan_init(void)
{
	PVulkanData *vulkan_data = malloc(sizeof *vulkan_data);

#ifdef PHANTOM_DEBUG_VULKAN
	p_vulkan_list_available_extensions();
#endif // PHANTOM_DEBUG_VULKAN

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
	// Final entry being NULL enables the sizeof trick to check array size
	const char *const vulkan_enabled_extensions[] = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		// TODO: make this implementation specific
		VK_KHR_XCB_SURFACE_EXTENSION_NAME,
		//VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef PHANTOM_DEBUG_VULKAN
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif // PHANTOM_DEBUG_VULKAN
		NULL};

	vk_instance_create_info.enabledExtensionCount = sizeof(vulkan_enabled_extensions) /
		sizeof(vulkan_enabled_extensions[0]) - 1;
	vk_instance_create_info.ppEnabledExtensionNames = vulkan_enabled_extensions;

// PHANTOM DEBUG SETUP
#ifdef PHANTOM_DEBUG_VULKAN
	// vulkan layers to use
	// Final entry being NULL enables the sizeof trick to check array size
	const char *const vulkan_enabled_layers[] = {
		"VK_LAYER_KHRONOS_validation",
		NULL};
	uint vulkan_enabled_layer_count = sizeof(vulkan_enabled_layers) / sizeof(vulkan_enabled_layers[0]) - 1;
	p_vulkan_check_validation_layers(vulkan_enabled_layers, vulkan_enabled_layer_count);

	vk_instance_create_info.enabledLayerCount = vulkan_enabled_layer_count;
	vk_instance_create_info.ppEnabledLayerNames = vulkan_enabled_layers;

	// create vulkan debug messenger
	VkDebugUtilsMessengerCreateInfoEXT vk_debug_utils_messenger_create_info;
	vk_debug_utils_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	vk_debug_utils_messenger_create_info.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	vk_debug_utils_messenger_create_info.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	vk_debug_utils_messenger_create_info.pfnUserCallback = _vulkan_debug_callback;
	vk_debug_utils_messenger_create_info.pUserData = NULL;
#else
	vk_instance_create_info.enabledLayerCount = 0;
#endif // PHANTOM_DEBUG_VULKAN

	VkResult result = vkCreateInstance(&vk_instance_create_info, NULL, &vulkan_data->instance);
	if (result != VK_SUCCESS)
	{
		e_log_message(E_LOG_ERROR, L"Phantom", L"Error initializing Vulkan.");
		exit(1);
	}

#ifdef PHANTOM_DEBUG_VULKAN
	if (p_vulkan_create_debug_utils_messenger(vulkan_data->instance, &vk_debug_utils_messenger_create_info, NULL,
				&vulkan_data->debug_messenger) != VK_SUCCESS) {
		e_log_message(E_LOG_ERROR, L"Vulkan", L"Failed to set up debug messenger!");
		exit(1);
	}
#endif // PHANTOM_DEBUG_VULKAN


	return vulkan_data;
}

/**
 * p_vulkan_deinit
 *
 * Deinitializes vulkan
 */
void p_vulkan_deinit(PVulkanData *vulkan_data)
{

#ifdef PHANTOM_DEBUG_VULKAN
	p_vulkan_destroy_debug_utils_messenger(vulkan_data->instance, vulkan_data->debug_messenger, NULL);
#endif // PHANTOM_DEBUG_VULKAN
	vkDestroyInstance(vulkan_data->instance, NULL);
	free(vulkan_data);
}
