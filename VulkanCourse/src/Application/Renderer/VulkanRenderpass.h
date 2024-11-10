#pragma once

#include "VkCtx.h"
#include "Global.h"

enum VulkanRenderpassFlags
{
	VULKAN_RP_FLAG_CLEAR_COLOR = 1,
	VULKAN_RP_FLAG_CLEAR_DEPTH
};

class VulkanRenderpass
{
public:
	static VulkanRenderpass Create(uint32_t flags);

	void Destroy();

	inline VkRenderPass GetHandle() { return m_Handle; }

private:
	VkRenderPass m_Handle = VK_NULL_HANDLE;
};