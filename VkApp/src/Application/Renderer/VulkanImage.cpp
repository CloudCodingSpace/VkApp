#include "VulkanImage.h"

#include "VkCtx.h"
#include "VulkanBuffer.h"
#include "Utils.h"

VulkanImage VulkanImage::Create(int32_t width, int32_t height, unsigned char* pixels, VulkanCommandPool pool, bool hasFrequentUpdates, bool forColorAttachment)
{
	auto* ctx = VkCtxHandler::GetCrntCtx();
	VulkanImage image{};
	image.m_Width = width;
	image.m_Height = height;
	
	// Image
	{
		VkImageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.extent = VkExtent3D{ (uint32_t)width, (uint32_t)height, 1 };
		info.format = forColorAttachment ? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_R8G8B8A8_UNORM;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		info.arrayLayers = 1;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.mipLevels = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.usage = ((forColorAttachment) ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_TRANSFER_DST_BIT) | VK_IMAGE_USAGE_SAMPLED_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;

		VK_CHECK(vkCreateImage(ctx->device, &info, nullptr, &image.m_Image))
	}

	// Image Memory
	{
		VkMemoryRequirements req{};
		vkGetImageMemoryRequirements(ctx->device, image.m_Image, &req);

		VkMemoryAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		info.allocationSize = req.size;
		info.memoryTypeIndex = FindMemoryType(req.memoryTypeBits, (!hasFrequentUpdates) ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VK_CHECK(vkAllocateMemory(ctx->device, &info, nullptr, &image.m_Memory));
		vkBindImageMemory(ctx->device, image.m_Image, image.m_Memory, 0);
	}

	if(!forColorAttachment)
	{
		VulkanBufferInputData data{};
		VulkanBuffer staging;

		if (!hasFrequentUpdates)
		{
			data.pool = pool;
			data.data = pixels;
			data.size = width * height * 4 * sizeof(unsigned char);
			staging = VulkanBuffer::CreateStagingBuffer(data);
			staging.BindMem();

			staging.MapMem();
			memcpy(staging.GetMappedMemPtr(), pixels, data.size);
			staging.UnmapMem();
		}

		VulkanCmdBuffer cmd = VulkanCmdBuffer::Allocate({ pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY });
		cmd.Begin();

		{
			VkImageMemoryBarrier copy_barrier[1] = {};
			copy_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			copy_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			copy_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			copy_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			copy_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			copy_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			copy_barrier[0].image = image.m_Image;
			copy_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copy_barrier[0].subresourceRange.levelCount = 1;
			copy_barrier[0].subresourceRange.layerCount = 1;
			vkCmdPipelineBarrier(cmd.GetHandle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, copy_barrier);
		}

		if (hasFrequentUpdates)
		{
			void* mem = nullptr;
			vkMapMemory(ctx->device, image.m_Memory, 0, width * height * 4 * sizeof(unsigned char), 0, &mem);
			memcpy(mem, pixels, width * height * 4 * sizeof(unsigned char));
			vkUnmapMemory(ctx->device, image.m_Memory);
		}
		else
		{
			VkBufferImageCopy region{};
			region.imageOffset = { 0, 0, 0 };
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageExtent = { (uint32_t)width, (uint32_t)height, 1 };
			region.bufferImageHeight = 0;
			region.bufferOffset = 0;
			region.bufferRowLength = 0;

			vkCmdCopyBufferToImage(cmd.GetHandle(), staging.GetHandle(), image.m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		}

		{
			VkImageMemoryBarrier use_barrier[1] = {};
			use_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			use_barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			use_barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			use_barrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			use_barrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			use_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			use_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			use_barrier[0].image = image.m_Image;
			use_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			use_barrier[0].subresourceRange.levelCount = 1;
			use_barrier[0].subresourceRange.layerCount = 1;
			vkCmdPipelineBarrier(cmd.GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, use_barrier);

			cmd.End();

			VkCommandBuffer cmdBuffs[] = {
				cmd.GetHandle()
			};

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = cmdBuffs;

			vkQueueSubmit(ctx->tQueue, 1, &submitInfo, nullptr);
			vkQueueWaitIdle(ctx->tQueue);
		}
		cmd.Free();

		if(!hasFrequentUpdates)
			staging.Destroy();
	}
	else
	{
		VulkanCmdBuffer cmd = VulkanCmdBuffer::Allocate({ pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY });
		cmd.Begin();

		{
			VkImageMemoryBarrier use_barrier{};
			use_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			use_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			use_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			use_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			use_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			use_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			use_barrier.image = image.m_Image;
			use_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			use_barrier.subresourceRange.baseMipLevel = 0;
			use_barrier.subresourceRange.levelCount = 1;
			use_barrier.subresourceRange.baseArrayLayer = 0;
			use_barrier.subresourceRange.layerCount = 1;

			vkCmdPipelineBarrier(
				cmd.GetHandle(),
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				0,
				0, nullptr, 0, nullptr, 1, &use_barrier
			);

			cmd.End();

			VkCommandBuffer cmdBuffs[] = { cmd.GetHandle() };

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = cmdBuffs;

			vkQueueSubmit(ctx->tQueue, 1, &submitInfo, nullptr);
			vkQueueWaitIdle(ctx->tQueue);
		}
		cmd.Free();
	}

	// Image view
	{
		VkImageViewCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image = image.m_Image;
		info.format = forColorAttachment ? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_R8G8B8A8_UNORM;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.layerCount = 1;
		info.subresourceRange.levelCount = 1;
		
		VK_CHECK(vkCreateImageView(ctx->device, &info, nullptr, &image.m_ImageView));
	}

	// Sampler
	{
		VkSamplerCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.anisotropyEnable = VK_FALSE;
		info.magFilter = VK_FILTER_LINEAR;
		info.minFilter = VK_FILTER_LINEAR;
		info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		info.compareEnable = VK_FALSE;
		info.compareOp = VK_COMPARE_OP_ALWAYS;
		info.maxLod = 100;
		info.minLod = -100;
		info.mipLodBias = 0.0f;
		info.unnormalizedCoordinates = VK_FALSE;
		info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		VK_CHECK(vkCreateSampler(ctx->device, &info, nullptr, &image.m_Sampler));
	}

	return image;
}

void VulkanImage::Destroy()
{
	auto* ctx = VkCtxHandler::GetCrntCtx();

	vkFreeMemory(ctx->device, m_Memory, nullptr);
	vkDestroyImage(ctx->device, m_Image, nullptr);
	vkDestroyImageView(ctx->device, m_ImageView, nullptr);
	vkDestroySampler(ctx->device, m_Sampler, nullptr);
}