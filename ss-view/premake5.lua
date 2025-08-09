workspace "ss-view"
architecture "x64"
    configurations { "Debug", "Release" }
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "core"
    location "%{prj.name}"
    kind "ConsoleApp"
    language "C++"
    targetname "%{prj.name}"
    targetdir ("bin/".. outputdir)
    objdir ("%{prj.name}/int/" .. outputdir)
    cppdialect "C++17"
    staticruntime "Off"

    files
    {
        "%{prj.name}/**.h",
        "%{prj.name}/**.c",
        "%{prj.name}/**.hpp"
,        "%{prj.name}/**.cpp"
    }

    includedirs
    {
        "%{prj.name}/include",
        "%{prj.name}/src"
    }

    defines
    {
        "SFML_STATIC"
    }

    libdirs "%{prj.name}/lib"

    links
    {
        "opengl32",
        "winmm",
        "gdi32",
        "flac",
        "freetype",
        "ogg",
        "openal32",
        "vorbis",
        "vorbisenc",
        "vorbisfile"
    }

    filter "system:windows"
		systemversion "latest"
		defines { "WIN32" }

	filter "configurations:Debug"
		defines { "_DEBUG", "_CONSOLE" }
		symbols "On"
        links
        {
            "sfml-audio-s-d",
            "sfml-graphics-s-d",
            "sfml-network-s-d",
            "sfml-system-s-d",
            "sfml-window-s-d"
        }

    filter "configurations:Release"
		defines { "NDEBUG", "_CONSOLE" }
		optimize "On"
        links
        {
            "sfml-audio-s",
            "sfml-graphics-s",
            "sfml-network-s",
            "sfml-system-s",
            "sfml-window-s"
        }

