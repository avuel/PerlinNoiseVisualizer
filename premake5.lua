-- premake5.lua
workspace "PerlinNoise"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "PerlinNoise"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "PerlinNoise"
