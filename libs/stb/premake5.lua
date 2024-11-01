project "STB"
    kind "StaticLib"
    language "C"
    staticruntime "on"

    targetdir ("bin/%{cfg.buildcfg}/%{prj.name}")
    objdir ("bin-int/%{cfg.buildcfg}/%{prj.name}")

    files
    {
        "include/stb_image.h",
        "src.c"
    }

    includedirs
    {
        "include"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
    
