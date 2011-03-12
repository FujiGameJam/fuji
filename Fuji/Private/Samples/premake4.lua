solution "Samples"
	configurations { "Debug", "DebugOpt", "Release", "Retail" }
--	platforms { "Native", "x32", "x64", "Xbox360", "PS3" }
--	platforms { "Native", "Xbox" }
	platforms { "Native", "x32", "x64" }

	-- include the fuji project...
	dofile "../Project/fujiproj.lua"

	-- include each of the samples --
	include "3DBox"
	include "Compute"
	include "HelloWorld"
	include "InputTest"
	include "ModelTest"
