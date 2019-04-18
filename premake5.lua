function win_sdk_version()
    local reg_arch = iif( os.is64bit(), "\\Wow6432Node\\", "\\" )
    local sdk_version = os.getWindowsRegistry( "HKLM:SOFTWARE" .. reg_arch .."Microsoft\\Microsoft SDKs\\Windows\\v10.0\\ProductVersion" )
    if sdk_version ~= nil then return sdk_version end
end

function include_glm()
    includedirs "external/glm/include/"
end
    
function includeSDL()
    includedirs "external/SDL2/include"
end

function linkSDL()
    libdirs "external/sdl2/lib/"
    filter "kind:not StaticLib"
        links { "SDL2", "SDL2main" }
    filter {}
end

function includeEmbree()
    includedirs "external/embree2/include"
end

function linkEmbree()
    libdirs "external/embree2/lib/"
    filter "kind:not StaticLib"
        links { "embree", "tbb", "tbbmalloc" }
    filter {}
end

function includeGlew()
    includedirs "external/glew/include"
end

function linkGlew()
    libdirs "external/glew/lib/"
    filter "kind:not StaticLib"
        links { "glew32", "opengl32", "glu32.lib" }
    filter {}
end

function includeImgui()
    includedirs "external/imgui-1.51"
end

function linkImgui()
    libdirs "external/imgui-1.51/lib/"
    filter "kind:StaticLib"
        links { "imgui" }
    filter {}
end

function includeStb()
    includedirs "external/stb-master"
end

function includeTiny()
    includedirs "external/tinyobjloader-1.0.6"
end

workspace "CGProject"
    location "solution"
    language "C++"

    startproject "game"
	
	architecture "x86_64"
    configurations { "debug", "release" }

	filter { "configurations:debug", "system:windows", "action:vs*"}
        symbols "On"
        targetdir "builds/debug"
        systemversion(win_sdk_version() .. ".0")
        objdir "builds/debug/obj/%{prj.name}/%{cfg.longname}"
	
	filter { "configurations:release", "system:windows", "action:vs*"}
        optimize "On"
        targetdir "builds/release"
        systemversion(win_sdk_version() .. ".0")
        objdir "builds/release/obj/%{prj.name}/%{cfg.longname}"

    filter { }

    -- Copy SDL library to build directory.
    filter { "system:windows" }
        os.copyfile("external/bin/SDL2.dll", "builds/debug/SDL2.dll")
        os.copyfile("external/bin/SDL2.dll", "builds/release/SDL2.dll")
        os.copyfile("external/bin/embree.dll", "builds/debug/embree.dll")
        os.copyfile("external/bin/embree.dll", "builds/release/embree.dll")
        os.copyfile("external/bin/glew32.dll", "builds/debug/glew32.dll")
        os.copyfile("external/bin/glew32.dll", "builds/release/glew32.dll")
        os.copyfile("external/bin/freeglut.dll", "builds/debug/freeglut.dll")
        os.copyfile("external/bin/freeglut.dll", "builds/release/freeglut.dll")
        os.copyfile("external/bin/tbb.dll", "builds/debug/tbb.dll")
        os.copyfile("external/bin/tbb.dll", "builds/release/tbb.dll")
        os.copyfile("external/bin/tbbmalloc.dll", "builds/debug/tbbmalloc.dll")
        os.copyfile("external/bin/tbbmalloc.dll", "builds/release/tbbmalloc.dll")
        
    filter {}

project "engine"
    kind "StaticLib"
    location "source/engine"
    files { "source/engine/**.h", "source/engine/**.cpp" }

    includeSDL()
    linkSDL()
    include_glm()
    includeEmbree()
    linkEmbree()
    includeGlew()
    linkGlew()
    includeImgui()
    linkImgui()
    includeStb()
    includeTiny()

function use_engine()
    includedirs "source/engine"
    links "engine"
    
    filter {}
    
    includeSDL()
    linkSDL()
    include_glm()
    includeEmbree()
    linkEmbree()
    includeGlew()
    linkGlew()
    includeImgui()
    includeStb()
    includeTiny()
end

project "game"
    kind "ConsoleApp"
    location "source/game"
    files { "source/game/**.h", "source/game/**.cpp", "source/game/shaders/**.frag", "source/game/shaders/**.vert", "source/game/shaders/**.tese", "source/game/shaders/**.tesc" }
    use_engine()
