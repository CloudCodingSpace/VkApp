-- Predefined global vars
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["glm"] = "../libs/glm"
IncludeDir["ImGui"] = "../libs/ImGui"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

-- The config
workspace "VkApp"
    architecture"x64"
    startproject "VkApp"
    configurations { 
        "Debug",
        "Release"
    }

group "libs"
   include "libs/tinyobj"
   include "libs/GLFW"
   include "libs/stb"
   include "libs/ImGui"
group ""

    include "VkApp"