#include "Renderer.h"

Renderer::Renderer(Window& window)
{
	VkCtxHandler::SetCrntCtx(m_Ctx);
	VkCtxHandler::InitCtx(window);
}

Renderer::~Renderer()
{
	VkCtxHandler::DestroyCtx();
}

void Renderer::Render()
{

}

void Renderer::Update()
{

}
