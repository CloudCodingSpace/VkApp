#include "Renderer.h"

Renderer::Renderer(Window& window)
{
	// The Ctx
	VkCtxHandler::SetCrntCtx(m_Ctx);
	VkCtxHandler::InitCtx(window);

	// RndrPass
	m_Pass = VulkanRenderpass::Create(VULKAN_RP_FLAG_CLEAR_COLOR);

	// Sync Objs
	CreateSyncObjs();

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
	VkCtxHandler::WaitDeviceIdle();

	DestroySyncObjs();

	m_CmdPool.Destroy();

	for (auto& buff : m_Framebuffs)
		buff.Destroy();

	m_Pass.Destroy();
	VkCtxHandler::DestroyCtx();
}

void Renderer::Render()
{
	BeginFrame();

	RecordFrame();
	
	EndFrame();
}

void Renderer::Update()
{

}

void Renderer::BeginFrame()
{
	VK_CHECK(vkWaitForFences(m_Ctx.device, 1, &m_InFlightFence, VK_TRUE, MAX_UINT64))
	VK_CHECK(vkResetFences(m_Ctx.device, 1, &m_InFlightFence))

	VK_CHECK(vkAcquireNextImageKHR(m_Ctx.device, m_Ctx.swapchain, MAX_UINT64, m_ImgAvailableSema, VK_NULL_HANDLE, &m_CrntImgIdx))
}

void Renderer::EndFrame()
{	
	// Submit
	{
		auto cmdBuffHandle = m_CmdBuff.GetHandle();

		VkSemaphore waitSemas[] = {
			m_ImgAvailableSema
		};

		VkSemaphore signalSemas[] = {
			m_RndrFinishedSema
		};

		VkPipelineStageFlags waitDstFlags[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffHandle;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemas;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemas;
		submitInfo.pWaitDstStageMask = waitDstFlags;

		VK_CHECK(vkQueueSubmit(m_Ctx.gQueue, 1, &submitInfo, m_InFlightFence))
	}
	// Present
	{
		VkSemaphore waitSemas[] = {
			m_RndrFinishedSema
		};

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Ctx.swapchain;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = waitSemas;
		presentInfo.pImageIndices = &m_CrntImgIdx;

		VK_CHECK(vkQueuePresentKHR(m_Ctx.pQueue, &presentInfo))
	}
}

void Renderer::RecordFrame()
{
	m_CmdBuff.Reset();
	m_CmdBuff.Begin();

	auto& frameBuff = m_Framebuffs[m_CrntImgIdx];

	VkClearValue clearVal = {{{ 0.1f, 0.1f, 0.1f, 1.0f }}};

	VkRenderPassBeginInfo rpBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = m_Pass.GetHandle(),
		.framebuffer = frameBuff.GetHandle(),
		.renderArea = {
			.offset = { 0, 0 },
			.extent = m_Ctx.scExtent
		},
		.clearValueCount = 1,
		.pClearValues = &clearVal
	};

	m_Pass.Begin(m_CmdBuff.GetHandle(), rpBeginInfo);

	// Other commands

	m_Pass.End(m_CmdBuff.GetHandle());
	m_CmdBuff.End();
}

void Renderer::CreateSyncObjs()
{
	VkSemaphoreCreateInfo semaInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};

	VkFenceCreateInfo fenceInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	VK_CHECK(vkCreateFence(m_Ctx.device, &fenceInfo, nullptr, &m_InFlightFence))
	VK_CHECK(vkCreateSemaphore(m_Ctx.device, &semaInfo, nullptr, &m_ImgAvailableSema))
	VK_CHECK(vkCreateSemaphore(m_Ctx.device, &semaInfo, nullptr, &m_RndrFinishedSema))
}

void Renderer::DestroySyncObjs()
{
	vkDestroyFence(m_Ctx.device, m_InFlightFence, nullptr);
	vkDestroySemaphore(m_Ctx.device, m_ImgAvailableSema, nullptr);
	vkDestroySemaphore(m_Ctx.device, m_RndrFinishedSema, nullptr);
}