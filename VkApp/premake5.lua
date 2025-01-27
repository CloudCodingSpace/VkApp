project "VkApp"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    staticruntime "off"

    files 
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
      "src/Application",
      "../libs/glfw/include",
      "../libs/stb/include",
      "../libs/tinyobj/include",

      "%{IncludeDir.VulkanSDK}",
      "%{IncludeDir.glm}",
    }

    flags
    {
        "MultiProcessorCompile"
    }

    libdirs
    {
        "%{LibraryDir.VulkanSDK}"
    }

    links
    {
        "GLFW",
        "TinyOBJ",
        "STB"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS"
    }
    
    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

    filter "action:vs2022"
        links 
        {
            "vulkan-1.lib"
        }

    filter "action:gmake or action:gmake2"
        filter "system:windows"
            links { "gdi32", "user32", "shell32" }

        links
        {
            "vulkan-1"
        }

    filter "system:windows"
        systemversion "latest"
        defines { "_WIN32" }

    filter "configurations:Debug"
        defines { "_DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        optimize "On"
        symbols "On"