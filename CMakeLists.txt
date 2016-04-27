cmake_minimum_required(VERSION 3.0.0)
project(chipemu CXX)
set(CHIPEMU_VERSION_MAJOR 0)
set(CHIPEMU_VERSION_MINOR 1)


option(CHIPEMU_USE_WEVERYTHING
    "Use the -Weverything compiler flag" OFF)

if(NOT MSVC)
  option(CHIPEMU_NO_ARCHNATIVE "Do not attempt to use the -march=native flag")
else()
  option(CHIPEMU_MSVC_USE_AVX_FLAG "Use the /arch:AVX flag")
  option(CHIPEMU_MSVC_USE_AVX2_FLAG "Use the /arch:AVX2 flag")
endif()

include(CheckCXXCompilerFlag)
include(CheckCXXSourceRuns)
include(CheckIncludeFiles)

SET(CHIPEMU_SOURCES
    src/chipemu.cc
    src/nmos.cc
    src/mos65xx.cc)

ADD_LIBRARY(chipemu SHARED ${CHIPEMU_SOURCES})

ADD_EXECUTABLE(testbench
               testbench/main.cc
               testbench/machine_6502.cc
               testbench/memory.cc
               testbench/machine_implementation.cc
               testbench/commodore.cc
               testbench/c64.cc
               testbench/cvic20.cc)

target_link_libraries(testbench chipemu)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CHIPEMU_STANDARD_FLAG "")


# doesn't seem to work
# set_property(TARGET chipemu PROPERTY CXX_STANDARD 14)
# set_property(TARGET chipemu PROPERTY CXX_STANDARD_REQUIRED ON)


if(NOT MSVC)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native ")

# Even after setting CXX_STANDARD to 14, cmake still doesn't
# compile the feature tests with the c++14 flag
#
  CHECK_CXX_COMPILER_FLAG("-std=c++14" CHIPEMU_COMPILER_SUPPORTS_CXX14)
  CHECK_CXX_COMPILER_FLAG("-std=c++1y" CHIPEMU_COMPILER_SUPPORTS_CXX1Y)
  if(CHIPEMU_COMPILER_SUPPORTS_CXX14)
    set(CHIPEMU_STANDARD_FLAG "-std=c++14")
  elseif(CHIPEMU_COMPILER_SUPPORTS_CXX1Y)
    set(CHIPEMU_STANDARD_FLAG "-std=c++1y")
  endif()

  target_compile_options(chipemu PRIVATE "${CHIPEMU_STANDARD_FLAG}")
  target_compile_options(testbench PRIVATE "${CHIPEMU_STANDARD_FLAG}")
  target_compile_options(testbench PRIVATE "${CHIPEMU_STANDARD_FLAG}")


endif()

if(MSVC)
#  About warning 4224 see:
#  https://connect.microsoft.com/VisualStudio/feedback/details/586147/warning-c4
#   "Warning C4224 is given for correct code if MS extensions are disabled."
#
#  Warning 4566 complains about codepage, no practical use of it
#
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4224 /wd4566")

# /Za  brings MSVC closer to ISO
  target_compile_options(chipemu PUBLIC "$<$<CONFIG:Release>:/Za /MD>")

endif()

if(CHIPEMU_COMPILER_SUPPORTS_WERROR)
  target_compile_options(chipemu PRIVATE -Werror)
  target_compile_options(testbench PRIVATE -Werror)
endif()
if(CHIPEMU_COMPILER_SUPPORTS_WALL)
  target_compile_options(chipemu PRIVATE -Wall)
  target_compile_options(testbench PRIVATE -Wall)
endif()
if(CHIPEMU_COMPILER_SUPPORTS_WEXTRA)
  target_compile_options(chipemu PRIVATE -Wextra)
  target_compile_options(testbench PRIVATE -Wextra)
endif()
if(CHIPEMU_COMPILER_SUPPORTS_PEDANTIC)
  target_compile_options(chipemu PRIVATE -pedantic)
  target_compile_options(testbench PRIVATE -pedantic)
endif()

# clang specific -Weverything flag, can result in a lot of warnings
#
if(CHIPEMU_USE_WEVERYTHING)
  CHECK_CXX_COMPILER_FLAG("-Weverything"
    CHIPEMU_COMPILER_SUPPORTS_WEVERYTHING)
  CHECK_CXX_COMPILER_FLAG("-Wno-c++98-compat"
    CHIPEMU_COMPILER_SUPPORTS_WNO98COMPAT)
  CHECK_CXX_COMPILER_FLAG("-Wno-c++98-compat-pedantic"
    CHIPEMU_COMPILER_SUPPORTS_WNO98COMPAT_PED)
  CHECK_CXX_COMPILER_FLAG("-Wno-padded" CHIPEMU_COMPILER_SUPPORTS_WNOPADDED)
  if(CHIPEMU_COMPILER_SUPPORTS_WEVERYTHING
     AND CHIPEMU_COMPILER_SUPPORTS_WNO98COMPAT
     AND CHIPEMU_COMPILER_SUPPORTS_WNO98COMPAT_PED
     AND CHIPEMU_COMPILER_SUPPORTS_WNOPADDED)
    target_compile_options(chipemu PRIVATE
      -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded)
  endif()
endif()

include_directories(include)

