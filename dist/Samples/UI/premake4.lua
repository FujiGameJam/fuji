project "UI"
	kind "WindowedApp"
	language "C++"

	files { "**.h", "**.cpp" }

	objdir "Build/"
	targetdir "./"

	flags { "WinMain", "StaticRuntime", "NoExceptions", "NoRTTI", "ExtraWarnings" }

	links { "Fuji", "Haku" }

	dofile "../../../Fuji/Public/Project/fujiconfig.lua"
	dofile "../../Project/hakuconfig.lua"
