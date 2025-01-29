#include "Renderer.h"

Renderer::Renderer(std::shared_ptr<Window> window)
{
	m_Window = window;
	m_CurrentFrameIdx = 0;

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

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VulkanCmdBufferInputData buffData{};
			buffData.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			buffData.pool = m_CmdPool;
			m_CmdBuffs[i] = VulkanCmdBuffer::Allocate(buffData);
		}
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
	if (m_Ctx.scExtent.width != m_Window->GetWidth() || m_Ctx.scExtent.height != m_Window->GetHeight())
	{
		OnResize(m_Window->GetWidth(), m_Window->GetHeight());
	}
}

void Renderer::BeginFrame()
{
	VK_CHECK(vkWaitForFences(m_Ctx.device, 1, &m_InFlightFences[m_CurrentFrameIdx], VK_TRUE, MAX_UINT64))
	VK_CHECK(vkResetFences(m_Ctx.device, 1, &m_InFlightFences[m_CurrentFrameIdx]))

	VkResult result = vkAcquireNextImageKHR(m_Ctx.device, m_Ctx.swapchain, MAX_UINT64, m_ImgAvailableSemas[m_CurrentFrameIdx], VK_NULL_HANDLE, &m_CrntImgIdx);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		OnResize(m_Window->GetWidth(), m_Window->GetHeight());
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		std::string resultStr = string_VkResult(result);
		std::string msg = "VkResult is " + resultStr;
		FATAL(msg);
	}
}

void Renderer::EndFrame()
{
	// Submit
	{
		auto cmdBuffHandle = m_CmdBuffs[m_CurrentFrameIdx].GetHandle();

		VkSemaphore waitSemas[] = {
			m_ImgAvailableSemas[m_CurrentFrameIdx]
		};

		VkSemaphore signalSemas[] = {
			m_RndrFinishedSemas[m_CurrentFrameIdx]
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

		VK_CHECK(vkQueueSubmit(m_Ctx.gQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrameIdx]))
	}
	// Present
	{
		VkSemaphore waitSemas[] = {
			m_RndrFinishedSemas[m_CurrentFrameIdx]
		};

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Ctx.swapchain;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = waitSemas;
		presentInfo.pImageIndices = &m_CrntImgIdx;

		VkResult result = vkQueuePresentKHR(m_Ctx.pQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			OnResize(m_Window->GetWidth(), m_Window->GetHeight());
			return;
		}
		else
		{
			VK_CHECK(result)
		}
	}

	m_CurrentFrameIdx = (m_CurrentFrameIdx + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::RecordFrame()
{
	m_CmdBuffs[m_CurrentFrameIdx].Reset();
	m_CmdBuffs[m_CurrentFrameIdx].Begin();

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

	m_Pass.Begin(m_CmdBuffs[m_CurrentFrameIdx].GetHandle(), rpBeginInfo);

	// Other commands

	m_Pass.End(m_CmdBuffs[m_CurrentFrameIdx].GetHandle());
	m_CmdBuffs[m_CurrentFrameIdx].End();
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

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VK_CHECK(vkCreateFence(m_Ctx.device, &fenceInfo, nullptr, &m_InFlightFences[i]))
		VK_CHECK(vkCreateSemaphore(m_Ctx.device, &semaInfo, nullptr, &m_ImgAvailableSemas[i]))
		VK_CHECK(vkCreateSemaphore(m_Ctx.device, &semaInfo, nullptr, &m_RndrFinishedSemas[i]))
	}
}

void Renderer::DestroySyncObjs()
{
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyFence(m_Ctx.device, m_InFlightFences[i], nullptr);
		vkDestroySemaphore(m_Ctx.device, m_ImgAvailableSemas[i], nullptr);
		vkDestroySemaphore(m_Ctx.device, m_RndrFinishedSemas[i], nullptr);
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	while (m_Window->GetWidth() == 0 || m_Window->GetHeight() == 0) {
		glfwWaitEvents();
	}

	VkCtxHandler::WaitDeviceIdle();

	for (auto& buff : m_Framebuffs)
		buff.Destroy();

	VkCtxHandler::OnResize(m_Window, width, height);

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