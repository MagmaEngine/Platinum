#include "phantom.h"
#include <string.h>

#ifdef PHANTOM_DISPLAY_X11
#include <vulkan/vulkan_xcb.h>
#elif defined PHANTOM_DISPLAY_WIN32
#include <vulkan/vulkan_win32.h>
#endif // PHANTOM_DISPLAY_XXXXXX

/**
 * p_vulkan_init_request
 *
 * generates a PVulkanRequest and returns it as part of request
 */
void p_vulkan_init_request(PVulkanRequest *vulkan_request)
{
#ifdef PHANTOM_DEBUG_VULKAN
	vulkan_request->debug = true;
#else
	vulkan_request->debug = false;
#endif // PHANTOM_DEBUG_VULKAN

	// queue flags
	vulkan_request->required_queue_flags = VK_QUEUE_GRAPHICS_BIT;
	vulkan_request->optional_queue_flags = 0ULL;

	// features
	vulkan_request->required_features = (VkPhysicalDeviceFeatures){0};
	vulkan_request->required_features.geometryShader = true;
	vulkan_request->optional_features = (VkPhysicalDeviceFeatures){0};

	// extensions
	vulkan_request->optional_extensions = e_dynarr_init(sizeof(char *), 1);

	vulkan_request->required_extensions = e_dynarr_init(sizeof(char *), 3);
	e_dynarr_add(vulkan_request->required_extensions, E_VOID_PTR_FROM_VALUE(char *, VK_KHR_SURFACE_EXTENSION_NAME));
#ifdef PHANTOM_DISPLAY_X11
	e_dynarr_add(vulkan_request->required_extensions, E_VOID_PTR_FROM_VALUE(char *, VK_KHR_XCB_SURFACE_EXTENSION_NAME));
#endif // PHANTOM_DISPLAY_X11
#ifdef PHANTOM_DISPLAY_WIN32
	e_dynarr_add(vulkan_request->required_extensions, E_VOID_PTR_FROM_VALUE(char *, VK_KHR_WIN32_SURFACE_EXTENSION_NAME));
#endif // PHANTOM_DISPLAY_WIN32
#ifdef PHANTOM_DEBUG_VULKAN
	e_dynarr_add(vulkan_request->required_extensions, E_VOID_PTR_FROM_VALUE(char *, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
#endif // PHANTOM_DEBUG_VULKAN
}

/**
 * p_vulkan_deinit_request
 *
 * deinits data created as a part of request init
 */
void p_vulkan_deinit_request(PVulkanRequest *vulkan_request)
{
	e_dynarr_deinit(vulkan_request->required_extensions);
	e_dynarr_deinit(vulkan_request->optional_extensions);
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
 * p_vulkan_extension_exists
 *
 * returns true if the vulkan_extension is found
 */
bool p_vulkan_extension_exists(char *vulkan_extension)
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
	return false;
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
 * p_vulkan_find_queue_family_info
 *
 * finds the queue family queue_flags of the current_physical_device and returns it
 */
static PVulkanQueueFamilyInfo p_vulkan_find_queue_family_info(
		const VkPhysicalDevice device,
		const VkQueueFlagBits queue_flag)
{
	PVulkanQueueFamilyInfo queue_family_info = {0};
	queue_family_info.flag = queue_flag;

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);
	VkQueueFamilyProperties *queue_families = malloc(queue_family_count * sizeof *queue_families);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

	for (uint i = 0; i < queue_family_count; i++)
	{
		if (queue_families[i].queueFlags & queue_flag)
		{
			queue_family_info.index = i;
			queue_family_info.flag = queue_flag;
			queue_family_info.exists = true;
			break;
		}
	}
	free(queue_families);
	return queue_family_info;
}

/**
 * p_vulkan_enable_features
 *
 * returns the required and optional features that exist
 * if a required feature does not exist it returns a 0 feature
 */
VkPhysicalDeviceFeatures p_vulkan_enable_features(PVulkanRequest *vulkan_request, VkPhysicalDevice device)
{
	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures(device, &device_features);

	VkPhysicalDeviceFeatures enabled_features = {0};

	VkBool32 contains_required_features = true;
	if (vulkan_request->required_features.robustBufferAccess) contains_required_features &= device_features.robustBufferAccess;
	if (vulkan_request->required_features.fullDrawIndexUint32) contains_required_features &= device_features.fullDrawIndexUint32;
	if (vulkan_request->required_features.imageCubeArray) contains_required_features &= device_features.imageCubeArray;
	if (vulkan_request->required_features.independentBlend) contains_required_features &= device_features.independentBlend;
	if (vulkan_request->required_features.geometryShader) contains_required_features &= device_features.geometryShader;
	if (vulkan_request->required_features.tessellationShader) contains_required_features &= device_features.tessellationShader;
	if (vulkan_request->required_features.sampleRateShading) contains_required_features &= device_features.sampleRateShading;
	if (vulkan_request->required_features.dualSrcBlend) contains_required_features &= device_features.dualSrcBlend;
	if (vulkan_request->required_features.logicOp) contains_required_features &= device_features.logicOp;
	if (vulkan_request->required_features.multiDrawIndirect) contains_required_features &= device_features.multiDrawIndirect;
	if (vulkan_request->required_features.drawIndirectFirstInstance) contains_required_features &= device_features.drawIndirectFirstInstance;
	if (vulkan_request->required_features.depthClamp) contains_required_features &= device_features.depthClamp;
	if (vulkan_request->required_features.depthBiasClamp) contains_required_features &= device_features.depthBiasClamp;
	if (vulkan_request->required_features.fillModeNonSolid) contains_required_features &= device_features.fillModeNonSolid;
	if (vulkan_request->required_features.depthBounds) contains_required_features &= device_features.depthBounds;
	if (vulkan_request->required_features.wideLines) contains_required_features &= device_features.wideLines;
	if (vulkan_request->required_features.largePoints) contains_required_features &= device_features.largePoints;
	if (vulkan_request->required_features.alphaToOne) contains_required_features &= device_features.alphaToOne;
	if (vulkan_request->required_features.multiViewport) contains_required_features &= device_features.multiViewport;
	if (vulkan_request->required_features.samplerAnisotropy) contains_required_features &= device_features.samplerAnisotropy;
	if (vulkan_request->required_features.textureCompressionETC2) contains_required_features &= device_features.textureCompressionETC2;
	if (vulkan_request->required_features.textureCompressionASTC_LDR) contains_required_features &= device_features.textureCompressionASTC_LDR;
	if (vulkan_request->required_features.textureCompressionBC) contains_required_features &= device_features.textureCompressionBC;
	if (vulkan_request->required_features.occlusionQueryPrecise) contains_required_features &= device_features.occlusionQueryPrecise;
	if (vulkan_request->required_features.pipelineStatisticsQuery) contains_required_features &= device_features.pipelineStatisticsQuery;
	if (vulkan_request->required_features.vertexPipelineStoresAndAtomics) contains_required_features &= device_features.vertexPipelineStoresAndAtomics;
	if (vulkan_request->required_features.fragmentStoresAndAtomics) contains_required_features &= device_features.fragmentStoresAndAtomics;
	if (vulkan_request->required_features.shaderTessellationAndGeometryPointSize) contains_required_features &= device_features.shaderTessellationAndGeometryPointSize;
	if (vulkan_request->required_features.shaderImageGatherExtended) contains_required_features &= device_features.shaderImageGatherExtended;
	if (vulkan_request->required_features.shaderStorageImageExtendedFormats) contains_required_features &= device_features.shaderStorageImageExtendedFormats;
	if (vulkan_request->required_features.shaderStorageImageMultisample) contains_required_features &= device_features.shaderStorageImageMultisample;
	if (vulkan_request->required_features.shaderStorageImageReadWithoutFormat) contains_required_features &= device_features.shaderStorageImageReadWithoutFormat;
	if (vulkan_request->required_features.shaderStorageImageWriteWithoutFormat) contains_required_features &= device_features.shaderStorageImageWriteWithoutFormat;
	if (vulkan_request->required_features.shaderUniformBufferArrayDynamicIndexing) contains_required_features &= device_features.shaderUniformBufferArrayDynamicIndexing;
	if (vulkan_request->required_features.shaderSampledImageArrayDynamicIndexing) contains_required_features &= device_features.shaderSampledImageArrayDynamicIndexing;
	if (vulkan_request->required_features.shaderStorageBufferArrayDynamicIndexing) contains_required_features &= device_features.shaderStorageBufferArrayDynamicIndexing;
	if (vulkan_request->required_features.shaderStorageImageArrayDynamicIndexing) contains_required_features &= device_features.shaderStorageImageArrayDynamicIndexing;
	if (vulkan_request->required_features.shaderClipDistance) contains_required_features &= device_features.shaderClipDistance;
	if (vulkan_request->required_features.shaderCullDistance) contains_required_features &= device_features.shaderCullDistance;
	if (vulkan_request->required_features.shaderFloat64) contains_required_features &= device_features.shaderFloat64;
	if (vulkan_request->required_features.shaderInt64) contains_required_features &= device_features.shaderInt64;
	if (vulkan_request->required_features.shaderInt16) contains_required_features &= device_features.shaderInt16;
	if (vulkan_request->required_features.shaderResourceResidency) contains_required_features &= device_features.shaderResourceResidency;
	if (vulkan_request->required_features.shaderResourceMinLod) contains_required_features &= device_features.shaderResourceMinLod;
	if (vulkan_request->required_features.sparseBinding) contains_required_features &= device_features.sparseBinding;
	if (vulkan_request->required_features.sparseResidencyBuffer) contains_required_features &= device_features.sparseResidencyBuffer;
	if (vulkan_request->required_features.sparseResidencyImage2D) contains_required_features &= device_features.sparseResidencyImage2D;
	if (vulkan_request->required_features.sparseResidencyImage3D) contains_required_features &= device_features.sparseResidencyImage3D;
	if (vulkan_request->required_features.sparseResidency2Samples) contains_required_features &= device_features.sparseResidency2Samples;
	if (vulkan_request->required_features.sparseResidency4Samples) contains_required_features &= device_features.sparseResidency4Samples;
	if (vulkan_request->required_features.sparseResidency8Samples) contains_required_features &= device_features.sparseResidency8Samples;
	if (vulkan_request->required_features.sparseResidency16Samples) contains_required_features &= device_features.sparseResidency16Samples;
	if (vulkan_request->required_features.sparseResidencyAliased) contains_required_features &= device_features.sparseResidencyAliased;
	if (vulkan_request->required_features.variableMultisampleRate) contains_required_features &= device_features.variableMultisampleRate;
	if (vulkan_request->required_features.inheritedQueries) contains_required_features &= device_features.inheritedQueries;
	if (!contains_required_features)
	{
		return enabled_features;
	} else {
		enabled_features = vulkan_request->required_features;
	}

	// optional features
	if (vulkan_request->optional_features.robustBufferAccess) enabled_features.robustBufferAccess = true;
	if (vulkan_request->optional_features.fullDrawIndexUint32) enabled_features.fullDrawIndexUint32 = true;
	if (vulkan_request->optional_features.imageCubeArray) enabled_features.imageCubeArray = true;
	if (vulkan_request->optional_features.independentBlend) enabled_features.independentBlend = true;
	if (vulkan_request->optional_features.geometryShader) enabled_features.geometryShader = true;
	if (vulkan_request->optional_features.tessellationShader) enabled_features.tessellationShader = true;
	if (vulkan_request->optional_features.sampleRateShading) enabled_features.sampleRateShading = true;
	if (vulkan_request->optional_features.dualSrcBlend) enabled_features.dualSrcBlend = true;
	if (vulkan_request->optional_features.logicOp) enabled_features.logicOp = true;
	if (vulkan_request->optional_features.multiDrawIndirect) enabled_features.multiDrawIndirect = true;
	if (vulkan_request->optional_features.drawIndirectFirstInstance) enabled_features.drawIndirectFirstInstance = true;
	if (vulkan_request->optional_features.depthClamp) enabled_features.depthClamp = true;
	if (vulkan_request->optional_features.depthBiasClamp) enabled_features.depthBiasClamp = true;
	if (vulkan_request->optional_features.fillModeNonSolid) enabled_features.fillModeNonSolid = true;
	if (vulkan_request->optional_features.depthBounds) enabled_features.depthBounds = true;
	if (vulkan_request->optional_features.wideLines) enabled_features.wideLines = true;
	if (vulkan_request->optional_features.largePoints) enabled_features.largePoints = true;
	if (vulkan_request->optional_features.alphaToOne) enabled_features.alphaToOne = true;
	if (vulkan_request->optional_features.multiViewport) enabled_features.multiViewport = true;
	if (vulkan_request->optional_features.samplerAnisotropy) enabled_features.samplerAnisotropy = true;
	if (vulkan_request->optional_features.textureCompressionETC2) enabled_features.textureCompressionETC2 = true;
	if (vulkan_request->optional_features.textureCompressionASTC_LDR) enabled_features.textureCompressionASTC_LDR = true;
	if (vulkan_request->optional_features.textureCompressionBC) enabled_features.textureCompressionBC = true;
	if (vulkan_request->optional_features.occlusionQueryPrecise) enabled_features.occlusionQueryPrecise = true;
	if (vulkan_request->optional_features.pipelineStatisticsQuery) enabled_features.pipelineStatisticsQuery = true;
	if (vulkan_request->optional_features.vertexPipelineStoresAndAtomics) enabled_features.vertexPipelineStoresAndAtomics = true;
	if (vulkan_request->optional_features.fragmentStoresAndAtomics) enabled_features.fragmentStoresAndAtomics = true;
	if (vulkan_request->optional_features.shaderTessellationAndGeometryPointSize) enabled_features.shaderTessellationAndGeometryPointSize = true;
	if (vulkan_request->optional_features.shaderImageGatherExtended) enabled_features.shaderImageGatherExtended = true;
	if (vulkan_request->optional_features.shaderStorageImageExtendedFormats) enabled_features.shaderStorageImageExtendedFormats = true;
	if (vulkan_request->optional_features.shaderStorageImageMultisample) enabled_features.shaderStorageImageMultisample = true;
	if (vulkan_request->optional_features.shaderStorageImageReadWithoutFormat) enabled_features.shaderStorageImageReadWithoutFormat = true;
	if (vulkan_request->optional_features.shaderStorageImageWriteWithoutFormat) enabled_features.shaderStorageImageWriteWithoutFormat = true;
	if (vulkan_request->optional_features.shaderUniformBufferArrayDynamicIndexing) enabled_features.shaderUniformBufferArrayDynamicIndexing = true;
	if (vulkan_request->optional_features.shaderSampledImageArrayDynamicIndexing) enabled_features.shaderSampledImageArrayDynamicIndexing = true;
	if (vulkan_request->optional_features.shaderStorageBufferArrayDynamicIndexing) enabled_features.shaderStorageBufferArrayDynamicIndexing = true;
	if (vulkan_request->optional_features.shaderStorageImageArrayDynamicIndexing) enabled_features.shaderStorageImageArrayDynamicIndexing = true;
	if (vulkan_request->optional_features.shaderClipDistance) enabled_features.shaderClipDistance = true;
	if (vulkan_request->optional_features.shaderCullDistance) enabled_features.shaderCullDistance = true;
	if (vulkan_request->optional_features.shaderFloat64) enabled_features.shaderFloat64 = true;
	if (vulkan_request->optional_features.shaderInt64) enabled_features.shaderInt64 = true;
	if (vulkan_request->optional_features.shaderInt16) enabled_features.shaderInt16 = true;
	if (vulkan_request->optional_features.shaderResourceResidency) enabled_features.shaderResourceResidency = true;
	if (vulkan_request->optional_features.shaderResourceMinLod) enabled_features.shaderResourceMinLod = true;
	if (vulkan_request->optional_features.sparseBinding) enabled_features.sparseBinding = true;
	if (vulkan_request->optional_features.sparseResidencyBuffer) enabled_features.sparseResidencyBuffer = true;
	if (vulkan_request->optional_features.sparseResidencyImage2D) enabled_features.sparseResidencyImage2D = true;
	if (vulkan_request->optional_features.sparseResidencyImage3D) enabled_features.sparseResidencyImage3D = true;
	if (vulkan_request->optional_features.sparseResidency2Samples) enabled_features.sparseResidency2Samples = true;
	if (vulkan_request->optional_features.sparseResidency4Samples) enabled_features.sparseResidency4Samples = true;
	if (vulkan_request->optional_features.sparseResidency8Samples) enabled_features.sparseResidency8Samples = true;
	if (vulkan_request->optional_features.sparseResidency16Samples) enabled_features.sparseResidency16Samples = true;
	if (vulkan_request->optional_features.sparseResidencyAliased) enabled_features.sparseResidencyAliased = true;
	if (vulkan_request->optional_features.variableMultisampleRate) enabled_features.variableMultisampleRate = true;
	if (vulkan_request->optional_features.inheritedQueries) enabled_features.inheritedQueries = true;

	return enabled_features;
}

/**
 * p_vulkan_enable_queue_flags
 *
 * returns the required and optional queue flags that exist
 * if a required queue flag does not exist it returns 0
 */
VkQueueFlags p_vulkan_enable_queue_flags(PVulkanRequest *vulkan_request, VkPhysicalDevice device)
{
	VkQueueFlags queue_flags = 0ULL;

	// evaluate optional and required queue flags
	for (VkQueueFlags flag = 1ULL; flag < VK_QUEUE_FLAG_BITS_MAX_ENUM; flag <<= 1)
	{
		PVulkanQueueFamilyInfo queue_family_info = p_vulkan_find_queue_family_info(device, flag);
		if ((flag & vulkan_request->optional_queue_flags) && queue_family_info.exists)
			queue_flags |= flag;

		if ((flag & vulkan_request->required_queue_flags) && !queue_family_info.exists)
		{
			queue_flags = 0ULL;
			break;
		}
	}
	queue_flags |= vulkan_request->required_queue_flags;
	return queue_flags;
}

/**
 * p_vulkan_set_device
 *
 * sets the physical device to use in vulkan_data from physical_device
 * also creates a logical device with vulkan_request and sets it to vulkan_data
 */
PHANTOM_API void p_vulkan_set_device(PVulkanData *vulkan_data, PVulkanRequest *vulkan_request,
		VkPhysicalDevice physical_device)
{
	if (e_dynarr_find(vulkan_data->compatible_devices, &physical_device) == -1)
	{
		e_log_message(E_LOG_ERROR, L"Vulkan General", L"Selected device is not compatible");
		exit(1);
	}

	// Set physical device
	vulkan_data->current_physical_device = physical_device;

	// Set queue family indices
	if (vulkan_data->queue_family_info != NULL)
		e_dynarr_deinit(vulkan_data->queue_family_info);
	vulkan_data->queue_family_info = e_dynarr_init(sizeof(PVulkanQueueFamilyInfo), 1);

	VkQueueFlags enabled_queue_flags = p_vulkan_enable_queue_flags(vulkan_request, physical_device);
	for (VkQueueFlags flag = 1ULL; flag < VK_QUEUE_FLAG_BITS_MAX_ENUM; flag <<= 1)
	{
		if (flag & enabled_queue_flags)
		{
			PVulkanQueueFamilyInfo *queue_family_info = malloc(sizeof *queue_family_info);
			PVulkanQueueFamilyInfo temp_queue_family_info = p_vulkan_find_queue_family_info(physical_device, flag);
			memcpy(queue_family_info, &temp_queue_family_info, sizeof temp_queue_family_info);
			e_dynarr_add(vulkan_data->queue_family_info, queue_family_info);
			free(queue_family_info);
		}
	}
	uint num_queue_families = vulkan_data->queue_family_info->num_items;
	VkDeviceQueueCreateInfo queue_create_infos[num_queue_families];
	for (uint i = 0; i < vulkan_data->queue_family_info->num_items; i++)
	{
		queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[i].queueFamilyIndex = E_DYNARR_GET(vulkan_data->queue_family_info, PVulkanQueueFamilyInfo, i).index;
		queue_create_infos[i].queueCount = 1;
		queue_create_infos[i].pQueuePriorities = E_PTR_FROM_VALUE(float, 1.f);
	}


	// TODO: finish me
	VkPhysicalDeviceFeatures device_features;
	VkDeviceCreateInfo device_create_info;
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
}

/**
 * p_vulkan_pick_physical_device
 *
 * picks the physical device for use with vulkan and adds it to vulkan_data
 */
static void p_vulkan_auto_pick_physical_device(PVulkanData *vulkan_data, PVulkanRequest *vulkan_request)
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
	for (uint i = 0; i < compatible_devices->num_items; i++) {
		int score = 0;
		VkPhysicalDevice device = E_DYNARR_GET(compatible_devices, VkPhysicalDevice, i);
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(device, &device_properties);

		// lots of score goes to discrete gpus
		if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score = 1000;

		// Maximum possible size of textures affects graphics quality
		score += device_properties.limits.maxImageDimension2D;

		// evaluate optional and required queue flags
		VkQueueFlags enabled_queue_flags = p_vulkan_enable_queue_flags(vulkan_request, device);
		if (enabled_queue_flags == 0ULL)
		{
			score = -1;
			goto end_device_score_eval;
		} else {
			score += 100*e_count_set_bits(enabled_queue_flags);
		}

		// evaluate optional and required features
		VkPhysicalDeviceFeatures enabled_features = p_vulkan_enable_features(vulkan_request, device);
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
		p_vulkan_set_device(vulkan_data, vulkan_request,
				E_DYNARR_GET(compatible_devices, VkPhysicalDevice, max_score_index));
	}

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
PVulkanData *p_vulkan_init(PVulkanRequest *vulkan_request)
{
	PVulkanData *vulkan_data = malloc(sizeof *vulkan_data);
	vulkan_data->queue_family_info = NULL;

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

	// Add all required extensions and optional extensions that exist
	EDynarr *enabled_extensions = e_dynarr_init_arr(sizeof (char *), vulkan_request->required_extensions->num_items,
			vulkan_request->required_extensions->arr);
	for (uint i = 0; i < vulkan_request->optional_extensions->num_items; i++)
	{
		char *extension = E_DYNARR_GET(vulkan_request->optional_extensions, char *, i);
		if(p_vulkan_extension_exists(extension))
			e_dynarr_add(enabled_extensions, extension);
	}

	vk_instance_create_info.enabledExtensionCount = enabled_extensions->num_items;
	vk_instance_create_info.ppEnabledExtensionNames = enabled_extensions->arr;

#ifdef PHANTOM_DEBUG_VULKAN
	VkDebugUtilsMessengerCreateInfoEXT vk_debug_utils_messenger_create_info =
		p_vulkan_init_debug_messenger(&vk_instance_create_info);
	p_vulkan_create_instance(vulkan_data, vk_instance_create_info);
	e_dynarr_deinit(enabled_extensions);

	if (p_vulkan_create_debug_utils_messenger(vulkan_data->instance, &vk_debug_utils_messenger_create_info, NULL,
				&vulkan_data->debug_messenger) != VK_SUCCESS) {
		e_log_message(E_LOG_ERROR, L"Vulkan General", L"Failed to set up debug messenger!");
		exit(1);
	}
#else
	vk_instance_create_info.enabledLayerCount = 0;
	vk_instance_create_info.pNext = NULL;
	p_vulkan_create_instance(vulkan_data, vk_instance_create_info);
	e_dynarr_deinit(enabled_extensions);
#endif // PHANTOM_DEBUG_VULKAN

	p_vulkan_auto_pick_physical_device(vulkan_data, vulkan_request);

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

	e_dynarr_deinit(vulkan_data->queue_family_info);
	e_dynarr_deinit(vulkan_data->compatible_devices);
	vkDestroyInstance(vulkan_data->instance, NULL);
	free(vulkan_data);
}
