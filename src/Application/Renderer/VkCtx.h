#pragma once

#include <vulkan/vulkan.h>

struct VkCtx
{
	
};

class VkCtxHelper
{
public:
	static void InitCtx(VkCtx& ctx);
	static void DestroyCtx(VkCtx& ctx);
};