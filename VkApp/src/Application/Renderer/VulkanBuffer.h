#pragma once

#include <vulkan/vulkan.h>

#include "VulkanCommand.h"

enum class VulkanBufferType
{
    VULKAN_BUFFER_TYPE_VERTEX,
    VULKAN_BUFFER_TYPE_INDEX
};

struct VulkanBufferInputData
{
    uint64_t size = 0;
    void* data = nullptr;
    VulkanCommandPool pool;
};

class VulkanBuffer
{
public:
    static VulkanBuffer Create(VulkanBufferType type, VulkanBufferInputData& inputData);
    void Destroy();

    void MapMem(void** data);
    void UnmapMem();    

    void BindMem();

    void Resize(VulkanBufferInputData inputData);

    inline VkBuffer GetHandle() const { return m_Handle; }
    inline VkDeviceMemory GetMemory() const { return m_Memory; }
    inline VulkanBufferType GetType() const { return m_Type; }
    inline VulkanBufferInputData GetInputData() const { return m_InputData; }

private:
    VkBuffer m_Handle = nullptr;
    VkDeviceMemory m_Memory = nullptr;
    VulkanBufferType m_Type;
    VulkanBufferInputData m_InputData{};

public:
    // Util Functions
    static VulkanBuffer CreateVertexBuffer(VulkanBufferInputData& inputData);
    static VulkanBuffer CreateIndexBuffer(VulkanBufferInputData& inputData);
    static VulkanBuffer CreateStagingBuffer(VulkanBufferInputData& inputData);
    
    static void Copy(VulkanBuffer* src, VulkanBuffer* dst, uint64_t size, uint64_t srcOffset, uint64_t dstOffset);
    static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};