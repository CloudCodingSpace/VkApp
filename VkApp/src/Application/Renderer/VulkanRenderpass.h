#pragma once

#include "VkCtx.h"
#include "Utils.h"

enum VulkanRenderpassFlags
{
	VULKAN_RP_FLAG_CLEAR_COLOR = 1,
	VULKAN_RP_FLAG_CLEAR_DEPTH,
	VULKAN_RP_FLAG_CLEAR_NONE
};

class VulkanRenderpass
{
public:
	static VulkanRenderpass Create(uint32_t flags);
	void Destroy();

	void Begin(VkCommandBuffer buff, VkRenderPassBeginInfo& beginInfo);
	void End(VkCommandBuffer buff);

	inline VkRenderPass GetHandle() { return m_Handle; }
private:
	VkRenderPass m_Handle = VK_NULL_HANDLE;
};