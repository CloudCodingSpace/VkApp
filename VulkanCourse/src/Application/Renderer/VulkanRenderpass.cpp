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
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;

		if (flags & VULKAN_RP_FLAG_CLEAR_DEPTH)
		{
			// TODO: Add the depth stencil ref to the renderpass
		}

		VkRenderPassCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1; // TODO: Make it extendable
		info.pAttachments = &colorAttach; // TODO: Make it extendable
		info.subpassCount = 1;
		info.pSubpasses = &subpass;

		VK_CHECK(vkCreateRenderPass(ctx->device, &info, nullptr, &pass.m_Handle))
	}

	return pass;
}

void VulkanRenderpass::Destroy()
{
	VkCtx* ctx = VkCtxHandler::GetCrntCtx();

	vkDestroyRenderPass(ctx->device, m_Handle, nullptr);
}
