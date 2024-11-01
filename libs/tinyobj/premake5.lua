project "TinyOBJ"
    kind "StaticLib"
    language "C++"
    cppdialect "C++11"
    staticruntime "off"

    targetdir ("bin/%{cfg.buildcfg}/%{prj.name}")
    objdir ("bin-int/%{cfg.buildcfg}/%{prj.name}")

    files
    {
        "include/tiny_obj_loader.h",
        "src.cpp"
    }

    includedirs
    {
        "include"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
