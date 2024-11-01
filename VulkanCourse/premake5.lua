project "VulkanCourse"
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

    links
    {
        "GLFW",
        "TinyOBJ",
        "STB",
        "%{Library.Vulkan}"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS"
    }
    
    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

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