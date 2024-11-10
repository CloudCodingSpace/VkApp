#pragma once

#include "VkCtx.h"
#include "VulkanRenderpass.h"
#include "VulkanFramebuffer.h"
#include "VulkanCommand.h"

class Renderer
{
public:
	Renderer(Window& window);
	~Renderer();

	void Render();
	void Update();

private:
	VkCtx m_Ctx{};
	VulkanRenderpass m_Pass;
	std::vector<VulkanFramebuffer> m_Framebuffs;
	VulkanCommandPool m_CmdPool;
	VulkanCmdBuffer m_CmdBuff;
};