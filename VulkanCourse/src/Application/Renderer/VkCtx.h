#pragma once

#include <vulkan/vulkan.h>

#include <Window/Window.h>
#include <limits>

struct VulkanQueueFamilyProps
{
	uint32_t pQueueIdx = std::numeric_limits<uint32_t>::max();
	uint32_t tQueueIdx = std::numeric_limits<uint32_t>::max();
	uint32_t gQueueIdx = std::numeric_limits<uint32_t>::max();

	inline bool IsFull() {
		return pQueueIdx != std::numeric_limits<uint32_t>::max() && gQueueIdx != std::numeric_limits<uint32_t>::max() && tQueueIdx != std::numeric_limits<uint32_t>::max();
	}
};


struct VkCtx
{
	VkInstance instance = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VulkanQueueFamilyProps queueProps{};
	VkQueue gQueue = VK_NULL_HANDLE;
	VkQueue pQueue = VK_NULL_HANDLE;
	VkQueue tQueue = VK_NULL_HANDLE;

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