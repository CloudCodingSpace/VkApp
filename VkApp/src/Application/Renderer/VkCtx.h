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
	VkInstance instance = nullptr;
	VkSurfaceKHR surface = nullptr;
	VkPhysicalDevice physicalDevice = nullptr;
	VulkanQueueFamilyProps queueProps{};
	VkQueue gQueue = nullptr;
	VkQueue pQueue = nullptr;
	VkQueue tQueue = nullptr;
	VkDevice device = nullptr;
	VulkanScCaps scCaps{};
	VkExtent2D scExtent;
	VkPresentModeKHR scMode;
	VkSurfaceFormatKHR scFormat;
	VkSwapchainKHR swapchain = nullptr;
	std::vector<VkImage> scImgs;
	std::vector<VkImageView> scImgViews;

	bool vsync = true;

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
	static void InitCtx(Window& window, bool vsync = true);
	static void DestroyCtx();

	static void OnResize(Window& window, uint32_t width, uint32_t height);

	static void SetCrntCtx(VkCtx& ctx);
	static VkCtx* GetCrntCtx();

	static void WaitDeviceIdle();
private:
	static void CheckCrntCtx(std::string funcName, int lineNum);
};