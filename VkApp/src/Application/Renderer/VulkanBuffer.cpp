#include "VulkanBuffer.h"

#include "VkCtx.h"
#include "Utils.h"

VulkanBuffer VulkanBuffer::Create(VulkanBufferType type, VulkanBufferInputData& inputData)
{
    if(type == VulkanBufferType::VULKAN_BUFFER_TYPE_VERTEX)
        return CreateVertexBuffer(inputData);
    if(type == VulkanBufferType::VULKAN_BUFFER_TYPE_INDEX)
        return CreateIndexBuffer(inputData);
    if(type == VulkanBufferType::VULKAN_BUFFER_TYPE_UNIFORM_BUFFER)
        return CreateUniformBuffer(inputData);

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

void VulkanBuffer::MapMem()
{
    vkMapMemory(VkCtxHandler::GetCrntCtx()->device, m_Memory, 0, m_InputData.size, 0, &m_MappedMem);
}

void VulkanBuffer::UnmapMem()
{
    vkUnmapMemory(VkCtxHandler::GetCrntCtx()->device, m_Memory);
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

    if(allocInfo.memoryTypeIndex == INVALID_IDX)
    {
        FATAL("Failed to find suitable memory type index for allocating the buffers!");
        exit(-1);
    }

    VK_CHECK(vkAllocateMemory(ctx->device, &allocInfo, nullptr, &buffer.m_Memory))

    buffer.BindMem();

    VulkanBuffer staging = CreateStagingBuffer(inputData);
    staging.BindMem();

    vkMapMemory(ctx->device, staging.m_Memory, 0, inputData.size, 0, &staging.m_MappedMem);
    memcpy(staging.m_MappedMem, inputData.data, inputData.size);
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
    
    if(allocInfo.memoryTypeIndex == INVALID_IDX)
    {
        FATAL("Failed to find suitable memory type index for allocating the buffers!");
        exit(-1);
    }

    VK_CHECK(vkAllocateMemory(ctx->device, &allocInfo, nullptr, &buffer.m_Memory))

    buffer.BindMem();

    VulkanBuffer staging = CreateStagingBuffer(inputData);
    staging.BindMem();

    vkMapMemory(ctx->device, staging.m_Memory, 0, inputData.size, 0, &staging.m_MappedMem);
    memcpy(staging.m_MappedMem, inputData.data, inputData.size);
    vkUnmapMemory(ctx->device, staging.m_Memory);

    Copy(&staging, &buffer, inputData.size, 0, 0);

    staging.Destroy();
    
    return buffer;
}

VulkanBuffer VulkanBuffer::CreateUniformBuffer(VulkanBufferInputData& inputData)
{
    VkCtx* ctx = VkCtxHandler::GetCrntCtx();
    VulkanBuffer buffer{};
    buffer.m_InputData = inputData;
    buffer.m_Type = VulkanBufferType::VULKAN_BUFFER_TYPE_UNIFORM_BUFFER;

    VkBufferCreateInfo buffInfo{};
    buffInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffInfo.size = inputData.size;
    buffInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VK_CHECK(vkCreateBuffer(ctx->device, &buffInfo, nullptr, &buffer.m_Handle));
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(ctx->device, buffer.m_Handle, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    if(allocInfo.memoryTypeIndex == INVALID_IDX)
    {
        FATAL("Failed to find suitable memory type index for allocating the buffers!");
        exit(-1);
    }

    VK_CHECK(vkAllocateMemory(ctx->device, &allocInfo, nullptr, &buffer.m_Memory))

    buffer.BindMem();

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

    if(allocInfo.memoryTypeIndex == INVALID_IDX)
    {
        FATAL("Failed to find suitable memory type index for allocating the buffers!");
        exit(-1);
    }

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