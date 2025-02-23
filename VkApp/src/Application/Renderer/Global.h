#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 col;

	static VkVertexInputBindingDescription GetBindingDesc();
	static VkVertexInputAttributeDescription GetPosAttribDesc();
	static VkVertexInputAttributeDescription GetColAttribDesc();
};

struct PushConstData
{
	glm::mat4 ViewProj = glm::mat4(1.0f);
	glm::mat4 Transform = glm::mat4(1.0f);
};