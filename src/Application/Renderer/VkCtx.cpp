#include "VkCtx.h"

#include <stdexcept>
#include <Utility.h>


void VkCtxHelper::InitCtx(VkCtx& ctx, Window& window)
{
	// Creating Instance
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "VulkanApp";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "VulkanAppEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		uint32_t extCount = 0;
		const char** exts = glfwGetRequiredInstanceExtensions(&extCount);

		VkInstanceCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.enabledExtensionCount = extCount;
		info.ppEnabledExtensionNames = exts;
		info.enabledLayerCount = 0;
		info.pApplicationInfo = &appInfo;
		
		VK_CHECK(vkCreateInstance(&info, nullptr, &ctx.instance))
	}
}

void VkCtxHelper::DestroyCtx(VkCtx& ctx)
{
	vkDestroyInstance(ctx.instance, nullptr);
}
