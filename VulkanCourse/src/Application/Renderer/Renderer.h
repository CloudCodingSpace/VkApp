#pragma once

#include "VkCtx.h"
#include "VulkanRenderpass.h"

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
};