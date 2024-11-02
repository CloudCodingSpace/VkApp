#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include "Logger.h"

inline void check_vk_result(VkResult result, uint32_t lineNum, std::string funcName, std::string fileName)
{
    if (result != VK_SUCCESS)
    {
        std::string resultStr = string_VkResult(result);
        std::string msg = "VkResult is " + resultStr + " (line: " + std::to_string(lineNum) + ", function: " + funcName + ", file: " + fileName + ")";
        FATAL(msg);
    }
}

#define VK_CHECK(result) check_vk_result(result, __LINE__, __func__, __FILE__);