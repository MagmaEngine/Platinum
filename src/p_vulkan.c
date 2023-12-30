#include "phantom.h"
#include <string.h>

#ifdef PHANTOM_DISPLAY_X11
#include <vulkan/vulkan_xcb.h>
#elif defined PHANTOM_DISPLAY_WIN32
#include <vulkan/vulkan_win32.h>
#endif // PHANTOM_DISPLAY_XXXXXX

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
	e_log_message(E_LOG_INFO, L"Vulkan General", L"Number of extensions: %i", vulkan_available_extension_count);

	VkExtensionProperties *vulkan_available_extensions = malloc(vulkan_available_extension_count *
			sizeof(VkExtensionProperties));
	vkEnumerateInstanceExtensionProperties(NULL, &vulkan_available_extension_count, vulkan_available_extensions);
	for (uint32_t i = 0; i < vulkan_available_extension_count; i++)
		e_log_message(E_LOG_DEBUG, L"Vulkan General", L"Supported extension: %s", vulkan_available_extensions[i].extensionName);
	free(vulkan_available_extensions);

}

/**
 * p_vulkan_list_available_devices
 *
 * lists the devices availible with vulkan
 */
void p_vulkan_list_available_devices(VkInstance instance)
{
	uint32_t vulkan_available_device_count = 0;
	vkEnumeratePhysicalDevices(instance, &vulkan_available_device_count, NULL);
	e_log_message(E_LOG_INFO, L"Vulkan General", L"Number of devices: %i", vulkan_available_device_count);
	if (vulkan_available_device_count == 0)
	{
		e_log_message(E_LOG_ERROR, L"Vulkan General", L"Failed to find GPUs with Vulkan support!");
		exit(1);
	}
}

/**
 * p_vulkan_list_available_layers
 *
 * lists the layers available with vulkan
 */
void p_vulkan_list_available_layers(void)
{
	uint32_t vulkan_available_layer_count;
	vkEnumerateInstanceLayerProperties(&vulkan_available_layer_count, NULL);
	e_log_message(E_LOG_INFO, L"Vulkan General", L"Number of layers: %i", vulkan_available_layer_count);

	VkLayerProperties *vulkan_available_layers = malloc(vulkan_available_layer_count *
			sizeof(*vulkan_available_layers));
	vkEnumerateInstanceLayerProperties(&vulkan_available_layer_count, vulkan_available_layers);

	for (uint32_t i = 0; i < vulkan_available_layer_count; i++) {
		e_log_message(E_LOG_DEBUG, L"Vulkan General", L"Supported layers: %s", vulkan_available_layers[i].layerName);
	}
	free(vulkan_available_layers);
}

/**
 * p_vulkan_check_validation_layers
 *
 * should only be called when PHANTOM_DEBUG_VULKAN is set.
 * it checks if the desired validation layers exist
 * if not it returns an error
 */
static void p_vulkan_check_validation_layers(const char *const vulkan_enabled_layers[], uint vulkan_enabled_layer_count)
{
	p_vulkan_list_available_layers();

	uint32_t vulkan_available_layer_count;
	vkEnumerateInstanceLayerProperties(&vulkan_available_layer_count, NULL);
	VkLayerProperties *vulkan_available_layers = malloc(vulkan_available_layer_count *
			sizeof(*vulkan_available_layers));
	vkEnumerateInstanceLayerProperties(&vulkan_available_layer_count, vulkan_available_layers);

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
			e_log_message(E_LOG_ERROR, L"Vulkan General", L"Validation layer %s not found", vulkan_enabled_layers[i]);
			exit(1);
		}
	}
	free(vulkan_available_layers);
}

/**
 * _vulkan_debug_callback
 *
 * the function that overrides vulkan's default callback to integrate
 * with enigma's logging system
 */
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

/**
 * p_vulkan_create_debug_utils_messenger
 *
 * creates the debug messenger EXT object
 */
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

/**
 * p_vulkan_destroy_debug_utils_messenger
 *
 * destroys the debug messenger EXT object
 */
static void p_vulkan_destroy_debug_utils_messenger(
		VkInstance vk_instance,
		VkDebugUtilsMessengerEXT vulkan_debug_messenger,
		const VkAllocationCallbacks* pAllocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != NULL)
		func(vk_instance, vulkan_debug_messenger, pAllocator);
}

/**
 * p_vulkan_init_debug_messenger
 *
 * creates a VkDebugUtilsMessengerCreateInfoEXT and adds it to the vk_instance_create_info
 */
static VkDebugUtilsMessengerCreateInfoEXT p_vulkan_init_debug_messenger(VkInstanceCreateInfo *vk_instance_create_info)
{
	// vulkan layers to use
	// Final entry being NULL enables the sizeof trick to check array size
	const char *const vulkan_enabled_layers[] = {
		"VK_LAYER_KHRONOS_validation",
		NULL};
	uint vulkan_enabled_layer_count = sizeof(vulkan_enabled_layers) / sizeof(vulkan_enabled_layers[0]) - 1;
	p_vulkan_check_validation_layers(vulkan_enabled_layers, vulkan_enabled_layer_count);

	vk_instance_create_info->enabledLayerCount = vulkan_enabled_layer_count;
	vk_instance_create_info->ppEnabledLayerNames = vulkan_enabled_layers;

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
	vk_instance_create_info->pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &vk_debug_utils_messenger_create_info;
	return vk_debug_utils_messenger_create_info;
}

/**
 * p_vulkan_create_instance
 *
 * creates a VkInstance from vk_instance_create_info and adds it to vulkan_data
 */
static void p_vulkan_create_instance(PVulkanData *vulkan_data, const VkInstanceCreateInfo vk_instance_create_info)
{
	VkResult result = vkCreateInstance(&vk_instance_create_info, NULL, &vulkan_data->instance);
	if (result != VK_SUCCESS)
	{
		e_log_message(E_LOG_ERROR, L"Phantom", L"Error initializing Vulkan.");
		exit(1);
	}
}

/**
 * p_vulkan_find_queue_families
 *
 * finds queue families of the current_physical_device and assigns them to vulkan_data
 */
static PVulkanQueueFamilyInfo p_vulkan_find_queue_families(
		const VkPhysicalDevice device,
		const VkQueueFlags queue_flags)
{
	PVulkanQueueFamilyInfo queue_family_info;
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);
	VkQueueFamilyProperties *queue_families = malloc(queue_family_count * sizeof *queue_families);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

	for (uint i = 0; i < queue_family_count; i++)
	{
		if (queue_families[i].queueFlags & queue_flags)
		{
			queue_family_info.exists = true;
			queue_family_info.value = i;
			goto queue_found;
		}
	}
	queue_family_info.exists = false;
queue_found:
	free(queue_families);
	return queue_family_info;
}

/**
 * p_vulkan_pick_physical_device
 *
 * picks the physical device for use with vulkan and adds it to vulkan_data
 */
static void p_vulkan_pick_physical_device(PVulkanData *vulkan_data)
{
	vulkan_data->current_physical_device = VK_NULL_HANDLE;
	uint32_t vulkan_available_device_count = 0;
	vkEnumeratePhysicalDevices(vulkan_data->instance, &vulkan_available_device_count, NULL);

	vulkan_data->compatible_devices = e_dynarr_init(sizeof (VkPhysicalDevice), vulkan_available_device_count);
	EDynarr *compatible_devices = vulkan_data->compatible_devices;
	vkEnumeratePhysicalDevices(vulkan_data->instance, &vulkan_available_device_count, compatible_devices->arr);
	compatible_devices->num_items = vulkan_available_device_count;

	// check device suitability, assign scores
	EDynarr *device_score = e_dynarr_init(sizeof (int), compatible_devices->num_items);
	EDynarr *graphics_queue_family_info = e_dynarr_init(sizeof (PVulkanQueueFamilyInfo), compatible_devices->num_items);
	for (uint i = 0; i < compatible_devices->num_items; i++) {
		int score = 0;
		VkPhysicalDevice device = E_DYNARR_GET(compatible_devices, VkPhysicalDevice, i);
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(device, &device_properties);
		VkPhysicalDeviceFeatures device_features;
		vkGetPhysicalDeviceFeatures(device, &device_features);

		// lots of score goes to discrete gpus
		if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score = 1000;

		// Maximum possible size of textures affects graphics quality
		score += device_properties.limits.maxImageDimension2D;

		PVulkanQueueFamilyInfo current_graphics_queue_family_info = p_vulkan_find_queue_families(device,
				VK_QUEUE_GRAPHICS_BIT);
		e_dynarr_add(graphics_queue_family_info, &current_graphics_queue_family_info);

		// remove unqualified devices from compatible_devices
		if (!device_features.geometryShader || !current_graphics_queue_family_info.exists)
			score = -1;

		e_dynarr_add(device_score, E_VOID_PTR_FROM_VALUE(int, score));
	}

	// remove incompatible gpus and set default gpu
	int max_score = 0;
	int max_score_index = -1;
	for (uint i = 0; i < compatible_devices->num_items;)
	{
		int current_device_score = E_DYNARR_GET(device_score, int, i);
		if (current_device_score > max_score)
		{
			max_score = current_device_score;
			max_score_index = i;
			i++;
		} else if (current_device_score < 0) {
			e_dynarr_remove_ordered(compatible_devices, i);
			e_dynarr_remove_ordered(device_score, i);
			e_dynarr_remove_ordered(graphics_queue_family_info, i);
		} else {
			i++;
		}
	}
	e_dynarr_deinit(device_score);

	if (max_score_index >= 0)
	{
		vulkan_data->current_physical_device = E_DYNARR_GET(compatible_devices, VkPhysicalDevice, max_score_index);
		vulkan_data->graphics_queue_family = E_DYNARR_GET(graphics_queue_family_info, PVulkanQueueFamilyInfo, max_score_index);
	}
	e_dynarr_deinit(graphics_queue_family_info);

	if (vulkan_data->current_physical_device == VK_NULL_HANDLE)
	{
		e_log_message(E_LOG_ERROR, L"Vulkan General", L"Failed to find a suitable GPU!");
		exit(1);
	}
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
		//VK_KHR_SWAPCHAIN_EXTENSION_NAME,

#ifdef PHANTOM_DISPLAY_X11
		VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif // PHANTOM_DISPLAY_X11

#ifdef PHANTOM_DISPLAY_WIN32
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif // PHANTOM_DISPLAY_WIN32

#ifdef PHANTOM_DEBUG_VULKAN
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif // PHANTOM_DEBUG_VULKAN
		NULL};

	vk_instance_create_info.enabledExtensionCount = sizeof(vulkan_enabled_extensions) /
		sizeof(vulkan_enabled_extensions[0]) - 1;
	vk_instance_create_info.ppEnabledExtensionNames = vulkan_enabled_extensions;

#ifdef PHANTOM_DEBUG_VULKAN
	VkDebugUtilsMessengerCreateInfoEXT vk_debug_utils_messenger_create_info =
		p_vulkan_init_debug_messenger(&vk_instance_create_info);
	p_vulkan_create_instance(vulkan_data, vk_instance_create_info);

	if (p_vulkan_create_debug_utils_messenger(vulkan_data->instance, &vk_debug_utils_messenger_create_info, NULL,
				&vulkan_data->debug_messenger) != VK_SUCCESS) {
		e_log_message(E_LOG_ERROR, L"Vulkan General", L"Failed to set up debug messenger!");
		exit(1);
	}
#else
	vk_instance_create_info.enabledLayerCount = 0;
	vk_instance_create_info.pNext = NULL;
	p_vulkan_create_instance(vulkan_data, vk_instance_create_info);
#endif // PHANTOM_DEBUG_VULKAN

	p_vulkan_pick_physical_device(vulkan_data);

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
	e_dynarr_deinit(vulkan_data->compatible_devices);
	vkDestroyInstance(vulkan_data->instance, NULL);
	free(vulkan_data);
}
