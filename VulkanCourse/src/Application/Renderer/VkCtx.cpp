#include "VkCtx.h"
#include <Utility.h>

#include <stdexcept>
#include <cstring>
#include <iostream>
#include <set>

#include <glm/glm.hpp>

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
			void* pUserData)
{

	std::string msg = pCallbackData->pMessage;
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		INFO(("From the validation layer :- " + msg + "\n\n"));
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		WARN(("From the validation layer :- " + msg + "\n\n"));
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		ERROR(("From the validation layer :- " + msg + "\n\n"));
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
		VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr))

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()))
		
		std::set<std::string> requiredExtensions(deviceExts.begin(), deviceExts.end());
		
		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}
		
		return requiredExtensions.empty();
	} ();

	return queuePresent && deviceExtsSupported;
}

static void GetScCaps()
{
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_Ctx->physicalDevice, s_Ctx->surface, &s_Ctx->scCaps.caps))

	uint32_t formatCount = 0;
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(s_Ctx->physicalDevice, s_Ctx->surface, &formatCount, nullptr))
	if (formatCount)
	{
		s_Ctx->scCaps.formats.resize(formatCount);
		VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(s_Ctx->physicalDevice, s_Ctx->surface, &formatCount, s_Ctx->scCaps.formats.data()))
	}
	
	uint32_t modeCount = 0;
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(s_Ctx->physicalDevice, s_Ctx->surface, &modeCount, nullptr))
	if (modeCount)
	{
		s_Ctx->scCaps.modes.resize(modeCount);
		VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(s_Ctx->physicalDevice, s_Ctx->surface, &modeCount, s_Ctx->scCaps.modes.data()))
	}
}

static void SelectScProps(Window& window)
{
	// Selecting the present mode
	{
		bool set = false;
		for (const auto& mode : s_Ctx->scCaps.modes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				s_Ctx->scMode = mode;
				set = true;
				break;
			}
		}

		if (!set)
			s_Ctx->scMode = VK_PRESENT_MODE_FIFO_KHR;
	}
	// Selecting the surface format
	{
		bool set = false;
		for (const auto& format : s_Ctx->scCaps.formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			{
				s_Ctx->scFormat = format;
				set = true;
				break;
			}
		}
		if (!set)
			s_Ctx->scFormat = s_Ctx->scCaps.formats[0];
	}
	// Selecting the extent
	{
		if (s_Ctx->scCaps.caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
			s_Ctx->scExtent = s_Ctx->scCaps.caps.currentExtent;
		else
		{
			int width, height;
			glfwGetFramebufferSize(window.GetInternHandle(), &width, &height);

			s_Ctx->scExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			s_Ctx->scExtent.width = glm::clamp(s_Ctx->scExtent.width, s_Ctx->scCaps.caps.minImageExtent.width, s_Ctx->scCaps.caps.maxImageExtent.width);
			s_Ctx->scExtent.height = glm::clamp(s_Ctx->scExtent.height, s_Ctx->scCaps.caps.minImageExtent.height, s_Ctx->scCaps.caps.maxImageExtent.height);
		}
	}
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
			VK_CHECK(vkEnumerateInstanceLayerProperties(&lyrCount, nullptr))
			std::vector<VkLayerProperties> props(lyrCount);
			VK_CHECK(vkEnumerateInstanceLayerProperties(&lyrCount, props.data()))

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
			VkDebugUtilsMessengerCreateInfoEXT debinfo{};
			debinfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debinfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debinfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debinfo.pfnUserCallback = DebugMessengerCallback;

			es.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			info.enabledLayerCount = (uint32_t)layers.size();
			info.ppEnabledLayerNames = layers.data();
			info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debinfo;
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
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = DebugMessengerCallback;

		auto createFunc = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Ctx->instance, "vkCreateDebugUtilsMessengerEXT");
		if (createFunc)
			VK_CHECK(createFunc(s_Ctx->instance, &info, nullptr, &s_Ctx->debugger))
	}
#endif

		// Creating the window surface
	{
		VK_CHECK(glfwCreateWindowSurface(s_Ctx->instance, window.GetInternHandle(), nullptr, &s_Ctx->surface))
	}

	// Selecting a suitable physical device
	{
		uint32_t count = 0;
		VK_CHECK(vkEnumeratePhysicalDevices(s_Ctx->instance, &count, nullptr))
		std::vector<VkPhysicalDevice> devices(count);
		VK_CHECK(vkEnumeratePhysicalDevices(s_Ctx->instance, &count, devices.data()))

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
		uint32_t familiesIdxs[] = {
			s_Ctx->queueProps.gQueueIdx,
			s_Ctx->queueProps.pQueueIdx,
			s_Ctx->queueProps.tQueueIdx
		};

		int loopCount = 3;
		if (familiesIdxs[0] == familiesIdxs[1])
			loopCount--;
		if (familiesIdxs[1] == familiesIdxs[2])
			loopCount--;
		if (familiesIdxs[0] == familiesIdxs[2])
			loopCount--;

		if (loopCount == 0)
			loopCount++;


		std::vector<VkDeviceQueueCreateInfo> qInfo(loopCount);
		for (int i = 0; i < loopCount; i++)
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
		info.queueCreateInfoCount = loopCount;
		info.pQueueCreateInfos = qInfo.data();
		info.enabledExtensionCount = (uint32_t)deviceExts.size();
		info.ppEnabledExtensionNames = deviceExts.data();
		info.enabledLayerCount = 0;

#if defined(_DEBUG) || !defined(NDEBUG)
		info.enabledLayerCount = (uint32_t)layers.size();
		info.ppEnabledLayerNames = layers.data();
#endif

		VK_CHECK(vkCreateDevice(s_Ctx->physicalDevice, &info, nullptr, &s_Ctx->device))
	}
	// Retrieving queue objects 
	{
		vkGetDeviceQueue(s_Ctx->device, s_Ctx->queueProps.gQueueIdx, 0, &s_Ctx->gQueue);
		vkGetDeviceQueue(s_Ctx->device, s_Ctx->queueProps.pQueueIdx, 0, &s_Ctx->pQueue);
		vkGetDeviceQueue(s_Ctx->device, s_Ctx->queueProps.tQueueIdx, 0, &s_Ctx->tQueue);
	}

	GetScCaps(); // Retrieving the various surface's rendering capabilities
	SelectScProps(window); // Selecting the most appropriate present modes, formats, extent for the swapchain

	// Creating the swapchain
	{
		auto caps = s_Ctx->scCaps.caps;

		uint32_t imgCount = caps.minImageCount + 1;
		if (caps.maxImageCount > 0 && imgCount > caps.maxImageCount)
		{
			imgCount = caps.maxImageCount;
		}

		VkSwapchainCreateInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.surface = s_Ctx->surface;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.minImageCount = imgCount;
		info.imageFormat = s_Ctx->scFormat.format;
		info.imageColorSpace = s_Ctx->scFormat.colorSpace;
		info.preTransform = caps.currentTransform;
		info.presentMode = s_Ctx->scMode;
		info.imageArrayLayers = 1;
		info.imageExtent = s_Ctx->scExtent;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		info.clipped = VK_TRUE;
		info.oldSwapchain = VK_NULL_HANDLE;

		uint32_t familiesIdxs[] = {
			s_Ctx->queueProps.gQueueIdx,
			s_Ctx->queueProps.pQueueIdx,
			s_Ctx->queueProps.tQueueIdx
		};

		if (familiesIdxs[0] != familiesIdxs[1] || familiesIdxs[1] != familiesIdxs[2] || familiesIdxs[0] != familiesIdxs[2])
		{
			info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			info.queueFamilyIndexCount = 3;
			info.pQueueFamilyIndices = familiesIdxs;
		}
		else
		{
			info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		VK_CHECK(vkCreateSwapchainKHR(s_Ctx->device, &info, nullptr, &s_Ctx->swapchain))
	}
	// Retrieving the swapchain images
	{
		uint32_t count = 0;
		VK_CHECK(vkGetSwapchainImagesKHR(s_Ctx->device, s_Ctx->swapchain, &count, nullptr))
		s_Ctx->scImgs.resize(count);
		VK_CHECK(vkGetSwapchainImagesKHR(s_Ctx->device, s_Ctx->swapchain, &count, s_Ctx->scImgs.data()))
	}
	// Creating the image views
	{
		s_Ctx->scImgViews.resize(s_Ctx->scImgs.size());
		int i = 0;
		for (const auto& img : s_Ctx->scImgs)
		{
			VkImageViewCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.image = img;
			info.components = {
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY
			};
			info.format = s_Ctx->scFormat.format;
			info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			info.subresourceRange.baseArrayLayer = 0;
			info.subresourceRange.baseMipLevel = 0;
			info.subresourceRange.layerCount = 1;
			info.subresourceRange.levelCount = 1;

			VK_CHECK(vkCreateImageView(s_Ctx->device, &info, nullptr, &s_Ctx->scImgViews[i]))

			i++;
		}
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

	for (const auto& imgView : s_Ctx->scImgViews)
	{
		vkDestroyImageView(s_Ctx->device, imgView, nullptr);
	}

	vkDestroySwapchainKHR(s_Ctx->device, s_Ctx->swapchain, nullptr);
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