#include "VulkanBuffer.h"

#include "VkCtx.h"
#include "Utils.h"

VulkanBuffer VulkanBuffer::Create(VulkanBufferType type, VulkanBufferInputData& inputData)
{
    if(type == VulkanBufferType::VULKAN_BUFFER_TYPE_VERTEX)
        return CreateVertexBuffer(inputData);
    if(type == VulkanBufferType::VULKAN_BUFFER_TYPE_INDEX)
        return CreateIndexBuffer(inputData);

    return {};
}

void VulkanBuffer::Destroy()
{
    VkCtx* ctx = VkCtxHandler::GetCrntCtx();

    vkDestroyBuffer(ctx->device, m_Handle, nullptr);
    vkFreeMemory(ctx->device, m_Memory, nullptr);
}

void VulkanBuffer::BindMem()
{
    vkBindBufferMemory(VkCtxHandler::GetCrntCtx()->device, m_Handle, m_Memory, 0);
}

void VulkanBuffer::Resize(VulkanBufferInputData inputData)
{
    Destroy();

    *this = Create(m_Type, inputData);
}

VulkanBuffer VulkanBuffer::CreateVertexBuffer(VulkanBufferInputData& inputData)
{
    VkCtx* ctx = VkCtxHandler::GetCrntCtx();
    VulkanBuffer buffer{};
    buffer.m_Type = VulkanBufferType::VULKAN_BUFFER_TYPE_VERTEX;
    buffer.m_InputData = inputData;
    
    VkBufferCreateInfo buffInfo{};
    buffInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffInfo.size = inputData.size;
    buffInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VK_CHECK(vkCreateBuffer(ctx->device, &buffInfo, nullptr, &buffer.m_Handle));
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(ctx->device, buffer.m_Handle, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vkAllocateMemory(ctx->device, &allocInfo, nullptr, &buffer.m_Memory))

    buffer.BindMem();

    VulkanBuffer staging = CreateStagingBuffer(inputData);
    staging.BindMem();

    void* data;
    vkMapMemory(ctx->device, staging.m_Memory, 0, inputData.size, 0, &data);
    memcpy(data, inputData.data, inputData.size);
    vkUnmapMemory(ctx->device, staging.m_Memory);

    Copy(&staging, &buffer, inputData.size, 0, 0);

    staging.Destroy();

    return buffer;
}

VulkanBuffer VulkanBuffer::CreateIndexBuffer(VulkanBufferInputData& inputData)
{
    VkCtx* ctx = VkCtxHandler::GetCrntCtx();
    VulkanBuffer buffer{};
    buffer.m_Type = VulkanBufferType::VULKAN_BUFFER_TYPE_INDEX;
    buffer.m_InputData = inputData;
    
    VkBufferCreateInfo buffInfo{};
    buffInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffInfo.size = inputData.size;
    buffInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VK_CHECK(vkCreateBuffer(ctx->device, &buffInfo, nullptr, &buffer.m_Handle));
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(ctx->device, buffer.m_Handle, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vkAllocateMemory(ctx->device, &allocInfo, nullptr, &buffer.m_Memory))

    buffer.BindMem();

    VulkanBuffer staging = CreateStagingBuffer(inputData);
    staging.BindMem();

    void* data;
    vkMapMemory(ctx->device, staging.m_Memory, 0, inputData.size, 0, &data);
    memcpy(data, inputData.data, inputData.size);
    vkUnmapMemory(ctx->device, staging.m_Memory);

    Copy(&staging, &buffer, inputData.size, 0, 0);

    staging.Destroy();        
    
    return buffer;
}

VulkanBuffer VulkanBuffer::CreateStagingBuffer(VulkanBufferInputData& inputData)
{
    VkCtx* ctx = VkCtxHandler::GetCrntCtx();
    VulkanBuffer buffer{};
    buffer.m_InputData = inputData;

    VkBufferCreateInfo buffInfo{};
    buffInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffInfo.size = inputData.size;
    buffInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VK_CHECK(vkCreateBuffer(ctx->device, &buffInfo, nullptr, &buffer.m_Handle));
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(ctx->device, buffer.m_Handle, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VK_CHECK(vkAllocateMemory(ctx->device, &allocInfo, nullptr, &buffer.m_Memory))
    
    return buffer;
}

void VulkanBuffer::Copy(VulkanBuffer* src, VulkanBuffer* dst, uint64_t size, uint64_t srcOffset, uint64_t dstOffset)
{
    VkCtx* ctx = VkCtxHandler::GetCrntCtx();
    
    VulkanCmdBuffer cmd = VulkanCmdBuffer::Allocate({src->m_InputData.pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY});
    cmd.Begin();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    vkCmdCopyBuffer(cmd.GetHandle(), src->m_Handle, dst->m_Handle, 1, &copyRegion);

    cmd.End();

    VkCommandBuffer cmdBuffs[] = {
        cmd.GetHandle()
    };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = cmdBuffs;

    vkQueueSubmit(ctx->tQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(ctx->tQueue);

    cmd.Free();
}

uint32_t VulkanBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(VkCtxHandler::GetCrntCtx()->physicalDevice, &memProperties);
    
    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    return INVALID_IDX;
}