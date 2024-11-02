#include "Renderer.h"

Renderer::Renderer(Window& window)
{
	VkCtxHandler::SetCrntCtx(m_Ctx);
	VkCtxHandler::InitCtx(window);

	m_Pass = VulkanRenderpass::Create(VULKAN_RP_FLAG_CLEAR_DEPTH);
}

Renderer::~Renderer()
{
	m_Pass.Destroy();
	VkCtxHandler::DestroyCtx();
}

void Renderer::Render()
{
	
}

void Renderer::Update()
{

}
