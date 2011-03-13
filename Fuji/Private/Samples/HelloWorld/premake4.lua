project "HelloWorld"
	kind "WindowedApp"
	language "C++"

	files { "**.h", "**.cpp" }

	objdir "Build/"
	targetdir "./"

	flags { "StaticRuntime", "NoExceptions", "NoRTTI", "ExtraWarnings" }

	links { "Fuji" }

	dofile "../../Project/fujiconfig.lua"
