project "VMA"
    kind "StaticLib"
    language "C++"
    staticruntime "off"

    targetdir ("bin/%{cfg.buildcfg}/%{prj.name}")
    objdir ("bin-int/%{cfg.buildcfg}/%{prj.name}")

    files
    {
        "include/vma/vk_mem_alloc.h",
        "src.cpp"
    }

    includedirs
    {
        "include",
        "%{IncludeDir.VulkanSDK}"
    }

    libdirs
    {
        "%{LibraryDir.VulkanSDK}"
    }

    filter "action:vs2022"
        links 
        {
            "vulkan-1.lib"
        }

    filter "action:gmake or action:gmake2"
        links
        {
            "vulkan-1"
        }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"