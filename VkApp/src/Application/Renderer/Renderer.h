#pragma once

#include "VkCtx.h"
#include "VulkanRenderpass.h"
#include "VulkanFramebuffer.h"
#include "VulkanCommand.h"
#include "VulkanPipeline.h"

#include <memory>

#define MAX_FRAMES_IN_FLIGHT 2

class Renderer
{
public:
	Renderer(std::shared_ptr<Window> window);
	~Renderer();

	void Render();
	void Update();

private:
	uint32_t m_CrntImgIdx, m_CurrentFrameIdx;
	VkCtx m_Ctx{};
	VulkanRenderpass m_Pass;
	std::vector<VulkanFramebuffer> m_Framebuffs;
	VulkanCommandPool m_CmdPool;
	VulkanCmdBuffer m_CmdBuffs[MAX_FRAMES_IN_FLIGHT];
	VulkanPipeline m_Pipeline;
	// Sync objs
	VkFence m_InFlightFences[MAX_FRAMES_IN_FLIGHT];
	VkSemaphore m_ImgAvailableSemas[MAX_FRAMES_IN_FLIGHT], m_RndrFinishedSemas[MAX_FRAMES_IN_FLIGHT];

	std::shared_ptr<Window> m_Window;

private:
	void BeginFrame();
	void EndFrame();

	void RecordFrame();

	void CreateSyncObjs();
	void DestroySyncObjs();

	void OnResize(uint32_t width, uint32_t height);
};