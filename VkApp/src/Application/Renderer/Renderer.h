#pragma once

#include "VkCtx.h"
#include "VulkanRenderpass.h"
#include "VulkanFramebuffer.h"
#include "VulkanCommand.h"

#include <memory>

class Renderer
{
public:
	Renderer(std::shared_ptr<Window> window);
	~Renderer();

	void Render();
	void Update();

private:
	uint32_t m_CrntImgIdx;
	VkCtx m_Ctx{};
	VulkanRenderpass m_Pass;
	std::vector<VulkanFramebuffer> m_Framebuffs;
	VulkanCommandPool m_CmdPool;
	VulkanCmdBuffer m_CmdBuff;
	// Sync objs
	VkFence m_InFlightFence = VK_NULL_HANDLE;
	VkSemaphore m_ImgAvailableSema = VK_NULL_HANDLE, m_RndrFinishedSema = VK_NULL_HANDLE;

	std::shared_ptr<Window> m_Window;

private:
	void BeginFrame();
	void EndFrame();

	void RecordFrame();

	void CreateSyncObjs();
	void DestroySyncObjs();

	void OnResize(uint32_t width, uint32_t height);
};