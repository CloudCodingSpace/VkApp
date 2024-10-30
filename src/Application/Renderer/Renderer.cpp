#include "Renderer.h"

Renderer::Renderer()
{
	VkCtxHelper::InitCtx(m_Ctx);
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
