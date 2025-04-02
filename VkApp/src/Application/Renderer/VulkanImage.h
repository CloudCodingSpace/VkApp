#pragma once

#include <vulkan/vulkan.h>

#include "VulkanCommand.h"

class VulkanImage
{
public:
	static VulkanImage Create(int32_t width, int32_t height, unsigned char* pixels, VulkanCommandPool pool, bool hasFrequentUpdates = false, bool forColorAttachment = false);
	void Destroy();

	inline VkImage GetHandle() { return m_Image; }
	inline VkImageView GetViewHandle() { return m_ImageView; }
	inline VkSampler GetSamplerHandle() { return m_Sampler; }
	inline VkDeviceMemory GetImageMemory() { return m_Memory; }

	inline int32_t GetWidth() { return m_Width; }
	inline int32_t GetHeight() { return m_Height; }

private:
	VkImage m_Image = nullptr;
	VkDeviceMemory m_Memory = nullptr;
	VkImageView m_ImageView = nullptr;
	VkSampler m_Sampler = nullptr;

	int32_t m_Width, m_Height;
};