#include "Global.h"

VkVertexInputBindingDescription Vertex::GetBindingDesc()
{
    VkVertexInputBindingDescription desc{};
    desc.binding = 0;
    desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    desc.stride = sizeof(Vertex);

    return desc;
}

VkVertexInputAttributeDescription Vertex::GetPosAttribDesc()
{
    VkVertexInputAttributeDescription desc{};
    desc.binding = 0;
    desc.format = VK_FORMAT_R32G32B32_SFLOAT;
    desc.location = 0;
    desc.offset = offsetof(Vertex, pos);

    return desc;
}

VkVertexInputAttributeDescription Vertex::GetColAttribDesc()
{
    VkVertexInputAttributeDescription desc{};
    desc.binding = 0;
    desc.format = VK_FORMAT_R32G32B32_SFLOAT;
    desc.location = 1;
    desc.offset = offsetof(Vertex, col);

    return desc;
}