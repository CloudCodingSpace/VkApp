#include "Renderer.h"

Renderer::Renderer(Window& window)
{
	VkCtxHelper::InitCtx(m_Ctx, window);
}

Renderer::~Renderer()
{
	VkCtxHelper::DestroyCtx(m_Ctx);
}

void Renderer::Render()
{

}

void Renderer::Update()
{

}
