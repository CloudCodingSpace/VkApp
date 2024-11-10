#pragma once

#include "Global.h"

#include <vulkan/vulkan.h>

struct VulkanCmdPoolInputData
{
    uint32_t queueFamily = MAX_UINT32;
    VkCommandPoolCreateFlags flags = 0;
};

class VulkanCommandPool
{
public:
    static VulkanCommandPool Create(VulkanCmdPoolInputData data);
    void Destroy();

    inline VkCommandPool GetHandle() const { return m_Handle; }
private:
    VkCommandPool m_Handle = VK_NULL_HANDLE;
};

struct VulkanCmdBufferInputData
{
    VulkanCommandPool pool;
    VkCommandBufferLevel level;
};

class VulkanCmdBuffer
{
public:
    static VulkanCmdBuffer Allocate(VulkanCmdBufferInputData data);
    void Free();

    inline VkCommandBuffer GetHandle() const { return m_Handle; }
    inline VulkanCommandPool* GetPool() const { return m_PoolRef; } 
private:
    VkCommandBuffer m_Handle = VK_NULL_HANDLE;
    VulkanCommandPool* m_PoolRef;
};