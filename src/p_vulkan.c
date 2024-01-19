#include "phantom.h"
#include <string.h>

#ifdef PHANTOM_DISPLAY_X11
#include <vulkan/vulkan_xcb.h>
#elif defined PHANTOM_DISPLAY_WIN32
#include <vulkan/vulkan_win32.h>
#endif // PHANTOM_DISPLAY_XXXXXX

/**
 * p_vulkan_request_app_create
 *
 * generates a PVulkanAppRequest and returns it as part of request
 */
PHANTOM_API PVulkanAppRequest *p_vulkan_request_app_create(void)
{
	PVulkanAppRequest *vulkan_request_app = malloc(sizeof *vulkan_request_app);
#ifdef PHANTOM_DEBUG_VULKAN
	vulkan_request_app->debug = true;
#else
	vulkan_request_app->debug = false;
#endif // PHANTOM_DEBUG_VULKAN

	// extensions
	vulkan_request_app->optional_extensions = e_dynarr_init(sizeof(char *), 1);

	vulkan_request_app->required_extensions = e_dynarr_init(sizeof(char *), 3);
	e_dynarr_add(vulkan_request_app->required_extensions, E_VOID_PTR_FROM_VALUE(char *,
				VK_KHR_SURFACE_EXTENSION_NAME));
#ifdef PHANTOM_DISPLAY_X11
	e_dynarr_add(vulkan_request_app->required_extensions, E_VOID_PTR_FROM_VALUE(char *,
				VK_KHR_XCB_SURFACE_EXTENSION_NAME));
#endif // PHANTOM_DISPLAY_X11
#ifdef PHANTOM_DISPLAY_WIN32
	e_dynarr_add(vulkan_request_app->required_extensions, E_VOID_PTR_FROM_VALUE(char *,
				VK_KHR_WIN32_SURFACE_EXTENSION_NAME));
#endif // PHANTOM_DISPLAY_WIN32
#ifdef PHANTOM_DEBUG_VULKAN
	e_dynarr_add(vulkan_request_app->required_extensions, E_VOID_PTR_FROM_VALUE(char *,
				VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
#endif // PHANTOM_DEBUG_VULKAN

	// layers
	vulkan_request_app->required_layers = e_dynarr_init(sizeof(char *), 1);
#ifdef PHANTOM_DEBUG_VULKAN
	e_dynarr_add(vulkan_request_app->required_layers, E_VOID_PTR_FROM_VALUE(char *, "VK_LAYER_KHRONOS_validation"));
#endif // PHANTOM_DEBUG_VULKAN
	vulkan_request_app->optional_layers = e_dynarr_init(sizeof(char *), 1);
	return vulkan_request_app;
}

/**
 * p_vulkan_request_display_create
 *
 * generates a PVulkanDisplayRequest and returns it as part of request
 */
PHANTOM_API PVulkanDisplayRequest *p_vulkan_request_display_create(void)
{
	PVulkanDisplayRequest *vulkan_request_display = malloc(sizeof *vulkan_request_display);
	memset(vulkan_request_display, 0, sizeof *vulkan_request_display);
	vulkan_request_display->require_present = true;

	// extensions
	vulkan_request_display->optional_extensions = e_dynarr_init(sizeof(char *), 1);
	vulkan_request_display->required_extensions = e_dynarr_init(sizeof(char *), 3);
	e_dynarr_add(vulkan_request_display->required_extensions, E_VOID_PTR_FROM_VALUE(char *,
				VK_KHR_SWAPCHAIN_EXTENSION_NAME));

	// layers
	vulkan_request_display->required_layers = e_dynarr_init(sizeof(char *), 1);
#ifdef PHANTOM_DEBUG_VULKAN
	e_dynarr_add(vulkan_request_display->required_layers, E_VOID_PTR_FROM_VALUE(char *,
				"VK_LAYER_KHRONOS_validation"));
#endif // PHANTOM_DEBUG_VULKAN
	vulkan_request_display->optional_layers = e_dynarr_init(sizeof(char *), 1);

	vulkan_request_display->required_queue_flags = VK_QUEUE_GRAPHICS_BIT;
	vulkan_request_display->optional_queue_flags = 0ULL;

	vulkan_request_display->required_features.geometryShader = true;
	return vulkan_request_display;
}

/**
 * p_vulkan_request_app_destroy
 *
 * deinits data created as a part of request init
 */
PHANTOM_API void p_vulkan_request_app_destroy(PVulkanAppRequest *vulkan_request_app)
{
	e_dynarr_deinit(vulkan_request_app->required_extensions);
	e_dynarr_deinit(vulkan_request_app->required_layers);
	e_dynarr_deinit(vulkan_request_app->optional_extensions);
	e_dynarr_deinit(vulkan_request_app->optional_layers);
	free(vulkan_request_app);
}

/**
 * p_vulkan_request_display_destroy
 *
 * deinits data created as a part of request init
 */
PHANTOM_API void p_vulkan_request_display_destroy(PVulkanDisplayRequest *vulkan_request_display)
{
	e_dynarr_deinit(vulkan_request_display->required_extensions);
	e_dynarr_deinit(vulkan_request_display->required_layers);
	e_dynarr_deinit(vulkan_request_display->optional_extensions);
	e_dynarr_deinit(vulkan_request_display->optional_layers);
	free(vulkan_request_display);
}

/**
 * _vulkan_extension_exists
 *
 * returns true if the vulkan_extension is found
 */
static bool _vulkan_extension_exists(const char * const vulkan_extension)
{
	// Check availible extensions
	uint32_t vulkan_available_extension_count;
	vkEnumerateInstanceExtensionProperties(NULL, &vulkan_available_extension_count, NULL);
	VkExtensionProperties *vulkan_available_extensions = malloc(vulkan_available_extension_count *
			sizeof(VkExtensionProperties));
	vkEnumerateInstanceExtensionProperties(NULL, &vulkan_available_extension_count, vulkan_available_extensions);
	for (uint32_t i = 0; i < vulkan_available_extension_count; i++)
	{
		if (strcmp(vulkan_extension, vulkan_available_extensions[i].extensionName))
		{
			free(vulkan_available_extensions);
			return true;
		}
	}
	free(vulkan_available_extensions);
	return false;
}

/**
 * _vulkan_layer_exists
 *
 * returns true if the vulkan_layer is found
 */
static bool _vulkan_layer_exists(const char * const vulkan_layer)
{
	// Check availible layers
	uint32_t vulkan_available_layer_count;
	vkEnumerateInstanceLayerProperties(&vulkan_available_layer_count, NULL);
	VkLayerProperties *vulkan_available_layers = malloc(vulkan_available_layer_count * sizeof(VkLayerProperties));
	vkEnumerateInstanceLayerProperties(&vulkan_available_layer_count, vulkan_available_layers);
	for (uint32_t i = 0; i < vulkan_available_layer_count; i++)
	{
		if (strcmp(vulkan_layer, vulkan_available_layers[i].layerName))
		{
			free(vulkan_available_layers);
			return true;
		}
	}
	free(vulkan_available_layers);
	return false;
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
	e_log_message(E_LOG_INFO, L"Vulkan General", L"Number of extensions: %i", vulkan_available_extension_count);

	VkExtensionProperties *vulkan_available_extensions = malloc(vulkan_available_extension_count *
			sizeof(VkExtensionProperties));
	vkEnumerateInstanceExtensionProperties(NULL, &vulkan_available_extension_count, vulkan_available_extensions);
	for (uint32_t i = 0; i < vulkan_available_extension_count; i++)
		e_log_message(E_LOG_DEBUG, L"Vulkan General", L"Supported extension: %s",
				vulkan_available_extensions[i].extensionName);
	free(vulkan_available_extensions);
}

/**
 * p_vulkan_list_available_devices
 *
 * lists the devices availible with vulkan
 */
void p_vulkan_list_available_devices(const VkInstance instance)
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
 * _vulkan_create_debug_utils_messenger
 *
 * creates the debug messenger EXT object
 */
static VkResult _vulkan_create_debug_utils_messenger(
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
 * _vulkan_destroy_debug_utils_messenger
 *
 * destroys the debug messenger EXT object
 */
static void _vulkan_destroy_debug_utils_messenger(
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
 * _vulkan_init_debug_messenger
 *
 * creates a VkDebugUtilsMessengerCreateInfoEXT and adds it to the vk_instance_create_info
 */
static VkDebugUtilsMessengerCreateInfoEXT _vulkan_init_debug_messenger(void)
{
	// create vulkan debug messenger
	VkDebugUtilsMessengerCreateInfoEXT vk_debug_utils_messenger_create_info = {0};
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
	return vk_debug_utils_messenger_create_info;
}

/**
 * _vulkan_find_viable_queue_family_info
 *
 * returns the first viable queue family info, when present_queue is true, it ignores queue_flag
 * and returns a present queue
 */
static PVulkanQueueFamilyInfo _vulkan_find_viable_queue_family_info(const VkPhysicalDevice device,
		const VkSurfaceKHR surface, VkQueueFlagBits queue_flag, bool require_present)
{
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);
	VkQueueFamilyProperties *queue_families = malloc(queue_family_count * sizeof *queue_families);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

	for (uint i = 0; i < queue_family_count; i++)
	{
		VkBool32 present_supported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_supported);
		PVulkanQueueFamilyInfo queue_family_info = { .flags = queue_flag, .exists = true, .index = i };

		if ((require_present && present_supported) || (queue_families[i].queueFlags & queue_flag))
		{
			free(queue_families);
			return queue_family_info;
		}
	}
	free(queue_families);
	return (PVulkanQueueFamilyInfo){0};
}


/**
 * _vulkan_enable_features
 *
 * returns the required and optional features that exist
 * if a required feature does not exist it returns a 0 feature
 */
static VkPhysicalDeviceFeatures _vulkan_enable_features(const PVulkanDisplayRequest * const vulkan_request_display,
		const VkPhysicalDevice device)
{
	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures(device, &device_features);

	VkPhysicalDeviceFeatures enabled_features;

	EDynarr *missing_features = e_dynarr_init(sizeof (const wchar_t *), 1);
	if (vulkan_request_display->required_features.robustBufferAccess && !device_features.robustBufferAccess) e_dynarr_add(missing_features, L"robustBufferAccess");
	if (vulkan_request_display->required_features.fullDrawIndexUint32 && !device_features.fullDrawIndexUint32) e_dynarr_add(missing_features, L"fullDrawIndexUint32");
	if (vulkan_request_display->required_features.imageCubeArray && !device_features.imageCubeArray) e_dynarr_add(missing_features, L"imageCubeArray");
	if (vulkan_request_display->required_features.independentBlend && !device_features.independentBlend) e_dynarr_add(missing_features, L"independentBlend");
	if (vulkan_request_display->required_features.geometryShader && !device_features.geometryShader) e_dynarr_add(missing_features, L"geometryShader");
	if (vulkan_request_display->required_features.tessellationShader && !device_features.tessellationShader) e_dynarr_add(missing_features, L"tessellationShader");
	if (vulkan_request_display->required_features.sampleRateShading && !device_features.sampleRateShading) e_dynarr_add(missing_features, L"sampleRateShading");
	if (vulkan_request_display->required_features.dualSrcBlend && !device_features.dualSrcBlend) e_dynarr_add(missing_features, L"dualSrcBlend");
	if (vulkan_request_display->required_features.logicOp && !device_features.logicOp) e_dynarr_add(missing_features, L"logicOp");
	if (vulkan_request_display->required_features.multiDrawIndirect && !device_features.multiDrawIndirect) e_dynarr_add(missing_features, L"multiDrawIndirect");
	if (vulkan_request_display->required_features.drawIndirectFirstInstance && !device_features.drawIndirectFirstInstance) e_dynarr_add(missing_features, L"drawIndirectFirstInstance");
	if (vulkan_request_display->required_features.depthClamp && !device_features.depthClamp) e_dynarr_add(missing_features, L"depthClamp");
	if (vulkan_request_display->required_features.depthBiasClamp && !device_features.depthBiasClamp) e_dynarr_add(missing_features, L"depthBiasClamp");
	if (vulkan_request_display->required_features.fillModeNonSolid && !device_features.fillModeNonSolid) e_dynarr_add(missing_features, L"fillModeNonSolid");
	if (vulkan_request_display->required_features.depthBounds && !device_features.depthBounds) e_dynarr_add(missing_features, L"depthBounds");
	if (vulkan_request_display->required_features.wideLines && !device_features.wideLines) e_dynarr_add(missing_features, L"wideLines");
	if (vulkan_request_display->required_features.largePoints && !device_features.largePoints) e_dynarr_add(missing_features, L"largePoints");
	if (vulkan_request_display->required_features.alphaToOne && !device_features.alphaToOne) e_dynarr_add(missing_features, L"alphaToOne");
	if (vulkan_request_display->required_features.multiViewport && !device_features.multiViewport) e_dynarr_add(missing_features, L"multiViewport");
	if (vulkan_request_display->required_features.samplerAnisotropy && !device_features.samplerAnisotropy) e_dynarr_add(missing_features, L"samplerAnisotropy");
	if (vulkan_request_display->required_features.textureCompressionETC2 && !device_features.textureCompressionETC2) e_dynarr_add(missing_features, L"textureCompressionETC2");
	if (vulkan_request_display->required_features.textureCompressionASTC_LDR && !device_features.textureCompressionASTC_LDR) e_dynarr_add(missing_features, L"textureCompressionASTC_LDR");
	if (vulkan_request_display->required_features.textureCompressionBC && !device_features.textureCompressionBC) e_dynarr_add(missing_features, L"textureCompressionBC");
	if (vulkan_request_display->required_features.occlusionQueryPrecise && !device_features.occlusionQueryPrecise) e_dynarr_add(missing_features, L"occlusionQueryPrecise");
	if (vulkan_request_display->required_features.pipelineStatisticsQuery && !device_features.pipelineStatisticsQuery) e_dynarr_add(missing_features, L"pipelineStatisticsQuery");
	if (vulkan_request_display->required_features.vertexPipelineStoresAndAtomics && !device_features.vertexPipelineStoresAndAtomics) e_dynarr_add(missing_features, L"vertexPipelineStoresAndAtomics");
	if (vulkan_request_display->required_features.fragmentStoresAndAtomics && !device_features.fragmentStoresAndAtomics) e_dynarr_add(missing_features, L"fragmentStoresAndAtomics");
	if (vulkan_request_display->required_features.shaderTessellationAndGeometryPointSize && !device_features.shaderTessellationAndGeometryPointSize) e_dynarr_add(missing_features, L"shaderTessellationAndGeometryPointSize");
	if (vulkan_request_display->required_features.shaderImageGatherExtended && !device_features.shaderImageGatherExtended) e_dynarr_add(missing_features, L"shaderImageGatherExtended");
	if (vulkan_request_display->required_features.shaderStorageImageExtendedFormats && !device_features.shaderStorageImageExtendedFormats) e_dynarr_add(missing_features, L"shaderStorageImageExtendedFormats");
	if (vulkan_request_display->required_features.shaderStorageImageMultisample && !device_features.shaderStorageImageMultisample) e_dynarr_add(missing_features, L"shaderStorageImageMultisample");
	if (vulkan_request_display->required_features.shaderStorageImageReadWithoutFormat && !device_features.shaderStorageImageReadWithoutFormat) e_dynarr_add(missing_features, L"shaderStorageImageReadWithoutFormat");
	if (vulkan_request_display->required_features.shaderStorageImageWriteWithoutFormat && !device_features.shaderStorageImageWriteWithoutFormat) e_dynarr_add(missing_features, L"shaderStorageImageWriteWithoutFormat");
	if (vulkan_request_display->required_features.shaderUniformBufferArrayDynamicIndexing && !device_features.shaderUniformBufferArrayDynamicIndexing) e_dynarr_add(missing_features, L"shaderUniformBufferArrayDynamicIndexing");
	if (vulkan_request_display->required_features.shaderSampledImageArrayDynamicIndexing && !device_features.shaderSampledImageArrayDynamicIndexing) e_dynarr_add(missing_features, L"shaderSampledImageArrayDynamicIndexing");
	if (vulkan_request_display->required_features.shaderStorageBufferArrayDynamicIndexing && !device_features.shaderStorageBufferArrayDynamicIndexing) e_dynarr_add(missing_features, L"shaderStorageBufferArrayDynamicIndexing");
	if (vulkan_request_display->required_features.shaderStorageImageArrayDynamicIndexing && !device_features.shaderStorageImageArrayDynamicIndexing) e_dynarr_add(missing_features, L"shaderStorageImageArrayDynamicIndexing");
	if (vulkan_request_display->required_features.shaderClipDistance && !device_features.shaderClipDistance) e_dynarr_add(missing_features, L"shaderClipDistance");
	if (vulkan_request_display->required_features.shaderCullDistance && !device_features.shaderCullDistance) e_dynarr_add(missing_features, L"shaderCullDistance");
	if (vulkan_request_display->required_features.shaderFloat64 && !device_features.shaderFloat64) e_dynarr_add(missing_features, L"shaderFloat64");
	if (vulkan_request_display->required_features.shaderInt64 && !device_features.shaderInt64) e_dynarr_add(missing_features, L"shaderInt64");
	if (vulkan_request_display->required_features.shaderInt16 && !device_features.shaderInt16) e_dynarr_add(missing_features, L"shaderInt16");
	if (vulkan_request_display->required_features.shaderResourceResidency && !device_features.shaderResourceResidency) e_dynarr_add(missing_features, L"shaderResourceResidency");
	if (vulkan_request_display->required_features.shaderResourceMinLod && !device_features.shaderResourceMinLod) e_dynarr_add(missing_features, L"shaderResourceMinLod");
	if (vulkan_request_display->required_features.sparseBinding && !device_features.sparseBinding) e_dynarr_add(missing_features, L"sparseBinding");
	if (vulkan_request_display->required_features.sparseResidencyBuffer && !device_features.sparseResidencyBuffer) e_dynarr_add(missing_features, L"sparseResidencyBuffer");
	if (vulkan_request_display->required_features.sparseResidencyImage2D && !device_features.sparseResidencyImage2D) e_dynarr_add(missing_features, L"sparseResidencyImage2D");
	if (vulkan_request_display->required_features.sparseResidencyImage3D && !device_features.sparseResidencyImage3D) e_dynarr_add(missing_features, L"sparseResidencyImage3D");
	if (vulkan_request_display->required_features.sparseResidency2Samples && !device_features.sparseResidency2Samples) e_dynarr_add(missing_features, L"sparseResidency2Samples");
	if (vulkan_request_display->required_features.sparseResidency4Samples && !device_features.sparseResidency4Samples) e_dynarr_add(missing_features, L"sparseResidency4Samples");
	if (vulkan_request_display->required_features.sparseResidency8Samples && !device_features.sparseResidency8Samples) e_dynarr_add(missing_features, L"sparseResidency8Samples");
	if (vulkan_request_display->required_features.sparseResidency16Samples && !device_features.sparseResidency16Samples) e_dynarr_add(missing_features, L"sparseResidency16Samples");
	if (vulkan_request_display->required_features.sparseResidencyAliased && !device_features.sparseResidencyAliased) e_dynarr_add(missing_features, L"sparseResidencyAliased");
	if (vulkan_request_display->required_features.variableMultisampleRate && !device_features.variableMultisampleRate) e_dynarr_add(missing_features, L"variableMultisampleRate");
	if (vulkan_request_display->required_features.inheritedQueries && !device_features.inheritedQueries) e_dynarr_add(missing_features, L"inheritedQueries");
	if (missing_features->num_items > 0)
	{
		e_log_message(E_LOG_ERROR, L"Phantom", L"Vulkan required features not supported");
		for (uint i = 0; i < missing_features->num_items; i++)
			e_log_message(E_LOG_ERROR, L"Phantom", L"\t%ls", E_DYNARR_GET(missing_features, wchar_t *, i));
		exit(1);
	} else {
		enabled_features = vulkan_request_display->required_features;
	}
	e_dynarr_deinit(missing_features);

	// optional features
	if (vulkan_request_display->optional_features.robustBufferAccess) enabled_features.robustBufferAccess = true;
	if (vulkan_request_display->optional_features.fullDrawIndexUint32) enabled_features.fullDrawIndexUint32 = true;
	if (vulkan_request_display->optional_features.imageCubeArray) enabled_features.imageCubeArray = true;
	if (vulkan_request_display->optional_features.independentBlend) enabled_features.independentBlend = true;
	if (vulkan_request_display->optional_features.geometryShader) enabled_features.geometryShader = true;
	if (vulkan_request_display->optional_features.tessellationShader) enabled_features.tessellationShader = true;
	if (vulkan_request_display->optional_features.sampleRateShading) enabled_features.sampleRateShading = true;
	if (vulkan_request_display->optional_features.dualSrcBlend) enabled_features.dualSrcBlend = true;
	if (vulkan_request_display->optional_features.logicOp) enabled_features.logicOp = true;
	if (vulkan_request_display->optional_features.multiDrawIndirect) enabled_features.multiDrawIndirect = true;
	if (vulkan_request_display->optional_features.drawIndirectFirstInstance) enabled_features.drawIndirectFirstInstance = true;
	if (vulkan_request_display->optional_features.depthClamp) enabled_features.depthClamp = true;
	if (vulkan_request_display->optional_features.depthBiasClamp) enabled_features.depthBiasClamp = true;
	if (vulkan_request_display->optional_features.fillModeNonSolid) enabled_features.fillModeNonSolid = true;
	if (vulkan_request_display->optional_features.depthBounds) enabled_features.depthBounds = true;
	if (vulkan_request_display->optional_features.wideLines) enabled_features.wideLines = true;
	if (vulkan_request_display->optional_features.largePoints) enabled_features.largePoints = true;
	if (vulkan_request_display->optional_features.alphaToOne) enabled_features.alphaToOne = true;
	if (vulkan_request_display->optional_features.multiViewport) enabled_features.multiViewport = true;
	if (vulkan_request_display->optional_features.samplerAnisotropy) enabled_features.samplerAnisotropy = true;
	if (vulkan_request_display->optional_features.textureCompressionETC2) enabled_features.textureCompressionETC2 = true;
	if (vulkan_request_display->optional_features.textureCompressionASTC_LDR) enabled_features.textureCompressionASTC_LDR = true;
	if (vulkan_request_display->optional_features.textureCompressionBC) enabled_features.textureCompressionBC = true;
	if (vulkan_request_display->optional_features.occlusionQueryPrecise) enabled_features.occlusionQueryPrecise = true;
	if (vulkan_request_display->optional_features.pipelineStatisticsQuery) enabled_features.pipelineStatisticsQuery = true;
	if (vulkan_request_display->optional_features.vertexPipelineStoresAndAtomics) enabled_features.vertexPipelineStoresAndAtomics = true;
	if (vulkan_request_display->optional_features.fragmentStoresAndAtomics) enabled_features.fragmentStoresAndAtomics = true;
	if (vulkan_request_display->optional_features.shaderTessellationAndGeometryPointSize) enabled_features.shaderTessellationAndGeometryPointSize = true;
	if (vulkan_request_display->optional_features.shaderImageGatherExtended) enabled_features.shaderImageGatherExtended = true;
	if (vulkan_request_display->optional_features.shaderStorageImageExtendedFormats) enabled_features.shaderStorageImageExtendedFormats = true;
	if (vulkan_request_display->optional_features.shaderStorageImageMultisample) enabled_features.shaderStorageImageMultisample = true;
	if (vulkan_request_display->optional_features.shaderStorageImageReadWithoutFormat) enabled_features.shaderStorageImageReadWithoutFormat = true;
	if (vulkan_request_display->optional_features.shaderStorageImageWriteWithoutFormat) enabled_features.shaderStorageImageWriteWithoutFormat = true;
	if (vulkan_request_display->optional_features.shaderUniformBufferArrayDynamicIndexing) enabled_features.shaderUniformBufferArrayDynamicIndexing = true;
	if (vulkan_request_display->optional_features.shaderSampledImageArrayDynamicIndexing) enabled_features.shaderSampledImageArrayDynamicIndexing = true;
	if (vulkan_request_display->optional_features.shaderStorageBufferArrayDynamicIndexing) enabled_features.shaderStorageBufferArrayDynamicIndexing = true;
	if (vulkan_request_display->optional_features.shaderStorageImageArrayDynamicIndexing) enabled_features.shaderStorageImageArrayDynamicIndexing = true;
	if (vulkan_request_display->optional_features.shaderClipDistance) enabled_features.shaderClipDistance = true;
	if (vulkan_request_display->optional_features.shaderCullDistance) enabled_features.shaderCullDistance = true;
	if (vulkan_request_display->optional_features.shaderFloat64) enabled_features.shaderFloat64 = true;
	if (vulkan_request_display->optional_features.shaderInt64) enabled_features.shaderInt64 = true;
	if (vulkan_request_display->optional_features.shaderInt16) enabled_features.shaderInt16 = true;
	if (vulkan_request_display->optional_features.shaderResourceResidency) enabled_features.shaderResourceResidency = true;
	if (vulkan_request_display->optional_features.shaderResourceMinLod) enabled_features.shaderResourceMinLod = true;
	if (vulkan_request_display->optional_features.sparseBinding) enabled_features.sparseBinding = true;
	if (vulkan_request_display->optional_features.sparseResidencyBuffer) enabled_features.sparseResidencyBuffer = true;
	if (vulkan_request_display->optional_features.sparseResidencyImage2D) enabled_features.sparseResidencyImage2D = true;
	if (vulkan_request_display->optional_features.sparseResidencyImage3D) enabled_features.sparseResidencyImage3D = true;
	if (vulkan_request_display->optional_features.sparseResidency2Samples) enabled_features.sparseResidency2Samples = true;
	if (vulkan_request_display->optional_features.sparseResidency4Samples) enabled_features.sparseResidency4Samples = true;
	if (vulkan_request_display->optional_features.sparseResidency8Samples) enabled_features.sparseResidency8Samples = true;
	if (vulkan_request_display->optional_features.sparseResidency16Samples) enabled_features.sparseResidency16Samples = true;
	if (vulkan_request_display->optional_features.sparseResidencyAliased) enabled_features.sparseResidencyAliased = true;
	if (vulkan_request_display->optional_features.variableMultisampleRate) enabled_features.variableMultisampleRate = true;
	if (vulkan_request_display->optional_features.inheritedQueries) enabled_features.inheritedQueries = true;

	return enabled_features;
}

/**
 * _vulkan_enable_queue_flags
 *
 * returns the required and optional queue flags that exist
 * if a required queue flag does not exist it returns 0
 */
static VkQueueFlags _vulkan_enable_queue_flags(const PVulkanDisplayRequest * const vulkan_request_display,
		const VkPhysicalDevice device,
		const VkSurfaceKHR surface)
{
	if (vulkan_request_display->require_present)
	{
		PVulkanQueueFamilyInfo queue_family_info = _vulkan_find_viable_queue_family_info(
				device, surface, 0, true);
		if (!queue_family_info.exists)
		{
			e_log_message(E_LOG_ERROR, L"Vulkan General", L"Presentation not supported.");
			exit(1);
		}
	}

	VkQueueFlags queue_flags = 0ULL;
	// evaluate optional and required queue flags
	for (VkQueueFlags flag = 1ULL; flag < VK_QUEUE_FLAG_BITS_MAX_ENUM; flag <<= 1)
	{
		PVulkanQueueFamilyInfo queue_family_info = _vulkan_find_viable_queue_family_info(device, surface, flag, false);
		if ((flag & vulkan_request_display->optional_queue_flags) && queue_family_info.exists)
		{
			queue_flags |= flag;
			break;
		}
		if ((flag & vulkan_request_display->required_queue_flags) && !queue_family_info.exists)
		{
			e_log_message(E_LOG_ERROR, L"Vulkan General", L"Required Vulkan queue type \"%i\" does not exist!", flag);
			exit(1);
		}
	}
	queue_flags |= vulkan_request_display->required_queue_flags;
	return queue_flags;
}

/**
 * _vulkan_enable_extensions
 *
 * returns the required and optional extensions that exist
 * if a required extension does not exist it returns 0
 */
static EDynarr *_vulkan_enable_extensions(const EDynarr * const required_extensions,
		const EDynarr * const optional_extensions)
{
	EDynarr *enabled_extensions = e_dynarr_init(sizeof (char *), required_extensions->num_items);
	for (uint i = 0; i < required_extensions->num_items; i++)
	{
		char *extension = E_DYNARR_GET(required_extensions, char *, i);
		if(_vulkan_extension_exists(extension))
			e_dynarr_add(enabled_extensions, E_VOID_PTR_FROM_VALUE(char *, extension));
		else
		{
			e_log_message(E_LOG_ERROR, L"Vulkan General", L"Required extension \"%s\" does not exist!", extension);
			exit(1);
		}
	}
	for (uint i = 0; i < optional_extensions->num_items; i++)
	{
		char *extension = E_DYNARR_GET(optional_extensions, char *, i);
		if(_vulkan_extension_exists(extension))
			e_dynarr_add(enabled_extensions, E_VOID_PTR_FROM_VALUE(char *, extension));
	}
	return enabled_extensions;
}

/**
 * _vulkan_enable_layers
 *
 * returns the required and optional layers that exist
 * if a required layer does not exist it returns 0
 */
EDynarr *_vulkan_enable_layers(const EDynarr * const required_layers, const EDynarr * const optional_layers)
{
	EDynarr *enabled_layers = e_dynarr_init(sizeof (char *), required_layers->num_items);
	for (uint i = 0; i < required_layers->num_items; i++)
	{
		char *layer = E_DYNARR_GET(required_layers, char *, i);
		if(_vulkan_layer_exists(layer))
			e_dynarr_add(enabled_layers, E_VOID_PTR_FROM_VALUE(char *,layer));
		else
		{
			e_log_message(E_LOG_ERROR, L"Vulkan General", L"Required layer \"%s\" does not exist!", layer);
			exit(1);
		}
	}
	for (uint i = 0; i < optional_layers->num_items; i++)
	{
		char *layer = E_DYNARR_GET(optional_layers, char *, i);
		if(_vulkan_layer_exists(layer))
			e_dynarr_add(enabled_layers, E_VOID_PTR_FROM_VALUE(char *,layer));
	}
	return enabled_layers;
}

/**
 * _vulkan_swapchain_create
 *
 * helper function that creates a swapchain for the given window
 */
static void _vulkan_swapchain_create(
		PVulkanDataDisplay * const vulkan_data_display,
		const uint32_t framebuffer_width,
		const uint32_t framebuffer_height,
		const PVulkanSwapchainSupport swapchain_support)
{

	// Set extent
	vulkan_data_display->swapchain_extent.width = e_mini(
			e_maxi(framebuffer_width, swapchain_support.capabilities.minImageExtent.width),
			swapchain_support.capabilities.maxImageExtent.width);

	vulkan_data_display->swapchain_extent.height = e_mini(
			e_maxi(framebuffer_height, swapchain_support.capabilities.minImageExtent.height),
			swapchain_support.capabilities.maxImageExtent.height);

	// Save format for later
	vulkan_data_display->swapchain_format = swapchain_support.format->format;

	// Set image count
	uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
	if (swapchain_support.capabilities.maxImageCount > 0 && image_count > swapchain_support.capabilities.maxImageCount)
		image_count = swapchain_support.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR swapchain_create_info = {0};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface = vulkan_data_display->surface;
	swapchain_create_info.minImageCount = image_count;
	swapchain_create_info.imageFormat = swapchain_support.format->format;
	swapchain_create_info.imageColorSpace = swapchain_support.format->colorSpace;
	swapchain_create_info.imageExtent = vulkan_data_display->swapchain_extent;
	// TODO: add options to change this
	swapchain_create_info.preTransform = swapchain_support.capabilities.currentTransform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = *swapchain_support.present_mode;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.oldSwapchain = VK_NULL_HANDLE; // TODO: change this when we recreate the swapchain
	swapchain_create_info.imageArrayLayers = 1; // always 1 unless you use stereoscopic 3D

	// TODO: change this if/when we add post processing
	// right now we are rendering directly to the swapchain
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Set queue families
	if (vulkan_data_display->queue_family_infos[P_VULKAN_QUEUE_TYPE_GRAPHICS].index !=
			vulkan_data_display->queue_family_infos[P_VULKAN_QUEUE_TYPE_PRESENT].index)
	{
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_create_info.queueFamilyIndexCount = 2;
		swapchain_create_info.pQueueFamilyIndices = (uint32_t[]) {
			vulkan_data_display->queue_family_infos[P_VULKAN_QUEUE_TYPE_GRAPHICS].index,
			vulkan_data_display->queue_family_infos[P_VULKAN_QUEUE_TYPE_PRESENT].index
		};
	} else {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount = 0; // Optional
		swapchain_create_info.pQueueFamilyIndices = NULL; // Optional
	}

	if (vulkan_data_display->swapchain != VK_NULL_HANDLE)
		vkDestroySwapchainKHR(vulkan_data_display->logical_device, vulkan_data_display->swapchain, NULL);
	if (vkCreateSwapchainKHR(vulkan_data_display->logical_device, &swapchain_create_info, NULL,
			&vulkan_data_display->swapchain) != VK_SUCCESS)
	{
		e_log_message(E_LOG_ERROR, L"Vulkan General", L"Failed to create swapchain!");
		exit(1);
	}
	vkGetSwapchainImagesKHR(vulkan_data_display->logical_device, vulkan_data_display->swapchain, &image_count, NULL);
	vulkan_data_display->swapchain_images = e_dynarr_init(sizeof (VkImage), image_count);
	vulkan_data_display->swapchain_images->num_items = image_count;
	vkGetSwapchainImagesKHR(vulkan_data_display->logical_device, vulkan_data_display->swapchain, &image_count,
			vulkan_data_display->swapchain_images->arr);
}

/**
 * _vulkan_swapchain_auto_pick
 *
 * helper function that returns the swapchain details closest to the desired settings
 * if none exist returns NULL in appropriate fields. Must be freed
 */
static PVulkanSwapchainSupport _vulkan_swapchain_auto_pick(const VkPhysicalDevice physical_device,
		const VkSurfaceKHR surface)
{
	PVulkanSwapchainSupport swapchain_support = {0};

	// Set swapchain details
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &swapchain_support.capabilities);

	// Set surface format
	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, NULL);
	if (format_count != 0)
	{
		EDynarr *formats = e_dynarr_init(sizeof (VkSurfaceFormatKHR), format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats->arr);
		formats->num_items = format_count;


		VkSurfaceFormatKHR format = E_DYNARR_GET(formats, VkSurfaceFormatKHR, 0);
		for (uint i = 0; i < format_count; i++)
		{
			VkSurfaceFormatKHR current_format = E_DYNARR_GET(formats, VkSurfaceFormatKHR, i);
			if (current_format.format == VK_FORMAT_B8G8R8A8_SRGB)
			{
				format = current_format;
				break;
			}
		}
		swapchain_support.format = malloc(sizeof (VkSurfaceFormatKHR));
		*swapchain_support.format = format;
		e_dynarr_deinit(formats);
	}

	// Set present mode
	uint32_t present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL);
	if (present_mode_count != 0)
	{
		EDynarr *present_modes = e_dynarr_init(sizeof (VkPresentModeKHR), present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes->arr);
		present_modes->num_items = present_mode_count;

		VkPresentModeKHR present_mode = E_DYNARR_GET(present_modes, VkPresentModeKHR, 0);
		for (uint i = 0; i < present_mode_count; i++)
		{
			VkPresentModeKHR current_present_mode = E_DYNARR_GET(present_modes, VkPresentModeKHR, i);
			if (current_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				present_mode = current_present_mode;
				break;
			}
		}
		swapchain_support.present_mode = malloc(sizeof (VkPresentModeKHR));
		*swapchain_support.present_mode = present_mode;
		e_dynarr_deinit(present_modes);
	}
	return swapchain_support;
}

/**
 * p_vulkan_device_set
 *
 * sets the physical device to use in vulkan_data_display from physical_device
 * also creates a logical device with vulkan_request_app and sets it to vulkan_data_display
 */
PHANTOM_API void p_vulkan_device_set(
		PVulkanDataDisplay *vulkan_data_display,
		const PVulkanDisplayRequest * const vulkan_request_display,
		const VkPhysicalDevice physical_device,
		const VkSurfaceKHR surface,
		const PWindowData * const window_data)
{
	if (e_dynarr_find(vulkan_data_display->compatible_devices, &physical_device) == -1)
	{
		e_log_message(E_LOG_ERROR, L"Vulkan General", L"Selected device is not compatible");
		exit(1);
	}

	// Set physical device
	vulkan_data_display->current_physical_device = physical_device;

	// Set queue family indices
	VkQueueFlags enabled_queue_flags = _vulkan_enable_queue_flags(vulkan_request_display, physical_device, surface);
	if (enabled_queue_flags & VK_QUEUE_GRAPHICS_BIT)
		vulkan_data_display->queue_family_infos[P_VULKAN_QUEUE_TYPE_GRAPHICS] =
			_vulkan_find_viable_queue_family_info(physical_device, surface, VK_QUEUE_GRAPHICS_BIT, false);
	if (vulkan_request_display->require_present)
		vulkan_data_display->queue_family_infos[P_VULKAN_QUEUE_TYPE_PRESENT] =
			_vulkan_find_viable_queue_family_info(physical_device, surface, VK_QUEUE_GRAPHICS_BIT, true);
	if (enabled_queue_flags & VK_QUEUE_COMPUTE_BIT)
		vulkan_data_display->queue_family_infos[P_VULKAN_QUEUE_TYPE_COMPUTE] =
			_vulkan_find_viable_queue_family_info(physical_device, surface, VK_QUEUE_COMPUTE_BIT, false);
	if (enabled_queue_flags & VK_QUEUE_TRANSFER_BIT)
		vulkan_data_display->queue_family_infos[P_VULKAN_QUEUE_TYPE_TRANSFER] =
			_vulkan_find_viable_queue_family_info(physical_device, surface, VK_QUEUE_TRANSFER_BIT, false);
	if (enabled_queue_flags & VK_QUEUE_SPARSE_BINDING_BIT)
		vulkan_data_display->queue_family_infos[P_VULKAN_QUEUE_TYPE_SPARSE] =
			_vulkan_find_viable_queue_family_info(physical_device, surface, VK_QUEUE_SPARSE_BINDING_BIT, false);
	if (enabled_queue_flags & VK_QUEUE_PROTECTED_BIT)
		vulkan_data_display->queue_family_infos[P_VULKAN_QUEUE_TYPE_PROTECTED] =
			_vulkan_find_viable_queue_family_info(physical_device, surface, VK_QUEUE_PROTECTED_BIT, false);
	if (enabled_queue_flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
		vulkan_data_display->queue_family_infos[P_VULKAN_QUEUE_TYPE_VIDEO_DECODE] =
			_vulkan_find_viable_queue_family_info(physical_device, surface, VK_QUEUE_VIDEO_DECODE_BIT_KHR, false);
	if (enabled_queue_flags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
		vulkan_data_display->queue_family_infos[P_VULKAN_QUEUE_TYPE_OPTICAL_FLOW] =
			_vulkan_find_viable_queue_family_info(physical_device, surface, VK_QUEUE_OPTICAL_FLOW_BIT_NV, false);

	EDynarr *queue_family_infos = e_dynarr_init(sizeof (PVulkanQueueFamilyInfo), 1);
	for (uint i = 0; i < P_VULKAN_QUEUE_TYPE_MAX; i++)
	{
		bool unique = true;
		for (uint j = 0; j < queue_family_infos->num_items; j++)
		{
			if ((E_DYNARR_GET(queue_family_infos, PVulkanQueueFamilyInfo, j).exists) &&
				vulkan_data_display->queue_family_infos[i].index ==
				E_DYNARR_GET(queue_family_infos, PVulkanQueueFamilyInfo, j).index)
			{
				unique = false;
				break;
			}
		}
		if (unique)
			e_dynarr_add(queue_family_infos, &vulkan_data_display->queue_family_infos[i]);
	}

	uint num_queue_families = queue_family_infos->num_items;
	VkDeviceQueueCreateInfo queue_create_infos[num_queue_families];
	memset(queue_create_infos, 0, sizeof queue_create_infos);
	for (uint i = 0; i < queue_family_infos->num_items; i++)
	{
		queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[i].queueFamilyIndex = E_DYNARR_GET(queue_family_infos, PVulkanQueueFamilyInfo, i).index;
		queue_create_infos[i].queueCount = 1;
		queue_create_infos[i].pQueuePriorities = E_PTR_FROM_VALUE(float, 1.f);
	}

	// Set device features
	VkPhysicalDeviceFeatures device_features  = _vulkan_enable_features(vulkan_request_display, physical_device);

	// Create logical device
	VkDeviceCreateInfo device_create_info = {0};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pQueueCreateInfos = queue_create_infos;
	device_create_info.queueCreateInfoCount = num_queue_families;
	device_create_info.pEnabledFeatures = &device_features;

	EDynarr *enabled_extensions = _vulkan_enable_extensions(vulkan_request_display->required_extensions,
			vulkan_request_display->optional_extensions);
	device_create_info.enabledExtensionCount = enabled_extensions->num_items;
	device_create_info.ppEnabledExtensionNames = enabled_extensions->arr;

	EDynarr *enabled_layers = _vulkan_enable_layers(vulkan_request_display->required_layers,
			vulkan_request_display->optional_layers);
	device_create_info.enabledLayerCount = enabled_layers->num_items;
	device_create_info.ppEnabledLayerNames = enabled_layers->arr;

	// Set logical device queues
	EDynarr *device_queue_create_infos = e_dynarr_init(sizeof (VkDeviceQueueCreateInfo), 1);
	for (uint i = 0; i < queue_family_infos->num_items; i++)
	{
		VkDeviceQueueCreateInfo queue_create_info = {0};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = E_DYNARR_GET(queue_family_infos, PVulkanQueueFamilyInfo, i).index;
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = E_PTR_FROM_VALUE(float, 1.f);
		e_dynarr_add(device_queue_create_infos, &queue_create_info);
	}
	device_create_info.queueCreateInfoCount = device_queue_create_infos->num_items;
	device_create_info.pQueueCreateInfos = device_queue_create_infos->arr;

	// create logical device
	if (vulkan_data_display->logical_device != VK_NULL_HANDLE)
		vkDestroyDevice(vulkan_data_display->logical_device, NULL);
	if (vkCreateDevice(physical_device, &device_create_info, NULL, &vulkan_data_display->logical_device) != VK_SUCCESS)
	{
		e_log_message(E_LOG_ERROR, L"Vulkan General", L"Failed to create logical device!");
		exit(1);
	}
	e_dynarr_deinit(enabled_extensions);
	e_dynarr_deinit(enabled_layers);
	e_dynarr_deinit(device_queue_create_infos);
	e_dynarr_deinit(queue_family_infos);

	// Set queue handles
	for (uint i = 0; i < P_VULKAN_QUEUE_TYPE_MAX; i++)
	{
		if (!vulkan_data_display->queue_family_infos[i].exists)
			continue;
		VkQueue vk_queue = {0};
		vkGetDeviceQueue(vulkan_data_display->logical_device, vulkan_data_display->queue_family_infos[i].index, 0, &vk_queue);
		vulkan_data_display->queue_family_infos[i].queue = vk_queue;
	}

	// Set swapchain data
	PVulkanSwapchainSupport swapchain_support = _vulkan_swapchain_auto_pick(physical_device, surface);
	_vulkan_swapchain_create(vulkan_data_display, window_data->x, window_data->y, swapchain_support);
	free(swapchain_support.format);
	free(swapchain_support.present_mode);
}

/**
 * p_vulkan_device_auto_pick
 *
 * initializes compatible_devices in vulkan_data_display
 * picks the physical device for use with vulkan and returns it
 * returns VK_NULL_HANDLE if no suitable device is found
 */
PHANTOM_API VkPhysicalDevice p_vulkan_device_auto_pick(
		PVulkanDataDisplay *vulkan_data_display,
		const PVulkanDataApp * const vulkan_data_app,
		const PVulkanDisplayRequest * const vulkan_request_display,
		const VkSurfaceKHR surface)
{
	vulkan_data_display->current_physical_device = VK_NULL_HANDLE;

	uint32_t vulkan_available_device_count = 0;
	vkEnumeratePhysicalDevices(vulkan_data_app->instance, &vulkan_available_device_count, NULL);

	if (vulkan_data_display->compatible_devices != NULL)
		e_dynarr_deinit(vulkan_data_display->compatible_devices);
	vulkan_data_display->compatible_devices = e_dynarr_init(sizeof (VkPhysicalDevice), vulkan_available_device_count);
	EDynarr *compatible_devices = vulkan_data_display->compatible_devices;
	vkEnumeratePhysicalDevices(vulkan_data_app->instance, &vulkan_available_device_count, compatible_devices->arr);
	compatible_devices->num_items = vulkan_available_device_count;

	// check device suitability, assign scores
	EDynarr *device_score = e_dynarr_init(sizeof (int), compatible_devices->num_items);
	for (uint i = 0; i < compatible_devices->num_items; i++) {
		int score = 0;
		VkPhysicalDevice device = E_DYNARR_GET(compatible_devices, VkPhysicalDevice, i);
		VkPhysicalDeviceProperties device_properties = {0};
		vkGetPhysicalDeviceProperties(device, &device_properties);

		// lots of score goes to discrete gpus
		if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score = 10000;

		// Maximum possible size of textures affects graphics quality
		score += device_properties.limits.maxImageDimension2D;

		// Check for swapchain support
		PVulkanSwapchainSupport swapchain = _vulkan_swapchain_auto_pick(device, surface);
		if (swapchain.format == NULL || swapchain.present_mode == NULL)
		{
			score = -1;
			goto end_device_score_eval;
		}
		free(swapchain.format);
		free(swapchain.present_mode);

		// evaluate optional and required queue flags
		VkQueueFlags enabled_queue_flags = _vulkan_enable_queue_flags(vulkan_request_display, device, surface);
		if (enabled_queue_flags == 0ULL)
		{
			score = -1;
			goto end_device_score_eval;
		} else {
			score += 100*e_count_set_bits(enabled_queue_flags);
		}

		if (enabled_queue_flags == 0ULL)
		{
			score = -1;
			goto end_device_score_eval;
		} else {
			score += 100*e_count_set_bits(enabled_queue_flags);
		}

		// evaluate optional and required features
		VkPhysicalDeviceFeatures enabled_features = _vulkan_enable_features(vulkan_request_display, device);
		if (memcmp(&enabled_features, &(VkPhysicalDeviceFeatures){0}, sizeof (VkPhysicalDeviceFeatures)) == 0)
		{
			score = -1;
			goto end_device_score_eval;
		} else {
			uint enabled_features_count =
				(enabled_features.robustBufferAccess ? 1 : 0) +
				(enabled_features.fullDrawIndexUint32 ? 1 : 0) +
				(enabled_features.imageCubeArray ? 1 : 0) +
				(enabled_features.independentBlend ? 1 : 0) +
				(enabled_features.geometryShader ? 1 : 0) +
				(enabled_features.tessellationShader ? 1 : 0) +
				(enabled_features.sampleRateShading ? 1 : 0) +
				(enabled_features.dualSrcBlend ? 1 : 0) +
				(enabled_features.logicOp ? 1 : 0) +
				(enabled_features.multiDrawIndirect ? 1 : 0) +
				(enabled_features.drawIndirectFirstInstance ? 1 : 0) +
				(enabled_features.depthClamp ? 1 : 0) +
				(enabled_features.depthBiasClamp ? 1 : 0) +
				(enabled_features.fillModeNonSolid ? 1 : 0) +
				(enabled_features.depthBounds ? 1 : 0) +
				(enabled_features.wideLines ? 1 : 0) +
				(enabled_features.largePoints ? 1 : 0) +
				(enabled_features.alphaToOne ? 1 : 0) +
				(enabled_features.multiViewport ? 1 : 0) +
				(enabled_features.samplerAnisotropy ? 1 : 0) +
				(enabled_features.textureCompressionETC2 ? 1 : 0) +
				(enabled_features.textureCompressionASTC_LDR ? 1 : 0) +
				(enabled_features.textureCompressionBC ? 1 : 0) +
				(enabled_features.occlusionQueryPrecise ? 1 : 0) +
				(enabled_features.pipelineStatisticsQuery ? 1 : 0) +
				(enabled_features.vertexPipelineStoresAndAtomics ? 1 : 0) +
				(enabled_features.fragmentStoresAndAtomics ? 1 : 0) +
				(enabled_features.shaderTessellationAndGeometryPointSize ? 1 : 0) +
				(enabled_features.shaderImageGatherExtended ? 1 : 0) +
				(enabled_features.shaderStorageImageExtendedFormats ? 1 : 0) +
				(enabled_features.shaderStorageImageMultisample ? 1 : 0) +
				(enabled_features.shaderStorageImageReadWithoutFormat ? 1 : 0) +
				(enabled_features.shaderStorageImageWriteWithoutFormat ? 1 : 0) +
				(enabled_features.shaderUniformBufferArrayDynamicIndexing ? 1 : 0) +
				(enabled_features.shaderSampledImageArrayDynamicIndexing ? 1 : 0) +
				(enabled_features.shaderStorageBufferArrayDynamicIndexing ? 1 : 0) +
				(enabled_features.shaderStorageImageArrayDynamicIndexing ? 1 : 0) +
				(enabled_features.shaderClipDistance ? 1 : 0) +
				(enabled_features.shaderCullDistance ? 1 : 0) +
				(enabled_features.shaderFloat64 ? 1 : 0) +
				(enabled_features.shaderInt64 ? 1 : 0) +
				(enabled_features.shaderInt16 ? 1 : 0) +
				(enabled_features.shaderResourceResidency ? 1 : 0) +
				(enabled_features.shaderResourceMinLod ? 1 : 0) +
				(enabled_features.sparseBinding ? 1 : 0) +
				(enabled_features.sparseResidencyBuffer ? 1 : 0) +
				(enabled_features.sparseResidencyImage2D ? 1 : 0) +
				(enabled_features.sparseResidencyImage3D ? 1 : 0) +
				(enabled_features.sparseResidency2Samples ? 1 : 0) +
				(enabled_features.sparseResidency4Samples ? 1 : 0) +
				(enabled_features.sparseResidency8Samples ? 1 : 0) +
				(enabled_features.sparseResidency16Samples ? 1 : 0) +
				(enabled_features.sparseResidencyAliased ? 1 : 0) +
				(enabled_features.variableMultisampleRate ? 1 : 0) +
				(enabled_features.inheritedQueries ? 1 : 0);
			score += 100*enabled_features_count;
		}

end_device_score_eval:
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
		} else {
			i++;
		}
	}
	e_dynarr_deinit(device_score);

	if (max_score_index >= 0)
	{
		return E_DYNARR_GET(compatible_devices, VkPhysicalDevice, max_score_index);
	} else {
		return VK_NULL_HANDLE;
	}
}

/**
 * p_vulkan_init
 *
 * Initializes vulkan and sets the result in app_instance
 */
PHANTOM_API PVulkanDataApp *p_vulkan_init(PVulkanAppRequest *vulkan_request_app)
{
	PVulkanDataApp *vulkan_data_app = malloc(sizeof *vulkan_data_app);

#ifdef PHANTOM_DEBUG_VULKAN
	p_vulkan_list_available_extensions();
	p_vulkan_list_available_layers();
#endif // PHANTOM_DEBUG_VULKAN

	// create vulkan instance
	VkApplicationInfo vk_app_info = {0};
	vk_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vk_app_info.pApplicationName = "Phantom";
	vk_app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	vk_app_info.pEngineName = "No Engine";
	vk_app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	vk_app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo vk_instance_create_info = {0};
	vk_instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vk_instance_create_info.pApplicationInfo = &vk_app_info;

	// Add all required extensions and optional extensions that exist
	EDynarr *enabled_extensions = _vulkan_enable_extensions(vulkan_request_app->required_extensions,
			vulkan_request_app->optional_extensions);
	vk_instance_create_info.enabledExtensionCount = enabled_extensions->num_items;
	vk_instance_create_info.ppEnabledExtensionNames = enabled_extensions->arr;

	EDynarr *enabled_layers = _vulkan_enable_layers(vulkan_request_app->required_layers,
			vulkan_request_app->optional_layers);
	vk_instance_create_info.enabledLayerCount = enabled_layers->num_items;
	vk_instance_create_info.ppEnabledLayerNames = enabled_layers->arr;
	vk_instance_create_info.flags = 0;

#ifdef PHANTOM_DEBUG_VULKAN
	VkDebugUtilsMessengerCreateInfoEXT vk_debug_utils_messenger_create_info = _vulkan_init_debug_messenger();
	vk_instance_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &vk_debug_utils_messenger_create_info;
#else
	vk_instance_create_info.pNext = NULL;
#endif // PHANTOM_DEBUG_VULKAN

	if (vkCreateInstance(&vk_instance_create_info, NULL, &vulkan_data_app->instance) != VK_SUCCESS)
	{
		e_log_message(E_LOG_ERROR, L"Phantom", L"Error initializing Vulkan instance.");
		exit(1);
	}
	e_dynarr_deinit(enabled_extensions);
	e_dynarr_deinit(enabled_layers);

#ifdef PHANTOM_DEBUG_VULKAN
	if (_vulkan_create_debug_utils_messenger(vulkan_data_app->instance, &vk_debug_utils_messenger_create_info, NULL,
				&vulkan_data_app->debug_messenger) != VK_SUCCESS) {
		e_log_message(E_LOG_ERROR, L"Vulkan General", L"Failed to set up debug messenger!");
		exit(1);
	}
#endif // PHANTOM_DEBUG_VULKAN
	return vulkan_data_app;
}

/**
 * p_vulkan_deinit
 *
 * Deinitializes vulkan
 */
PHANTOM_API void p_vulkan_deinit(PVulkanDataApp *vulkan_data_app)
{
#ifdef PHANTOM_DEBUG_VULKAN
	_vulkan_destroy_debug_utils_messenger(vulkan_data_app->instance, vulkan_data_app->debug_messenger, NULL);
#endif // PHANTOM_DEBUG_VULKAN
	vkDestroyInstance(vulkan_data_app->instance, NULL);
	free(vulkan_data_app);
}

/**
 * p_vulkan_surface_create
 *
 * Creates a vulkan surface based on the platform
 * and assigns it to window_data
 */
PHANTOM_API void p_vulkan_surface_create(PWindowData *window_data, PVulkanDataApp *vulkan_data_app,
		PVulkanDisplayRequest *vulkan_request_display)
{
	PVulkanDataDisplay *vulkan_data_display = malloc(sizeof *vulkan_data_display);
	memset(vulkan_data_display, 0, sizeof *vulkan_data_display);
	vulkan_data_display->instance = vulkan_data_app->instance;

#ifdef PHANTOM_DISPLAY_X11
	VkXcbSurfaceCreateInfoKHR vk_surface_create_info = {0};
	vk_surface_create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	vk_surface_create_info.connection = window_data->display_info->connection;
	vk_surface_create_info.window = window_data->display_info->window;
	if (vkCreateXcbSurfaceKHR(vulkan_data_app->instance, &vk_surface_create_info, NULL, &vulkan_data_display->surface)
			!= VK_SUCCESS)
	{
		e_log_message(E_LOG_ERROR, L"Vulkan General", L"Failed to create XCB surface!");
		exit(1);
	}
#elif defined PHANTOM_DISPLAY_WAYLAND
	// TODO: implement me
#elif defined PHANTOM_DISPLAY_WIN32
	VkWin32SurfaceCreateInfoKHR vk_surface_create_info;
	vk_surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vk_surface_create_info.hwnd = window_data->display_info->hwnd;
	vk_surface_create_info.hinstance = window_data->display_info->hInstance;
	if (vkCreateWin32SurfaceKHR(vulkan_data_app->instance, &vk_surface_create_info, NULL, vulkan_data_display->surface)
			!= VK_SUCCESS)
	{
		e_log_message(E_LOG_ERROR, L"Vulkan General", L"Failed to create Win32 surface!");
		exit(1);
	}
#endif

	VkPhysicalDevice physical_device = p_vulkan_device_auto_pick(vulkan_data_display, vulkan_data_app,
			vulkan_request_display, vulkan_data_display->surface);
	physical_device = p_vulkan_device_auto_pick(vulkan_data_display, vulkan_data_app,
			vulkan_request_display, vulkan_data_display->surface);
	if (physical_device == VK_NULL_HANDLE)
	{
		e_log_message(E_LOG_ERROR, L"Vulkan General", L"Failed to find a suitable GPU!");
		exit(1);
	}
	p_vulkan_device_set(vulkan_data_display, vulkan_request_display,physical_device, vulkan_data_display->surface,
			window_data);

	window_data->vulkan_data_display = vulkan_data_display;
}

/**
 * p_vulkan_surface_destroy
 *
 * Destroys the vulkan_data_display
 */
PHANTOM_API void p_vulkan_surface_destroy(PVulkanDataDisplay *vulkan_data_display)
{
	e_dynarr_deinit(vulkan_data_display->compatible_devices);
	e_dynarr_deinit(vulkan_data_display->swapchain_images);
	vkDestroySwapchainKHR(vulkan_data_display->logical_device, vulkan_data_display->swapchain, NULL);
	vkDestroySurfaceKHR(vulkan_data_display->instance, vulkan_data_display->surface, NULL);
	vkDestroyDevice(vulkan_data_display->logical_device, NULL);
	free(vulkan_data_display);
}

