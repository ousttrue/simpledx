-- A solution contains projects, and defines the available configurations
solution "Sample"
configurations { "Release", "Debug" }
configuration "gmake Debug"
do
    buildoptions { "-g" }
    linkoptions { "-g" }
end

configuration "gmake"
do
  buildoptions { 
      "-Wall", 
  }
end

configuration "gmake windows"
do
  buildoptions { 
      "-U__CYGWIN__", 
  }
end

configuration "vs*"
do
    linkoptions { "/NODEFAULTLIB:LIBCMT" }
end

configuration "windows*"
do
    defines {
        'WIN32',
        '_WIN32',
        '_WINDOWS',
    }
end

configuration "Debug"
do
  defines { "DEBUG" }
  flags { "Symbols" }
  targetdir "debug"
end

configuration "Release"
do
  defines { "NDEBUG" }
  flags { "Optimize" }
  targetdir "release"
end

configuration {}

------------------------------------------------------------------------------
-- Project
------------------------------------------------------------------------------
project "Sample"
--language "C"
language "C++"
--kind "StaticLib"
--kind "DynamicLib"
--kind "ConsoleApp"
kind "WindowedApp"

flags {
    "WinMain",
}
files {
    "**.cpp", "**.h",
}
defines {
}
includedirs {
}
libdirs {
}
links {
}

