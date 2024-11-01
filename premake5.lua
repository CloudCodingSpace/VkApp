-- PremakeConfig
workspace "VulkanCourse"
    architecture"x64"
    startproject "VulkanCourse"
    configurations { 
            "Debug",
            "Release"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["glm"] = "../libs/glm"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"

group "libs"
   include "libs/tinyobj"
   include "libs/GLFW"
   include "libs/stb"
group ""

    include "VulkanCourse"