#include "VkCtx.h"
#include <Utility.h>

#include <stdexcept>
#include <vector>
#include <cstring>
#include <iostream>
#include <set>

static VkCtx* s_Ctx;

static std::vector<const char*> layers = {
	"VK_LAYER_KHRONOS_validation"
};

static std::vector<const char*> deviceExts = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::string msg = pCallbackData->pMessage;
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		INFO(("From the validation layer :- " + msg));
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		WARN(("From the validation layer :- " + msg));
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		ERROR(("From the validation layer :- " + msg));
		break;
	default:
		std::cerr << "From the validation layer :- " << pCallbackData->pMessage << "\n\n";
	}

	return VK_FALSE;
}

static bool IsPhysicalDeviceUsable(VkPhysicalDevice& device)
{
	bool queuePresent = [&]() {
		uint32_t count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
		std::vector<VkQueueFamilyProperties> props(count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &count, props.data());

		int i = 0;
		for (const auto& prop : props)
		{
			if (prop.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				s_Ctx->queueProps.gQueueIdx = i;

			if (prop.queueFlags & VK_QUEUE_TRANSFER_BIT)
				s_Ctx->queueProps.tQueueIdx = i;

			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, s_Ctx->surface, &presentSupport);

			if (presentSupport)
				s_Ctx->queueProps.pQueueIdx = i;

			if (s_Ctx->queueProps.IsFull())
				return true;

			i++;
		}

		return false;
	} ();

	bool deviceExtsSupported = [&]() {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
		
		std::set<std::string> requiredExtensions(deviceExts.begin(), deviceExts.end());
		
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}
		
		return requiredExtensions.empty();
	} ();

	return queuePresent && deviceExtsSupported;
}

void VkCtxHandler::InitCtx(Window& window)
{
	if (!s_Ctx)
	{
		std::string fileName = __FILE__;
		std::string msg = "There is no current Vulkan Backend Context! (File: " + fileName + ", Line: " + std::to_string(__LINE__) + ")";
		FATAL(msg)
	}

#if defined(_DEBUG) || !defined(NDEBUG)

	// Checking if validation layers are supported
	{
		bool isSupported = [&]() {
			uint32_t lyrCount = 0;
			vkEnumerateInstanceLayerProperties(&lyrCount, nullptr);
			std::vector<VkLayerProperties> props(lyrCount);
			vkEnumerateInstanceLayerProperties(&lyrCount, props.data());

			bool found = false;
			for (const auto layer : layers)
			{
				for (const auto& prop : props)
				{
					if (strcmp(prop.layerName, layer) == 0)
					{
						found = true;
						break;
					}
				}
			}

			if (!found)
				return true;

			return true;
		} ();

		if (!isSupported)
			FATAL("Validation layers are not supported!")
	}
#endif

	// Creating Instance
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "VulkanApp";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "VulkanAppEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.pApplicationInfo = &appInfo;

		std::vector<const char*> exts;
		// Getting the required Extensions
		{
			uint32_t extCount = 0;
			auto e = glfwGetRequiredInstanceExtensions(&extCount);
			std::vector<const char*> es(e, extCount + e);
#if defined(_DEBUG) || !defined(NDEBUG)
			es.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			info.enabledLayerCount = (uint32_t)layers.size();
			info.ppEnabledLayerNames = layers.data();
#endif
			exts = es;
		}

		info.enabledExtensionCount = (uint32_t)exts.size();
		info.ppEnabledExtensionNames = exts.data();

		VK_CHECK(vkCreateInstance(&info, nullptr, &s_Ctx->instance))
	}

#if defined(_DEBUG) || !defined(NDEBUG)

	// Creating the debug messenger
	{
		VkDebugUtilsMessengerCreateInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = DebugMessengerCallback;

		auto createFunc = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Ctx->instance, "vkCreateDebugUtilsMessengerEXT");
		if (createFunc)
			createFunc(s_Ctx->instance, &info, nullptr, &s_Ctx->debugger);
	}
#endif

	// Creating the window surface
	{
		VK_CHECK(glfwCreateWindowSurface(s_Ctx->instance, window.GetInternHandle(), nullptr, &s_Ctx->surface))
	}

	// Selecting a suitable physical device
	{
		uint32_t count = 0;
		vkEnumeratePhysicalDevices(s_Ctx->instance, &count, nullptr);
		std::vector<VkPhysicalDevice> devices(count);
		vkEnumeratePhysicalDevices(s_Ctx->instance, &count, devices.data());

		if (!count)
			FATAL("Failed to find any device on the current PC!")

		for (auto& device : devices)
		{
			bool supported = IsPhysicalDeviceUsable(device);

			if (supported)
			{
				s_Ctx->physicalDevice = device;
				break;
			}
		}
	}

	// Creating the logical device
	{
		VkPhysicalDeviceFeatures features{};
		vkGetPhysicalDeviceFeatures(s_Ctx->physicalDevice, &features);

		float qPriority = 1.0f;
		VkDeviceQueueCreateInfo qInfo[3];
		uint32_t familiesIdxs[] = {
			s_Ctx->queueProps.gQueueIdx,
			s_Ctx->queueProps.pQueueIdx,
			s_Ctx->queueProps.tQueueIdx
		};

		for (int i = 0; i < 3; i++)
		{
			VkDeviceQueueCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			info.pQueuePriorities = &qPriority;
			info.queueCount = 1;
			info.queueFamilyIndex = familiesIdxs[i];

			qInfo[i] = info;
		}

		VkDeviceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		info.pEnabledFeatures = &features;
		info.queueCreateInfoCount = 3;
		info.pQueueCreateInfos = qInfo;
		info.enabledExtensionCount = (uint32_t)deviceExts.size();
		info.ppEnabledExtensionNames = deviceExts.data();
		info.enabledLayerCount = 0;

#if defined(_DEBUG) || !defined(NDEBUG)
		info.enabledLayerCount = (uint32_t)layers.size();
		info.ppEnabledLayerNames = layers.data();
#endif

		VK_CHECK(vkCreateDevice(s_Ctx->physicalDevice, &info, nullptr, &s_Ctx->device))
	}
}

void VkCtxHandler::DestroyCtx()
{
	if (!s_Ctx)
	{
		std::string fileName = __FILE__;
		std::string msg = "There is no current Vulkan Backend Context! (File: " + fileName + ", Line: " + std::to_string(__LINE__) + ")";
		FATAL(msg)
	}

	vkDestroyDevice(s_Ctx->device, nullptr);
	vkDestroySurfaceKHR(s_Ctx->instance, s_Ctx->surface, nullptr);

#if defined(_DEBUG) || !defined(NDEBUG)
	auto destroyFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Ctx->instance, "vkDestroyDebugUtilsMessengerEXT");
	if (destroyFunc)
		destroyFunc(s_Ctx->instance, s_Ctx->debugger, nullptr);
#endif

	vkDestroyInstance(s_Ctx->instance, nullptr);
}

void VkCtxHandler::SetCrntCtx(VkCtx& ctx)
{
	s_Ctx = &ctx;
}

VkCtx* VkCtxHandler::GetCrntCtx()
{
	return s_Ctx;
}