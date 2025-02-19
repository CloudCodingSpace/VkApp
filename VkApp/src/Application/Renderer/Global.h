#pragma once

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