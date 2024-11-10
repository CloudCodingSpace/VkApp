#include "Renderer.h"

Renderer::Renderer(Window& window)
{
	// The Ctx
	VkCtxHandler::SetCrntCtx(m_Ctx);
	VkCtxHandler::InitCtx(window);

	// RndrPass
	m_Pass = VulkanRenderpass::Create(VULKAN_RP_FLAG_CLEAR_DEPTH);

	// Framebuff
	{
		VulkanFramebufferInputData inputData{};
		inputData.width = m_Ctx.scExtent.width;
		inputData.height = m_Ctx.scExtent.height;
		inputData.pass = m_Pass;

		int i = 0;
		m_Framebuffs.resize(m_Ctx.scImgViews.size());
		for (const auto& imgView : m_Ctx.scImgViews)
		{
			inputData.views = {
				imgView
			};

			m_Framebuffs[i] = VulkanFramebuffer::Create(inputData);

			i++;
		}
	}
	// Cmd pool and buff
	{
		VulkanCmdPoolInputData poolData{};
		poolData.queueFamily = m_Ctx.queueProps.graphicsQueueIdx;
		poolData.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		m_CmdPool = VulkanCommandPool::Create(poolData);

		VulkanCmdBufferInputData buffData{};
		buffData.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		buffData.pool = m_CmdPool;
		m_CmdBuff = VulkanCmdBuffer::Allocate(buffData);
	}
}

Renderer::~Renderer()
{
	m_CmdBuff.Free();
	m_CmdPool.Destroy();

	for (auto& buff : m_Framebuffs)
		buff.Destroy();

	m_Pass.Destroy();
	VkCtxHandler::DestroyCtx();
}

void Renderer::Render()
{
	
}

void Renderer::Update()
{

}
