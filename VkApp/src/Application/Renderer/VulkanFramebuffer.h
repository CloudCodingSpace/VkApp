#pragma once

#include "VulkanRenderpass.h"

struct VulkanFramebufferInputData
{
	uint32_t width = 0;
	uint32_t height = 0;
	std::vector<VkImageView> views{};
	VulkanRenderpass pass{};
};

class VulkanFramebuffer
{
public:
	static VulkanFramebuffer Create(VulkanFramebufferInputData& data);

	void Destroy();

	inline VkFramebuffer GetHandle() { return m_Handle; }
	inline uint32_t GetWidth() { return m_Width; }
	inline uint32_t GetHeight() { return m_Height; }

private:
	VkFramebuffer m_Handle = VK_NULL_HANDLE;
	uint32_t m_Width, m_Height;
};