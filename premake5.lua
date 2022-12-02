workspace "OpenGL"
	architecture "x64"
	startproject "Engine"
	
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	
	configurations {
		"Debug",
		"Release"
	}

--- Iclude directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] 	= "Engine/vendor/GLFW/include"
IncludeDir["Glad"] 	= "Engine/vendor/Glad/include"
IncludeDir["ImGui"] = "Engine/vendor/imgui"
IncludeDir["glm"] 	= "Engine/vendor/glm"

group "Dependencies"
	include "Engine/vendor/GLFW"
	include "Engine/vendor/Glad"
	include "Engine/vendor/imgui"

group ""
	
project "Engine"
	location "Engine"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	---pchheader "stdafx.h"
	---pchsource "source3/src/stdafx.cpp"

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}
	
	defines {
		"_CRT_SECURE_NO_WARNINGS"
	}
	
	includedirs {
		"%{prj.name}/src",
		---"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}"
	}

	links {
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}
	
	filter "system:windows"
		systemversion "latest"
		
		defines {
			"MGL_PLATFORM_WINDOWS",
			"GLFW_INCLUDE_NONE"
		}
		
	--- postbuildcommands { ("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"") }
		
	filter "configurations:Debug"
		defines "MGL_DEBUG"
		runtime "Debug"
		symbols "on"
		
	filter "configurations:Release"
		defines "MGL_RELEASE"
		runtime "Release"
		optimize "on"
		
	filter "configurations:Dist"
		defines "MGL_DIST"
		runtime "Release"
		optimize "on"
