#include "Renderer.h"

Renderer::Renderer(Window& window)
{
	VkCtxHandler::SetCrntCtx(m_Ctx);
	VkCtxHandler::InitCtx(window);

	m_Pass = VulkanRenderpass::Create(VULKAN_RP_FLAG_CLEAR_DEPTH);

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

Renderer::~Renderer()
{
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
