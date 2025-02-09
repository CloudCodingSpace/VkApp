#pragma once

#include "Utils.h"

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
    VkCommandPool m_Handle;
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

    void Begin();
    void End();

    inline void Reset() { VK_CHECK(vkResetCommandBuffer(m_Handle, 0)) }
    inline VkCommandBuffer GetHandle() const { return m_Handle; }
    inline VulkanCommandPool* GetPool() const { return m_PoolRef; } 
private:
    VkCommandBuffer m_Handle = VK_NULL_HANDLE;
    VulkanCommandPool* m_PoolRef;
};