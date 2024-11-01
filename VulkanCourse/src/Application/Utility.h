#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include "Logger.h"

void check_vk_result(VkResult result, uint32_t lineNum, const char* funcName, const char* fileName)
{
    if (result != VK_SUCCESS)
    {
        const char* msg = "VkResult is %s (line: %d, function: %s, file: %s)";
        FATAL(msg, string_VkResult(result), lineNum, funcName, fileName);
    }
}

#define VK_CHECK(result) check_vk_result(result, __LINE__, __func__, __FILE__);