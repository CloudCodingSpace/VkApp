#pragma once

#include <vulkan/vulkan.h>

#include "VkCtx.h"
#include "VulkanRenderpass.h"

enum class VulkanPipelineType
{
	VULKAN_PIPELINE_TYPE_GRAPHICS,
	VULKAN_PIPELINE_TYPE_COMPUTE
};

struct VulkanPipelineInfo
{
	VulkanPipelineType type;
	uint32_t layoutCount = 0, pushConstRangeCount = 0;
	VkDescriptorSetLayout* layouts = nullptr;
	VkPushConstantRange* pushConstRanges = nullptr;
	VkRenderPass renderPass = nullptr;
	std::string vertPath, fragPath;
	bool enableCulling = true, enableDepthClamp = false, enableRasterizationDiscard = false;
	VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	float lineWidth = 1.0f;
	VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
	VkExtent2D extent;
	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
};

class VulkanPipeline
{
public:
	static VulkanPipeline Create(VulkanPipelineInfo& info);
	void Destroy();

	void Bind(VkCommandBuffer buffer);

	inline VkPipelineBindPoint GetBindPoint() { return m_BindPoint; }
	inline VkPipeline GetHandle() { return m_Handle; }
	inline VkPipelineLayout GetLayout() { return m_Layout; }
	inline VulkanPipelineInfo GetInfo() { return m_Info; }
private:
	VkPipeline m_Handle = nullptr;
	VkPipelineLayout m_Layout = nullptr;
	VkPipelineBindPoint m_BindPoint;
	VulkanPipelineInfo m_Info;
	VkCtx* m_Ctx = nullptr;
};