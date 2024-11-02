#include "VulkanFramebuffer.h"

VulkanFramebuffer VulkanFramebuffer::Create(VulkanFramebufferInputData& data)
{
	VulkanFramebuffer buff{};
	buff.m_Width = data.width;
	buff.m_Height = data.width;

	{
		VkCtx* ctx = VkCtxHandler::GetCrntCtx();

		VkFramebufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.attachmentCount = (uint32_t)data.views.size();
		info.pAttachments = data.views.data();
		info.renderPass = data.pass.GetHandle();
		info.layers = 1;
		info.width = data.width;
		info.height = data.height;
		
		VK_CHECK(vkCreateFramebuffer(ctx->device, &info, nullptr, &buff.m_Handle))
	}

	return buff;
}

void VulkanFramebuffer::Destroy()
{
	VkCtx* ctx = VkCtxHandler::GetCrntCtx();
	vkDestroyFramebuffer(ctx->device, m_Handle, nullptr);
}