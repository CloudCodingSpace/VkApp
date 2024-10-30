#pragma once

#include <vulkan/vulkan.h>

#include <Window/Window.h>

struct VkCtx
{
	VkInstance instance;
};

class VkCtxHelper
{
public:
	static void InitCtx(VkCtx& ctx, Window& window);
	static void DestroyCtx(VkCtx& ctx);
};