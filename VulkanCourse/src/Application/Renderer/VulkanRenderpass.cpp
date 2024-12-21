#include "VulkanRenderpass.h"

VulkanRenderpass VulkanRenderpass::Create(uint32_t flags)
{
	VulkanRenderpass pass{};
	// Creating the renderpass
	{
		VkCtx* ctx = VkCtxHandler::GetCrntCtx();

		VkAttachmentDescription colorAttach{};
		colorAttach.format = ctx->scFormat.format;
		colorAttach.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		colorAttach.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		if (flags & VULKAN_RP_FLAG_CLEAR_COLOR)
		{
			colorAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		}
		else
		{
			colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		}

		VkAttachmentReference colorRef{};
		colorRef.attachment = 0;
		colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;

		if (flags & VULKAN_RP_FLAG_CLEAR_DEPTH)
		{
			// TODO: Add the depth stencil ref to the renderpass
		}

		VkSubpassDependency spDependency = {
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
		};

		VkRenderPassCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &colorAttach;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 1;
		info.pDependencies = &spDependency;

		VK_CHECK(vkCreateRenderPass(ctx->device, &info, nullptr, &pass.m_Handle))
	}

	return pass;
}

void VulkanRenderpass::Destroy()
{
	VkCtx* ctx = VkCtxHandler::GetCrntCtx();

	vkDestroyRenderPass(ctx->device, m_Handle, nullptr);
}

void VulkanRenderpass::Begin(VkCommandBuffer buff, VkRenderPassBeginInfo& beginInfo)
{
	vkCmdBeginRenderPass(buff, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderpass::End(VkCommandBuffer buff)
{
	vkCmdEndRenderPass(buff);
}