#pragma once

#include <vulkan/vulkan.h>

#include <Window/Window.h>
#include <limits>
#include <vector>

struct VulkanQueueFamilyProps
{
	uint32_t presentQueueIdx = std::numeric_limits<uint32_t>::max();
	uint32_t transferQueueIdx = std::numeric_limits<uint32_t>::max();
	uint32_t graphicsQueueIdx = std::numeric_limits<uint32_t>::max();

	inline bool IsFull() {
		return presentQueueIdx != std::numeric_limits<uint32_t>::max() && graphicsQueueIdx != std::numeric_limits<uint32_t>::max() && transferQueueIdx != std::numeric_limits<uint32_t>::max();
	}
};

struct VulkanScCaps
{
	VkSurfaceCapabilitiesKHR caps;
	std::vector<VkPresentModeKHR> modes;
	std::vector<VkSurfaceFormatKHR> formats;

	~VulkanScCaps()
	{
		modes.clear();
		formats.clear();
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
	VkDevice device = VK_NULL_HANDLE;
	VulkanScCaps scCaps{};
	VkExtent2D scExtent;
	VkPresentModeKHR scMode;
	VkSurfaceFormatKHR scFormat;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> scImgs;
	std::vector<VkImageView> scImgViews;

#if defined(_DEBUG) || !defined(NDEBUG)
	VkDebugUtilsMessengerEXT debugger;
#endif

	~VkCtx()
	{
		scImgs.clear();
		scImgViews.clear();
	}
};

class VkCtxHandler
{
public:
	static void InitCtx(Window& window);
	static void DestroyCtx();

	static void SetCrntCtx(VkCtx& ctx);
	static VkCtx* GetCrntCtx();
private:
	static void CheckCrntCtx(std::string funcName, int lineNum);
};