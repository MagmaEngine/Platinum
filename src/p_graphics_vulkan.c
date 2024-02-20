#include "platinum.h"
#include <string.h>
#include "_graphics_vulkan.h"

/**
 * _vulkan_app_request_convert
 *
 * converts PGraphicalAppRequest into PVulkanAppRequest
 */
static
PVulkanAppRequest *_vulkan_app_request_convert(const PGraphicalAppRequest * const graphical_app_request)
{
	PVulkanAppRequest *vulkan_app_request = malloc(sizeof(PVulkanAppRequest));
	// Convert wrapper request into needed data
	// extensions
	vulkan_app_request->required_extensions = e_dynarr_init(sizeof(char *), 3);
	if (!graphical_app_request->headless)
	{
		e_dynarr_add(vulkan_app_request->required_extensions, E_VOID_PTR_FROM_VALUE(char *,
					VK_KHR_SURFACE_EXTENSION_NAME));
#ifdef PLATINUM_DISPLAY_X11
		e_dynarr_add(vulkan_app_request->required_extensions, E_VOID_PTR_FROM_VALUE(char *,
					VK_KHR_XCB_SURFACE_EXTENSION_NAME));
#elif defined PLATINUM_DISPLAY_WIN32
		e_dynarr_add(vulkan_app_data->required_extensions, E_VOID_PTR_FROM_VALUE(char *,
					VK_KHR_WIN32_SURFACE_EXTENSION_NAME));
#endif // PLATINUM_DISPLAY
	}
#ifdef PLATINUM_DEBUG_GRAPHICS
	e_dynarr_add(vulkan_app_request->required_extensions, E_VOID_PTR_FROM_VALUE(char *,
				VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
#endif // PLATINUM_DEBUG_GRAPHICS

	// layers
	vulkan_app_request->required_layers = e_dynarr_init(sizeof(char *), 1);
#ifdef PLATINUM_DEBUG_GRAPHICS
	e_dynarr_add(vulkan_app_request->required_layers, E_VOID_PTR_FROM_VALUE(char *, "VK_LAYER_KHRONOS_validation"));
#endif // PLATINUM_DEBUG_GRAPHICS
	return vulkan_app_request;
}

/**
 * _vulkan_app_request_destroy
 *
 * deinits data created as a part of request init
 */
static
void _vulkan_app_request_destroy(PVulkanAppRequest *vulkan_app_request)
{
	e_dynarr_deinit(vulkan_app_request->required_extensions);
	e_dynarr_deinit(vulkan_app_request->required_layers);
	free(vulkan_app_request);
}

/**
 * _vulkan_display_request_convert
 *
 * converts PGraphicalDisplayRequest into PVulkanDisplayRequest
 */
static
PVulkanDisplayRequest *_vulkan_display_request_convert(const PGraphicalDisplayRequest * const graphic_display_request)
{
	PVulkanDisplayRequest *vulkan_display_request = malloc(sizeof(PVulkanDisplayRequest));

	// Convert wrapper request into needed data
	vulkan_display_request->stereoscopic = graphic_display_request->stereoscopic;
	vulkan_display_request->require_present = !graphic_display_request->headless;
	// extensions
	vulkan_display_request->required_extensions = e_dynarr_init(sizeof(char *), 3);
	if (!graphic_display_request->headless)
	{
		e_dynarr_add(vulkan_display_request->required_extensions, E_VOID_PTR_FROM_VALUE(char *,
					VK_KHR_SWAPCHAIN_EXTENSION_NAME));
	}
	// layers
	vulkan_display_request->required_layers = e_dynarr_init(sizeof(char *), 1);
#ifdef PLATINUM_DEBUG_GRAPHICS
	e_dynarr_add(vulkan_display_request->required_layers, E_VOID_PTR_FROM_VALUE(char *,
				"VK_LAYER_KHRONOS_validation"));
#endif // PLATINUM_DEBUG_GRAPHICS
	vulkan_display_request->required_queue_flags = VK_QUEUE_GRAPHICS_BIT;
	vulkan_display_request->required_features = (VkPhysicalDeviceFeatures){0};
	vulkan_display_request->required_features.geometryShader = true;
	return vulkan_display_request;
}

/**
 * _vulkan_display_request_destroy
 *
 * deinits data created as a part of request init
 */
void _vulkan_display_request_destroy(PVulkanDisplayRequest *vulkan_display_request)
{
	e_dynarr_deinit(vulkan_display_request->required_extensions);
	e_dynarr_deinit(vulkan_display_request->required_layers);
	free(vulkan_display_request);
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
	p_log_message(P_LOG_INFO, L"Vulkan General", L"Number of extensions: %i", vulkan_available_extension_count);

	VkExtensionProperties *vulkan_available_extensions = malloc(vulkan_available_extension_count *
			sizeof(VkExtensionProperties));
	vkEnumerateInstanceExtensionProperties(NULL, &vulkan_available_extension_count, vulkan_available_extensions);
	for (uint32_t i = 0; i < vulkan_available_extension_count; i++)
		p_log_message(P_LOG_DEBUG, L"Vulkan General", L"Supported extension: %s",
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
	p_log_message(P_LOG_INFO, L"Vulkan General", L"Number of devices: %i", vulkan_available_device_count);
	if (vulkan_available_device_count == 0)
	{
		p_log_message(P_LOG_ERROR, L"Vulkan General", L"Failed to find GPUs with Vulkan support!");
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
	p_log_message(P_LOG_INFO, L"Vulkan General", L"Number of layers: %i", vulkan_available_layer_count);

	VkLayerProperties *vulkan_available_layers = malloc(vulkan_available_layer_count *
			sizeof(*vulkan_available_layers));
	vkEnumerateInstanceLayerProperties(&vulkan_available_layer_count, vulkan_available_layers);

	for (uint32_t i = 0; i < vulkan_available_layer_count; i++) {
		p_log_message(P_LOG_DEBUG, L"Vulkan General", L"Supported layers: %s", vulkan_available_layers[i].layerName);
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

	enum PLogLevel log_level;
	switch (messageSeverity)
	{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			log_level = P_LOG_DEBUG;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			log_level = P_LOG_INFO;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			log_level = P_LOG_WARNING;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			log_level = P_LOG_ERROR;
			break;
		// this one should never happen
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
			log_level = P_LOG_MAX;
			break;
	}
	p_log_message(log_level, log_channel, L"%s", pCallbackData->pMessage);
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
 * _vulkan_get_required_features
 *
 * returns the required features that exist
 * if a required feature does not exist it exits
 */
static VkPhysicalDeviceFeatures _vulkan_get_required_features(const PVulkanDisplayRequest * const vulkan_display_request,
		const VkPhysicalDevice device)
{
	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures(device, &device_features);

	EDynarr *missing_features = e_dynarr_init(sizeof (const wchar_t *), 1);
	if (vulkan_display_request->required_features.robustBufferAccess && !device_features.robustBufferAccess) e_dynarr_add(missing_features, L"robustBufferAccess");
	if (vulkan_display_request->required_features.fullDrawIndexUint32 && !device_features.fullDrawIndexUint32) e_dynarr_add(missing_features, L"fullDrawIndexUint32");
	if (vulkan_display_request->required_features.imageCubeArray && !device_features.imageCubeArray) e_dynarr_add(missing_features, L"imageCubeArray");
	if (vulkan_display_request->required_features.independentBlend && !device_features.independentBlend) e_dynarr_add(missing_features, L"independentBlend");
	if (vulkan_display_request->required_features.geometryShader && !device_features.geometryShader) e_dynarr_add(missing_features, L"geometryShader");
	if (vulkan_display_request->required_features.tessellationShader && !device_features.tessellationShader) e_dynarr_add(missing_features, L"tessellationShader");
	if (vulkan_display_request->required_features.sampleRateShading && !device_features.sampleRateShading) e_dynarr_add(missing_features, L"sampleRateShading");
	if (vulkan_display_request->required_features.dualSrcBlend && !device_features.dualSrcBlend) e_dynarr_add(missing_features, L"dualSrcBlend");
	if (vulkan_display_request->required_features.logicOp && !device_features.logicOp) e_dynarr_add(missing_features, L"logicOp");
	if (vulkan_display_request->required_features.multiDrawIndirect && !device_features.multiDrawIndirect) e_dynarr_add(missing_features, L"multiDrawIndirect");
	if (vulkan_display_request->required_features.drawIndirectFirstInstance && !device_features.drawIndirectFirstInstance) e_dynarr_add(missing_features, L"drawIndirectFirstInstance");
	if (vulkan_display_request->required_features.depthClamp && !device_features.depthClamp) e_dynarr_add(missing_features, L"depthClamp");
	if (vulkan_display_request->required_features.depthBiasClamp && !device_features.depthBiasClamp) e_dynarr_add(missing_features, L"depthBiasClamp");
	if (vulkan_display_request->required_features.fillModeNonSolid && !device_features.fillModeNonSolid) e_dynarr_add(missing_features, L"fillModeNonSolid");
	if (vulkan_display_request->required_features.depthBounds && !device_features.depthBounds) e_dynarr_add(missing_features, L"depthBounds");
	if (vulkan_display_request->required_features.wideLines && !device_features.wideLines) e_dynarr_add(missing_features, L"wideLines");
	if (vulkan_display_request->required_features.largePoints && !device_features.largePoints) e_dynarr_add(missing_features, L"largePoints");
	if (vulkan_display_request->required_features.alphaToOne && !device_features.alphaToOne) e_dynarr_add(missing_features, L"alphaToOne");
	if (vulkan_display_request->required_features.multiViewport && !device_features.multiViewport) e_dynarr_add(missing_features, L"multiViewport");
	if (vulkan_display_request->required_features.samplerAnisotropy && !device_features.samplerAnisotropy) e_dynarr_add(missing_features, L"samplerAnisotropy");
	if (vulkan_display_request->required_features.textureCompressionETC2 && !device_features.textureCompressionETC2) e_dynarr_add(missing_features, L"textureCompressionETC2");
	if (vulkan_display_request->required_features.textureCompressionASTC_LDR && !device_features.textureCompressionASTC_LDR) e_dynarr_add(missing_features, L"textureCompressionASTC_LDR");
	if (vulkan_display_request->required_features.textureCompressionBC && !device_features.textureCompressionBC) e_dynarr_add(missing_features, L"textureCompressionBC");
	if (vulkan_display_request->required_features.occlusionQueryPrecise && !device_features.occlusionQueryPrecise) e_dynarr_add(missing_features, L"occlusionQueryPrecise");
	if (vulkan_display_request->required_features.pipelineStatisticsQuery && !device_features.pipelineStatisticsQuery) e_dynarr_add(missing_features, L"pipelineStatisticsQuery");
	if (vulkan_display_request->required_features.vertexPipelineStoresAndAtomics && !device_features.vertexPipelineStoresAndAtomics) e_dynarr_add(missing_features, L"vertexPipelineStoresAndAtomics");
	if (vulkan_display_request->required_features.fragmentStoresAndAtomics && !device_features.fragmentStoresAndAtomics) e_dynarr_add(missing_features, L"fragmentStoresAndAtomics");
	if (vulkan_display_request->required_features.shaderTessellationAndGeometryPointSize && !device_features.shaderTessellationAndGeometryPointSize) e_dynarr_add(missing_features, L"shaderTessellationAndGeometryPointSize");
	if (vulkan_display_request->required_features.shaderImageGatherExtended && !device_features.shaderImageGatherExtended) e_dynarr_add(missing_features, L"shaderImageGatherExtended");
	if (vulkan_display_request->required_features.shaderStorageImageExtendedFormats && !device_features.shaderStorageImageExtendedFormats) e_dynarr_add(missing_features, L"shaderStorageImageExtendedFormats");
	if (vulkan_display_request->required_features.shaderStorageImageMultisample && !device_features.shaderStorageImageMultisample) e_dynarr_add(missing_features, L"shaderStorageImageMultisample");
	if (vulkan_display_request->required_features.shaderStorageImageReadWithoutFormat && !device_features.shaderStorageImageReadWithoutFormat) e_dynarr_add(missing_features, L"shaderStorageImageReadWithoutFormat");
	if (vulkan_display_request->required_features.shaderStorageImageWriteWithoutFormat && !device_features.shaderStorageImageWriteWithoutFormat) e_dynarr_add(missing_features, L"shaderStorageImageWriteWithoutFormat");
	if (vulkan_display_request->required_features.shaderUniformBufferArrayDynamicIndexing && !device_features.shaderUniformBufferArrayDynamicIndexing) e_dynarr_add(missing_features, L"shaderUniformBufferArrayDynamicIndexing");
	if (vulkan_display_request->required_features.shaderSampledImageArrayDynamicIndexing && !device_features.shaderSampledImageArrayDynamicIndexing) e_dynarr_add(missing_features, L"shaderSampledImageArrayDynamicIndexing");
	if (vulkan_display_request->required_features.shaderStorageBufferArrayDynamicIndexing && !device_features.shaderStorageBufferArrayDynamicIndexing) e_dynarr_add(missing_features, L"shaderStorageBufferArrayDynamicIndexing");
	if (vulkan_display_request->required_features.shaderStorageImageArrayDynamicIndexing && !device_features.shaderStorageImageArrayDynamicIndexing) e_dynarr_add(missing_features, L"shaderStorageImageArrayDynamicIndexing");
	if (vulkan_display_request->required_features.shaderClipDistance && !device_features.shaderClipDistance) e_dynarr_add(missing_features, L"shaderClipDistance");
	if (vulkan_display_request->required_features.shaderCullDistance && !device_features.shaderCullDistance) e_dynarr_add(missing_features, L"shaderCullDistance");
	if (vulkan_display_request->required_features.shaderFloat64 && !device_features.shaderFloat64) e_dynarr_add(missing_features, L"shaderFloat64");
	if (vulkan_display_request->required_features.shaderInt64 && !device_features.shaderInt64) e_dynarr_add(missing_features, L"shaderInt64");
	if (vulkan_display_request->required_features.shaderInt16 && !device_features.shaderInt16) e_dynarr_add(missing_features, L"shaderInt16");
	if (vulkan_display_request->required_features.shaderResourceResidency && !device_features.shaderResourceResidency) e_dynarr_add(missing_features, L"shaderResourceResidency");
	if (vulkan_display_request->required_features.shaderResourceMinLod && !device_features.shaderResourceMinLod) e_dynarr_add(missing_features, L"shaderResourceMinLod");
	if (vulkan_display_request->required_features.sparseBinding && !device_features.sparseBinding) e_dynarr_add(missing_features, L"sparseBinding");
	if (vulkan_display_request->required_features.sparseResidencyBuffer && !device_features.sparseResidencyBuffer) e_dynarr_add(missing_features, L"sparseResidencyBuffer");
	if (vulkan_display_request->required_features.sparseResidencyImage2D && !device_features.sparseResidencyImage2D) e_dynarr_add(missing_features, L"sparseResidencyImage2D");
	if (vulkan_display_request->required_features.sparseResidencyImage3D && !device_features.sparseResidencyImage3D) e_dynarr_add(missing_features, L"sparseResidencyImage3D");
	if (vulkan_display_request->required_features.sparseResidency2Samples && !device_features.sparseResidency2Samples) e_dynarr_add(missing_features, L"sparseResidency2Samples");
	if (vulkan_display_request->required_features.sparseResidency4Samples && !device_features.sparseResidency4Samples) e_dynarr_add(missing_features, L"sparseResidency4Samples");
	if (vulkan_display_request->required_features.sparseResidency8Samples && !device_features.sparseResidency8Samples) e_dynarr_add(missing_features, L"sparseResidency8Samples");
	if (vulkan_display_request->required_features.sparseResidency16Samples && !device_features.sparseResidency16Samples) e_dynarr_add(missing_features, L"sparseResidency16Samples");
	if (vulkan_display_request->required_features.sparseResidencyAliased && !device_features.sparseResidencyAliased) e_dynarr_add(missing_features, L"sparseResidencyAliased");
	if (vulkan_display_request->required_features.variableMultisampleRate && !device_features.variableMultisampleRate) e_dynarr_add(missing_features, L"variableMultisampleRate");
	if (vulkan_display_request->required_features.inheritedQueries && !device_features.inheritedQueries) e_dynarr_add(missing_features, L"inheritedQueries");
	if (e_dynarr_item_count(missing_features) > 0)
	{
		p_log_message(P_LOG_ERROR, L"Phantom", L"Vulkan required features not supported");
		for (uint i = 0; i < e_dynarr_item_count(missing_features); i++)
			p_log_message(P_LOG_ERROR, L"Phantom", L"\t%ls", e_dynarr_get(missing_features, wchar_t *, i));
		exit(1);
	}
	e_dynarr_deinit(missing_features);
	return vulkan_display_request->required_features;
}

/**
 * _vulkan_get_required_queue_flags
 *
 * returns the required queue flags that exist
 * if a required queue flag does not exist it exits
 */
static VkQueueFlags _vulkan_get_required_queue_flags(const PVulkanDisplayRequest * const vulkan_display_request,
		const VkPhysicalDevice device,
		const VkSurfaceKHR surface)
{
	if (vulkan_display_request->require_present)
	{
		PVulkanQueueFamilyInfo queue_family_info = _vulkan_find_viable_queue_family_info(
				device, surface, 0, true);
		if (!queue_family_info.exists)
		{
			p_log_message(P_LOG_ERROR, L"Vulkan General", L"Presentation not supported.");
			exit(1);
		}
	}

	VkQueueFlags queue_flags = 0ULL;
	// evaluate optional and required queue flags
	for (VkQueueFlags flag = 1ULL; flag < VK_QUEUE_FLAG_BITS_MAX_ENUM; flag <<= 1)
	{
		PVulkanQueueFamilyInfo queue_family_info = _vulkan_find_viable_queue_family_info(device, surface, flag, false);
		if ((flag & vulkan_display_request->required_queue_flags) && !queue_family_info.exists)
		{
			p_log_message(P_LOG_ERROR, L"Vulkan General", L"Required Vulkan queue type \"%i\" does not exist!", flag);
			exit(1);
		}
	}
	queue_flags |= vulkan_display_request->required_queue_flags;
	return queue_flags;
}

/**
 * _vulkan_get_required_extensions
 *
 * returns the required and optional extensions that exist
 * if a required extension does not exist it exits
 */
static EDynarr *_vulkan_get_required_extensions(const EDynarr * const required_extensions)
{
	EDynarr *extensions = e_dynarr_init(sizeof (char *), e_dynarr_item_count(required_extensions));
	for (uint i = 0; i < e_dynarr_item_count(required_extensions); i++)
	{
		char *extension = e_dynarr_get(required_extensions, char *, i);
		if(_vulkan_extension_exists(extension))
			e_dynarr_add(extensions, E_VOID_PTR_FROM_VALUE(char *, extension));
		else
		{
			p_log_message(P_LOG_ERROR, L"Vulkan General", L"Required extension \"%s\" does not exist!", extension);
			exit(1);
		}
	}
	return extensions;
}

/**
 * _vulkan_get_required_layers
 *
 * returns the required and optional layers that exist
 * if a required layer does not exist it exits
 */
EDynarr *_vulkan_get_required_layers(const EDynarr * const required_layers)
{
	EDynarr *enabled_layers = e_dynarr_init(sizeof (char *), e_dynarr_item_count(required_layers));
	for (uint i = 0; i < e_dynarr_item_count(required_layers); i++)
	{
		char *layer = e_dynarr_get(required_layers, char *, i);
		if(_vulkan_layer_exists(layer))
			e_dynarr_add(enabled_layers, E_VOID_PTR_FROM_VALUE(char *,layer));
		else
		{
			p_log_message(P_LOG_ERROR, L"Vulkan General", L"Required layer \"%s\" does not exist!", layer);
			exit(1);
		}
	}
	return enabled_layers;
}

/**
 * _vulkan_swapchain_create
 *
 * helper function that creates a swapchain for the given window
 */
static void _vulkan_swapchain_create(
		PGraphicalDisplayData const vulkan_display_data,
		const uint32_t framebuffer_width,
		const uint32_t framebuffer_height,
		const PGraphicalDisplayRequest * const graphical_display_request,
		const PVulkanSwapchainSupport swapchain_support)
{

	// Set extent
	vulkan_display_data->swapchain_extent.width = E_MIN(
			E_MAX(framebuffer_width, swapchain_support.capabilities.minImageExtent.width),
			swapchain_support.capabilities.maxImageExtent.width);

	vulkan_display_data->swapchain_extent.height = E_MIN(
			E_MAX(framebuffer_height, swapchain_support.capabilities.minImageExtent.height),
			swapchain_support.capabilities.maxImageExtent.height);

	// Save format for later
	vulkan_display_data->swapchain_format = swapchain_support.format->format;

	// Set image count
	uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
	if (swapchain_support.capabilities.maxImageCount > 0 && image_count > swapchain_support.capabilities.maxImageCount)
		image_count = swapchain_support.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR swapchain_create_info = {0};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface = vulkan_display_data->surface;
	swapchain_create_info.minImageCount = image_count;
	swapchain_create_info.imageFormat = swapchain_support.format->format;
	swapchain_create_info.imageColorSpace = swapchain_support.format->colorSpace;
	swapchain_create_info.imageExtent = vulkan_display_data->swapchain_extent;
	// TODO: add options to change this
	swapchain_create_info.preTransform = swapchain_support.capabilities.currentTransform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = *swapchain_support.present_mode;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.oldSwapchain = VK_NULL_HANDLE; // TODO: change this when we recreate the swapchain
	swapchain_create_info.imageArrayLayers = (graphical_display_request->stereoscopic) ? 2 : 1;

	// TODO: change this if/when we add post processing
	// right now we are rendering directly to the swapchain
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Set queue families
	if (vulkan_display_data->queue_family_infos[P_VULKAN_QUEUE_TYPE_GRAPHICS].index !=
			vulkan_display_data->queue_family_infos[P_VULKAN_QUEUE_TYPE_PRESENT].index)
	{
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_create_info.queueFamilyIndexCount = 2;
		swapchain_create_info.pQueueFamilyIndices = (uint32_t[]) {
			vulkan_display_data->queue_family_infos[P_VULKAN_QUEUE_TYPE_GRAPHICS].index,
			vulkan_display_data->queue_family_infos[P_VULKAN_QUEUE_TYPE_PRESENT].index
		};
	} else {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount = 0; // Optional
		swapchain_create_info.pQueueFamilyIndices = NULL; // Optional
	}

	if (vulkan_display_data->swapchain != VK_NULL_HANDLE)
		vkDestroySwapchainKHR(vulkan_display_data->logical_device, vulkan_display_data->swapchain, NULL);
	if (vkCreateSwapchainKHR(vulkan_display_data->logical_device, &swapchain_create_info, NULL,
			&vulkan_display_data->swapchain) != VK_SUCCESS)
	{
		p_log_message(P_LOG_ERROR, L"Vulkan General", L"Failed to create swapchain!");
		exit(1);
	}
	vkGetSwapchainImagesKHR(vulkan_display_data->logical_device, vulkan_display_data->swapchain, &image_count, NULL);
	VkImage swapchain_images[image_count];
	vkGetSwapchainImagesKHR(vulkan_display_data->logical_device, vulkan_display_data->swapchain, &image_count,
			swapchain_images);
	vulkan_display_data->swapchain_images = e_dynarr_init_arr(sizeof (VkImage), image_count, swapchain_images);

	// Create image views
	vulkan_display_data->swapchain_image_views = e_dynarr_init(sizeof (VkImageView), image_count);

	for (uint i = 0; i < image_count; i++)
	{
		VkImageViewCreateInfo image_view_create_info = {0};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.image = e_dynarr_get(vulkan_display_data->swapchain_images, VkImage, i);
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = vulkan_display_data->swapchain_format;
		image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = 1;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = 1;

		VkImageView image_view;
		if (vkCreateImageView(vulkan_display_data->logical_device, &image_view_create_info, NULL, &image_view) != VK_SUCCESS)
		{
			p_log_message(P_LOG_ERROR, L"Vulkan General", L"Failed to create image view!");
			exit(1);
		}
		e_dynarr_add(vulkan_display_data->swapchain_image_views, &image_view);
	}

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
		VkSurfaceFormatKHR formats[format_count];
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats);
		EDynarr *surface_formats = e_dynarr_init_arr(sizeof (VkSurfaceFormatKHR), format_count, formats);

		VkSurfaceFormatKHR format = e_dynarr_get(surface_formats, VkSurfaceFormatKHR, 0);
		for (uint i = 0; i < format_count; i++)
		{
			VkSurfaceFormatKHR current_format = e_dynarr_get(surface_formats, VkSurfaceFormatKHR, i);
			if (current_format.format == VK_FORMAT_B8G8R8A8_SRGB)
			{
				format = current_format;
				break;
			}
		}
		swapchain_support.format = malloc(sizeof (VkSurfaceFormatKHR));
		*swapchain_support.format = format;
		e_dynarr_deinit(surface_formats);
	}

	// Set present mode
	uint32_t present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL);
	if (present_mode_count != 0)
	{
		VkPresentModeKHR modes[present_mode_count];
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, modes);
		EDynarr *present_modes = e_dynarr_init_arr(sizeof (VkPresentModeKHR), present_mode_count, modes);

		VkPresentModeKHR present_mode = e_dynarr_get(present_modes, VkPresentModeKHR, 0);
		for (uint i = 0; i < present_mode_count; i++)
		{
			VkPresentModeKHR current_present_mode = e_dynarr_get(present_modes, VkPresentModeKHR, i);
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
 * p_graphics_vulkan_device_set
 *
 * sets the physical device to use in vulkan_display_data from physical_device
 * also creates a logical device with vulkan_app_request and sets it to vulkan_display_data
 */
void p_graphics_vulkan_device_set(
		PGraphicalDisplayData graphical_display_data,
		const PGraphicalDisplayRequest * const graphical_display_request,
		const PGraphicalDevice physical_device,
		const PGraphicalDisplayData display,
		const PWindowData * const window_data)

{
	// convert
	PVulkanDisplayRequest *vulkan_display_request = _vulkan_display_request_convert(graphical_display_request);
	PGraphicalDisplayData vulkan_display_data = graphical_display_data;

	if (e_dynarr_find(vulkan_display_data->compatible_devices, &physical_device.handle) == -1)
	{
		p_log_message(P_LOG_ERROR, L"Vulkan General", L"Selected device is not compatible");
		exit(1);
	}

	// Set physical device
	vulkan_display_data->current_physical_device = physical_device.handle;

	// Set queue family indices
	VkQueueFlags enabled_queue_flags = _vulkan_get_required_queue_flags(vulkan_display_request, physical_device.handle, display->surface);
	if (enabled_queue_flags & VK_QUEUE_GRAPHICS_BIT)
		vulkan_display_data->queue_family_infos[P_VULKAN_QUEUE_TYPE_GRAPHICS] =
			_vulkan_find_viable_queue_family_info(physical_device.handle, display->surface, VK_QUEUE_GRAPHICS_BIT, false);
	if (vulkan_display_request->require_present)
		vulkan_display_data->queue_family_infos[P_VULKAN_QUEUE_TYPE_PRESENT] =
			_vulkan_find_viable_queue_family_info(physical_device.handle, display->surface, VK_QUEUE_GRAPHICS_BIT, true);
	if (enabled_queue_flags & VK_QUEUE_COMPUTE_BIT)
		vulkan_display_data->queue_family_infos[P_VULKAN_QUEUE_TYPE_COMPUTE] =
			_vulkan_find_viable_queue_family_info(physical_device.handle, display->surface, VK_QUEUE_COMPUTE_BIT, false);
	if (enabled_queue_flags & VK_QUEUE_TRANSFER_BIT)
		vulkan_display_data->queue_family_infos[P_VULKAN_QUEUE_TYPE_TRANSFER] =
			_vulkan_find_viable_queue_family_info(physical_device.handle, display->surface, VK_QUEUE_TRANSFER_BIT, false);
	if (enabled_queue_flags & VK_QUEUE_SPARSE_BINDING_BIT)
		vulkan_display_data->queue_family_infos[P_VULKAN_QUEUE_TYPE_SPARSE] =
			_vulkan_find_viable_queue_family_info(physical_device.handle, display->surface, VK_QUEUE_SPARSE_BINDING_BIT, false);
	if (enabled_queue_flags & VK_QUEUE_PROTECTED_BIT)
		vulkan_display_data->queue_family_infos[P_VULKAN_QUEUE_TYPE_PROTECTED] =
			_vulkan_find_viable_queue_family_info(physical_device.handle, display->surface, VK_QUEUE_PROTECTED_BIT, false);
	if (enabled_queue_flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
		vulkan_display_data->queue_family_infos[P_VULKAN_QUEUE_TYPE_VIDEO_DECODE] =
			_vulkan_find_viable_queue_family_info(physical_device.handle, display->surface, VK_QUEUE_VIDEO_DECODE_BIT_KHR, false);
	if (enabled_queue_flags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)
		vulkan_display_data->queue_family_infos[P_VULKAN_QUEUE_TYPE_OPTICAL_FLOW] =
			_vulkan_find_viable_queue_family_info(physical_device.handle, display->surface, VK_QUEUE_OPTICAL_FLOW_BIT_NV, false);

	EDynarr *queue_family_infos = e_dynarr_init(sizeof (PVulkanQueueFamilyInfo), 1);
	for (uint i = 0; i < P_VULKAN_QUEUE_TYPE_MAX; i++)
	{
		bool unique = true;
		for (uint j = 0; j < e_dynarr_item_count(queue_family_infos); j++)
		{
			if ((e_dynarr_get(queue_family_infos, PVulkanQueueFamilyInfo, j).exists) &&
				vulkan_display_data->queue_family_infos[i].index ==
				e_dynarr_get(queue_family_infos, PVulkanQueueFamilyInfo, j).index)
			{
				unique = false;
				break;
			}
		}
		if (unique)
			e_dynarr_add(queue_family_infos, &vulkan_display_data->queue_family_infos[i]);
	}

	const uint num_queue_families = e_dynarr_item_count(queue_family_infos);
	VkDeviceQueueCreateInfo queue_create_infos[num_queue_families];
	memset(queue_create_infos, 0, sizeof queue_create_infos);
	for (uint i = 0; i < num_queue_families; i++)
	{
		queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[i].queueFamilyIndex = e_dynarr_get(queue_family_infos, PVulkanQueueFamilyInfo, i).index;
		queue_create_infos[i].queueCount = 1;
		queue_create_infos[i].pQueuePriorities = E_PTR_FROM_VALUE(float, 1.f);
	}

	// Set device features
	VkPhysicalDeviceFeatures device_features  = _vulkan_get_required_features(vulkan_display_request, physical_device.handle);

	// Create logical device
	VkDeviceCreateInfo device_create_info = {0};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pQueueCreateInfos = queue_create_infos;
	device_create_info.queueCreateInfoCount = num_queue_families;
	device_create_info.pEnabledFeatures = &device_features;

	EDynarr *enabled_extensions = _vulkan_get_required_extensions(vulkan_display_request->required_extensions);
	device_create_info.enabledExtensionCount = e_dynarr_item_count(enabled_extensions);
	device_create_info.ppEnabledExtensionNames = e_dynarr_get_arr(enabled_extensions);

	EDynarr *enabled_layers = _vulkan_get_required_layers(vulkan_display_request->required_layers);
	device_create_info.enabledLayerCount = e_dynarr_item_count(enabled_layers);
	device_create_info.ppEnabledLayerNames = e_dynarr_get_arr(enabled_layers);

	_vulkan_display_request_destroy(vulkan_display_request);

	// Set logical device queues
	EDynarr *device_queue_create_infos = e_dynarr_init(sizeof (VkDeviceQueueCreateInfo), 1);
	for (uint i = 0; i < e_dynarr_item_count(queue_family_infos); i++)
	{
		VkDeviceQueueCreateInfo queue_create_info = {0};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = e_dynarr_get(queue_family_infos, PVulkanQueueFamilyInfo, i).index;
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = E_PTR_FROM_VALUE(float, 1.f);
		e_dynarr_add(device_queue_create_infos, &queue_create_info);
	}
	device_create_info.queueCreateInfoCount = e_dynarr_item_count(device_queue_create_infos);
	device_create_info.pQueueCreateInfos = e_dynarr_get_arr(device_queue_create_infos);

	// create logical device
	if (vulkan_display_data->logical_device != VK_NULL_HANDLE)
		vkDestroyDevice(vulkan_display_data->logical_device, NULL);
	if (vkCreateDevice(vulkan_display_data->current_physical_device, &device_create_info, NULL,
				&vulkan_display_data->logical_device) != VK_SUCCESS)
	{
		p_log_message(P_LOG_ERROR, L"Vulkan General", L"Failed to create logical device!");
		exit(1);
	}
	e_dynarr_deinit(enabled_extensions);
	e_dynarr_deinit(enabled_layers);
	e_dynarr_deinit(device_queue_create_infos);
	e_dynarr_deinit(queue_family_infos);

	// Set queue handles
	for (uint i = 0; i < P_VULKAN_QUEUE_TYPE_MAX; i++)
	{
		if (!vulkan_display_data->queue_family_infos[i].exists)
			continue;
		VkQueue vk_queue = {0};
		vkGetDeviceQueue(vulkan_display_data->logical_device, vulkan_display_data->queue_family_infos[i].index, 0, &vk_queue);
		vulkan_display_data->queue_family_infos[i].queue = vk_queue;
	}

	// Set swapchain data
	PVulkanSwapchainSupport swapchain_support = _vulkan_swapchain_auto_pick(physical_device.handle, display->surface);
	_vulkan_swapchain_create(vulkan_display_data, window_data->x, window_data->y, graphical_display_request,
			swapchain_support);
	free(swapchain_support.format);
	free(swapchain_support.present_mode);
}

/**
 * p_graphics_vulkan_device_auto_pick
 *
 * initializes compatible_devices in vulkan_display_data
 * picks the physical device for use with vulkan and returns it
 * returns VK_NULL_HANDLE if no suitable device is found
 */
PGraphicalDevice p_graphics_vulkan_device_auto_pick(
		PGraphicalDisplayData graphical_display_data,
		const PGraphicalAppData graphical_app_data,
		const PGraphicalDisplayRequest * const graphical_display_request)
{
	PGraphicalDisplayData vulkan_display_data = graphical_display_data;
	const PGraphicalAppData vulkan_app_data = graphical_app_data;
	vulkan_display_data->current_physical_device = VK_NULL_HANDLE;

	uint32_t vulkan_available_device_count = 0;
	vkEnumeratePhysicalDevices(vulkan_app_data->instance, &vulkan_available_device_count, NULL);

	if (vulkan_display_data->compatible_devices != NULL)
		e_dynarr_deinit(vulkan_display_data->compatible_devices);
	{
		VkPhysicalDevice compatible_devices[vulkan_available_device_count];
		vkEnumeratePhysicalDevices(vulkan_app_data->instance, &vulkan_available_device_count, compatible_devices);
		vulkan_display_data->compatible_devices = e_dynarr_init_arr(sizeof (VkPhysicalDevice),
				vulkan_available_device_count, compatible_devices);
	}
	EDynarr *compatible_devices = vulkan_display_data->compatible_devices;

	// check device suitability, assign scores
	EDynarr *device_score = e_dynarr_init(sizeof (int), e_dynarr_item_count(compatible_devices));
	for (uint i = 0; i < e_dynarr_item_count(compatible_devices); i++) {
		int score = 0;
		VkPhysicalDevice device = e_dynarr_get(compatible_devices, VkPhysicalDevice, i);
		VkPhysicalDeviceProperties device_properties = {0};
		vkGetPhysicalDeviceProperties(device, &device_properties);

		// lots of score goes to discrete gpus
		if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score = 10000;

		// Maximum possible size of textures affects graphics quality
		score += device_properties.limits.maxImageDimension2D;

		// Check for swapchain support
		PVulkanSwapchainSupport swapchain = _vulkan_swapchain_auto_pick(device, vulkan_display_data->surface);
		if (swapchain.format == NULL || swapchain.present_mode == NULL)
		{
			score = -1;
			goto end_device_score_eval;
		}
		free(swapchain.format);
		free(swapchain.present_mode);

		// get required queue flags to make sure they exist
		PVulkanDisplayRequest *vulkan_display_request = _vulkan_display_request_convert(graphical_display_request);
		_vulkan_get_required_queue_flags(vulkan_display_request, device, vulkan_display_data->surface);

		// get required features to make sure they exist
		_vulkan_get_required_features(vulkan_display_request, device);
		_vulkan_display_request_destroy(vulkan_display_request);

end_device_score_eval:
		e_dynarr_add(device_score, E_VOID_PTR_FROM_VALUE(int, score));
	}

	// remove incompatible gpus and set default gpu
	int max_score = 0;
	int max_score_index = -1;
	for (uint i = 0; i < e_dynarr_item_count(compatible_devices);)
	{
		int current_device_score = e_dynarr_get(device_score, int, i);
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
		PGraphicalDevice device = (PGraphicalDevice){0};
		device.handle = e_dynarr_get(compatible_devices, VkPhysicalDevice, max_score_index);
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(device.handle, &device_properties);
		device.name = device_properties.deviceName;
		return device;
	} else {
		return (PGraphicalDevice){0};
	}
}

/**
 * p_graphics_vulkan_init
 *
 * Initializes vulkan and sets the result in app_instance
 */
PGraphicalAppData p_graphics_vulkan_init(PGraphicalAppRequest *graphical_app_request)
{
	PVulkanAppRequest *vulkan_app_request = _vulkan_app_request_convert(graphical_app_request);
	PGraphicalAppData vulkan_app_data = malloc(sizeof *vulkan_app_data);

#ifdef PLATINUM_DEBUG_GRAPHICS
	p_vulkan_list_available_extensions();
	p_vulkan_list_available_layers();
#endif // PLATINUM_DEBUG_GRAPHICS

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
	EDynarr *enabled_extensions = _vulkan_get_required_extensions(vulkan_app_request->required_extensions);
	vk_instance_create_info.enabledExtensionCount = e_dynarr_item_count(enabled_extensions);
	vk_instance_create_info.ppEnabledExtensionNames = e_dynarr_get_arr(enabled_extensions);

	EDynarr *enabled_layers = _vulkan_get_required_layers(vulkan_app_request->required_layers);
	vk_instance_create_info.enabledLayerCount = e_dynarr_item_count(enabled_layers);
	vk_instance_create_info.ppEnabledLayerNames = e_dynarr_get_arr(enabled_layers);
	vk_instance_create_info.flags = 0;

	_vulkan_app_request_destroy(vulkan_app_request);

#ifdef PLATINUM_DEBUG_GRAPHICS
	VkDebugUtilsMessengerCreateInfoEXT vk_debug_utils_messenger_create_info = _vulkan_init_debug_messenger();
	vk_instance_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &vk_debug_utils_messenger_create_info;
#else
	vk_instance_create_info.pNext = NULL;
#endif // PLATINUM_DEBUG_GRAPHICS

	if (vkCreateInstance(&vk_instance_create_info, NULL, &vulkan_app_data->instance) != VK_SUCCESS)
	{
		p_log_message(P_LOG_ERROR, L"Phantom", L"Error initializing Vulkan instance.");
		exit(1);
	}
	e_dynarr_deinit(enabled_extensions);
	e_dynarr_deinit(enabled_layers);

#ifdef PLATINUM_DEBUG_GRAPHICS
	if (_vulkan_create_debug_utils_messenger(vulkan_app_data->instance, &vk_debug_utils_messenger_create_info, NULL,
				&vulkan_app_data->debug_messenger) != VK_SUCCESS) {
		p_log_message(P_LOG_ERROR, L"Vulkan General", L"Failed to set up debug messenger!");
		exit(1);
	}
#endif // PLATINUM_DEBUG_GRAPHICS
	return vulkan_app_data;
}

/**
 * p_graphics_vulkan_deinit
 *
 * Deinitializes vulkan
 */
void p_graphics_vulkan_deinit(PGraphicalAppData vulkan_app_data)
{
#ifdef PLATINUM_DEBUG_GRAPHICS
	_vulkan_destroy_debug_utils_messenger(vulkan_app_data->instance, vulkan_app_data->debug_messenger, NULL);
#endif // PLATINUM_DEBUG_GRAPHICS
	vkDestroyInstance(vulkan_app_data->instance, NULL);
	free(vulkan_app_data);
}

/**
 * _create_shader_module
 *
 * TODO: move me into renderer
 */
VkShaderModule _create_shader_module(PGraphicalDisplayData vulkan_display_data, const char *shader_data,
		uint shader_data_size)
{
	VkShaderModule shader_module;
	VkShaderModuleCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = shader_data_size;
	create_info.pCode = (uint32_t *)shader_data;
	if (vkCreateShaderModule(vulkan_display_data->logical_device, &create_info, NULL, &shader_module)
			!= VK_SUCCESS)
	{
		p_log_message(P_LOG_ERROR, L"Vulkan General", L"Failed to create shader module!");
		exit(1);
	}
	return shader_module;
}

/**
 * _graphics_vulkan_pipeline_init
 *
 * Initializes the pipeline for vulkan
 */
void _graphics_vulkan_pipeline_init(PGraphicalDisplayData vulkan_display_data)
{
	// Create render pass
	VkAttachmentDescription color_attachment_description = {0};
	color_attachment_description.format = vulkan_display_data->swapchain_format;
	color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Create subpass
	VkAttachmentReference color_attachment_reference = {0};
	color_attachment_reference.attachment = 0;
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass_description = {0};
	subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_description.colorAttachmentCount = 1;
	subpass_description.pColorAttachments = &color_attachment_reference;

	VkRenderPassCreateInfo render_pass_create_info = {0};
	render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_create_info.attachmentCount = 1;
	render_pass_create_info.pAttachments = &color_attachment_description;
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass_description;
	if (vkCreateRenderPass(vulkan_display_data->logical_device, &render_pass_create_info, NULL,
				&vulkan_display_data->render_pass) != VK_SUCCESS)
	{
		p_log_message(P_LOG_ERROR, L"Vulkan General", L"Failed to create render pass!");
		exit(1);
	}

	// TODO: refactor this to get shader path from config, also put render stuff in renderer
	char *shader_vert_path = "build/src/platinum/shaders/shader_vert.spv";
	uint shader_vert_size = p_file_get_size(shader_vert_path);
	char *shader_vert_data = malloc(shader_vert_size * sizeof(char));
	p_file_read(shader_vert_path, shader_vert_data, shader_vert_size);

	char *shader_frag_path = "build/src/platinum/shaders/shader_frag.spv";
	uint shader_frag_size = p_file_get_size(shader_frag_path);
	char *shader_frag_data = malloc(shader_frag_size * sizeof(char));
	p_file_read(shader_frag_path, shader_frag_data, shader_frag_size);

	if (vulkan_display_data->shaders != NULL)
		e_dynarr_deinit(vulkan_display_data->shaders);
	vulkan_display_data->shaders = e_dynarr_init(sizeof (VkShaderModule), 2);

	VkShaderModule vertShaderModule = _create_shader_module(vulkan_display_data, shader_vert_data, shader_vert_size);
	VkShaderModule fragShaderModule = _create_shader_module(vulkan_display_data, shader_frag_data, shader_frag_size);
	free(shader_vert_data);
	free(shader_frag_data);

	e_dynarr_add(vulkan_display_data->shaders, &vertShaderModule);
	e_dynarr_add(vulkan_display_data->shaders, &fragShaderModule);

	VkPipelineShaderStageCreateInfo vertex_shader_stage_info = {0};
	vertex_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertex_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertex_shader_stage_info.module = vertShaderModule;
	vertex_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo fragment_shader_stage_info = {0};
	fragment_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragment_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragment_shader_stage_info.module = fragShaderModule;
	fragment_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stage_infos[] = {vertex_shader_stage_info, fragment_shader_stage_info};

	// TODO: make vertex input data real
	// https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/02_Fixed_functions.html#_vertex_input
	VkPipelineVertexInputStateCreateInfo vertex_input_info = {0};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.pVertexBindingDescriptions = NULL;
	vertex_input_info.vertexAttributeDescriptionCount = 0;
	vertex_input_info.pVertexAttributeDescriptions = NULL;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	// TODO: make viewport configurable for different UI elements
	// (draggable columns, addable elements, etc) (like the blender editor ui)
	VkViewport viewport = {0};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) vulkan_display_data->swapchain_extent.width;
	viewport.height = (float) vulkan_display_data->swapchain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// TODO: make scissor configurable for different UI elements
	// just like the viewport. It should be the same size as the viewport
	VkRect2D scissor = {0};
	scissor.offset = (VkOffset2D){0, 0};
	scissor.extent = vulkan_display_data->swapchain_extent;

	EDynarr *dynamic_states = e_dynarr_init_arr(sizeof (VkDynamicState), 2,
			(VkDynamicState[]) {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR} );

	VkPipelineDynamicStateCreateInfo dynamic_state = {0};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = e_dynarr_item_count(dynamic_states);
	dynamic_state.pDynamicStates = e_dynarr_get_arr(dynamic_states);

	// TODO: find a way to make this dynamic
	// so the user can create different viewports and scissors
	VkPipelineViewportStateCreateInfo viewport_state = {0};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {0};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// TODO: Might be useful for shadow maps. Requires depthClamp feature in VkPhysicalDeviceFeatures
	rasterizer.depthClampEnable = VK_FALSE;
	// TODO: make this configurable. Any other mode requires a gpu feature. Might be useful for wireframe and/or point clouds
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	// TODO: make culling configurable
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {0};
	// TODO: make multisampling (anti-aliasing) configurable. Requires a gpu feature
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = NULL;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	// TODO: make color blending configurable
	VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blending = {0};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pSetLayouts = NULL;
	pipeline_layout_info.pushConstantRangeCount = 0;
	pipeline_layout_info.pPushConstantRanges = NULL;

	if (vulkan_display_data->pipeline_layout != NULL)
		vkDestroyPipelineLayout(vulkan_display_data->logical_device, vulkan_display_data->pipeline_layout, NULL);
	if (vkCreatePipelineLayout(vulkan_display_data->logical_device, &pipeline_layout_info, NULL,
				&vulkan_display_data->pipeline_layout) != VK_SUCCESS)
	{
		p_log_message(P_LOG_ERROR, L"Vulkan General", L"Failed to create pipeline layout!");
		exit(1);
	}

	VkGraphicsPipelineCreateInfo pipeline_info = {0};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stage_infos;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = NULL;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDynamicState = &dynamic_state;
	pipeline_info.layout = vulkan_display_data->pipeline_layout;
	pipeline_info.renderPass = vulkan_display_data->render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(vulkan_display_data->logical_device, VK_NULL_HANDLE, 1, &pipeline_info, NULL,
				&vulkan_display_data->graphics_pipeline) != VK_SUCCESS)
	{
		p_log_message(P_LOG_ERROR, L"Vulkan General", L"Failed to create graphics pipeline!");
		exit(1);
	}

	e_dynarr_deinit(dynamic_states);

	// Set up drawing framebuffers
	uint item_count = e_dynarr_item_count(vulkan_display_data->swapchain_images);

	if (vulkan_display_data->swapchain_framebuffers != NULL)
		e_dynarr_deinit(vulkan_display_data->swapchain_framebuffers);
	vulkan_display_data->swapchain_framebuffers = e_dynarr_init(sizeof (VkFramebuffer), item_count);

	for (uint i = 0; i < item_count; i++)
	{
		VkImageView *attachments = e_dynarr_get_arr(vulkan_display_data->swapchain_image_views);
		VkFramebufferCreateInfo framebuffer_info = {0};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = vulkan_display_data->render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = vulkan_display_data->swapchain_extent.width;
		framebuffer_info.height = vulkan_display_data->swapchain_extent.height;
		framebuffer_info.layers = 1;

		VkFramebuffer framebuffer;
		if (vkCreateFramebuffer(vulkan_display_data->logical_device, &framebuffer_info, NULL,
					&framebuffer) != VK_SUCCESS)
		{
			p_log_message(P_LOG_ERROR, L"Vulkan General", L"Failed to create framebuffer!");
			exit(1);
		}
		e_dynarr_add(vulkan_display_data->swapchain_framebuffers, &framebuffer);
	}

}

/**
 * p_graphics_vulkan_display_create
 *
 * Creates a vulkan surface based on the platform
 * and assigns it to window_data
 */
void p_graphics_vulkan_display_create(PWindowData *window_data, const PGraphicalAppData vulkan_app_data,
		const PGraphicalDisplayRequest * const vulkan_display_request)
{
	PGraphicalDisplayData vulkan_display_data = calloc(1, sizeof *vulkan_display_data);
	vulkan_display_data->instance = vulkan_app_data->instance;

	p_window_set_graphical_display(window_data, vulkan_app_data, vulkan_display_data);

	PGraphicalDevice physical_device = p_graphics_vulkan_device_auto_pick(vulkan_display_data, vulkan_app_data,
			vulkan_display_request);
	if (physical_device.handle == VK_NULL_HANDLE)
	{
		p_log_message(P_LOG_ERROR, L"Vulkan General", L"Failed to find a suitable GPU!");
		exit(1);
	}
	p_graphics_vulkan_device_set(vulkan_display_data, vulkan_display_request,physical_device, vulkan_display_data,
			window_data);

	window_data->graphical_display_data = vulkan_display_data;

	PThread thread_pipeline = p_thread_create((PThreadFunction) _graphics_vulkan_pipeline_init, vulkan_display_data);
	p_thread_detach(thread_pipeline);
}

/**
 * p_graphics_vulkan_display_destroy
 *
 * Destroys the vulkan_display_data
 */
void p_graphics_vulkan_display_destroy(PGraphicalDisplayData vulkan_display_data)
{
	for (uint i = 0; i < e_dynarr_item_count(vulkan_display_data->swapchain_framebuffers); i++)
		vkDestroyFramebuffer(vulkan_display_data->logical_device,
				e_dynarr_get(vulkan_display_data->swapchain_framebuffers, VkFramebuffer, i), NULL);
	e_dynarr_deinit(vulkan_display_data->swapchain_framebuffers);

	vkDestroyPipeline(vulkan_display_data->logical_device, vulkan_display_data->graphics_pipeline, NULL);
	vkDestroyPipelineLayout(vulkan_display_data->logical_device, vulkan_display_data->pipeline_layout, NULL);
	vkDestroyRenderPass(vulkan_display_data->logical_device, vulkan_display_data->render_pass, NULL);

	for (uint i = 0; i < e_dynarr_item_count(vulkan_display_data->shaders); i++)
		vkDestroyShaderModule(vulkan_display_data->logical_device,
				e_dynarr_get(vulkan_display_data->shaders, VkShaderModule, i), NULL);
	e_dynarr_deinit(vulkan_display_data->shaders);

	e_dynarr_deinit(vulkan_display_data->compatible_devices);
	e_dynarr_deinit(vulkan_display_data->swapchain_images);

	for (uint i = 0; i < e_dynarr_item_count(vulkan_display_data->swapchain_image_views); i++)
		vkDestroyImageView(vulkan_display_data->logical_device,
				e_dynarr_get(vulkan_display_data->swapchain_image_views, VkImageView, i), NULL);
	e_dynarr_deinit(vulkan_display_data->swapchain_image_views);

	vkDestroySwapchainKHR(vulkan_display_data->logical_device, vulkan_display_data->swapchain, NULL);
	vkDestroySurfaceKHR(vulkan_display_data->instance, vulkan_display_data->surface, NULL);
	vkDestroyDevice(vulkan_display_data->logical_device, NULL);
	free(vulkan_display_data);
}

