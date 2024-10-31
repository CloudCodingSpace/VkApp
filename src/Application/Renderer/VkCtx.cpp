#include "VkCtx.h"
#include <Utility.h>

#include <stdexcept>
#include <vector>
#include <cstring>

static VkCtx* s_Ctx;

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		INFO("From the validation layer :- %s", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		WARN("From the validation layer :- %s", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		ERROR("From the validation layer :- %s", pCallbackData->pMessage);
		break;
	default:
		ERROR("From the validation layer :- %s", pCallbackData->pMessage);
		break;
	}

	return VK_FALSE;
}

void VkCtxHandler::InitCtx(Window& window)
{
	if (!s_Ctx)
		FATAL("There is no current Vulkan Backend Context! File: %s, Line: %d", __FILE__, __LINE__)

	std::vector<const char*> layers = {
		"VK_LAYER_KHRONOS_validation"
	};

#if defined(_DEBUG) || !defined(NDEBUG)
	// Checking if validation layers are supported
	{
		bool isSupported = [&, layers]() {
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

		if(!isSupported)
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

		auto createFunc = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Ctx->instance, "vkCreateDebugUtilsMesengerEXT");
		if (createFunc)
			createFunc(s_Ctx->instance, &info, nullptr, &s_Ctx->debugger);
	}
#endif
}

void VkCtxHandler::DestroyCtx()
{
	if (!s_Ctx)
		FATAL("There is no current Vulkan Backend Context! File: %s, Line: %d", __FILE__, __LINE__)

#if defined(_DEBUG) || !defined(NDEBUG)
	auto destroyFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Ctx->instance, "vkDestroyDebugUtilsMesengerEXT");
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
