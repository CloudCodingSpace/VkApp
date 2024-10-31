#pragma once

#include <vulkan/vulkan.h>

#include <Window/Window.h>

struct VkCtx
{
	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
#if defined(_DEBUG) || !defined(N_DEBUG)
	VkDebugUtilsMessengerEXT debugger;
#endif
};

class VkCtxHandler
{
public:
	static void InitCtx(Window& window);
	static void DestroyCtx();

	static void SetCrntCtx(VkCtx& ctx);
	static VkCtx* GetCrntCtx();
};