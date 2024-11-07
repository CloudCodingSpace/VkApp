-- Predefined global vars
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["glm"] = "../libs/glm"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

-- The config
workspace "VulkanCourse"
    architecture"x64"
    startproject "VulkanCourse"
    configurations { 
            "Debug",
            "Release"
    }



group "libs"
   include "libs/tinyobj"
   include "libs/GLFW"
   include "libs/stb"
group ""

    include "VulkanCourse"