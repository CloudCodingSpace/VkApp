#pragma once

#include <vulkan/vulkan.h>

#include "VulkanCommand.h"

enum class VulkanBufferType
{
    VULKAN_BUFFER_TYPE_VERTEX,
    VULKAN_BUFFER_TYPE_INDEX,
    VULKAN_BUFFER_TYPE_UNIFORM_BUFFER
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

    void BindMem();

    void MapMem();
    void UnmapMem();

    void Resize(VulkanBufferInputData inputData);
    
    inline void* GetMappedMemPtr() { MapMem(); return m_MappedMem; }
    inline VkBuffer GetHandle() const { return m_Handle; }
    inline VkDeviceMemory GetMemory() const { return m_Memory; }
    inline VulkanBufferType GetType() const { return m_Type; }
    inline VulkanBufferInputData GetInputData() const { return m_InputData; }

private:
    VkBuffer m_Handle = nullptr;
    VkDeviceMemory m_Memory = nullptr;
    VulkanBufferType m_Type;
    VulkanBufferInputData m_InputData{};
    void* m_MappedMem = nullptr;

public:
    // Util Functions
    static VulkanBuffer CreateVertexBuffer(VulkanBufferInputData& inputData);
    static VulkanBuffer CreateIndexBuffer(VulkanBufferInputData& inputData);
    static VulkanBuffer CreateUniformBuffer(VulkanBufferInputData& inputData);
    static VulkanBuffer CreateStagingBuffer(VulkanBufferInputData& inputData);
    
    static void Copy(VulkanBuffer* src, VulkanBuffer* dst, uint64_t size, uint64_t srcOffset, uint64_t dstOffset);
};