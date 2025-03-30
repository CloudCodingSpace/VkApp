#pragma once

#include <vulkan/vulkan.h>

#include "VulkanCommand.h"

class VulkanImage
{
public:
	static VulkanImage Create(int32_t width, int32_t height, unsigned char* pixels, VulkanCommandPool pool);
	void Destroy();

	inline VkImage GetHandle() { return m_Image; }
	inline VkImageView GetViewHandle() { return m_ImageView; }
	inline VkSampler GetSamplerHandle() { return m_Sampler; }
	inline VkDeviceMemory GetImageMemory() { return m_Memory; }

private:
	VkImage m_Image = nullptr;
	VkDeviceMemory m_Memory = nullptr;
	VkImageView m_ImageView = nullptr;
	VkSampler m_Sampler = nullptr;

	int32_t m_Width, m_Height;
};