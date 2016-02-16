cmake_minimum_required(VERSION 3.0.0)
project(chipemu CXX)
set(CHIPEMU_VERSION_MAJOR 0)
set(CHIPEMU_VERSION_MINOR 1)


option(CHIPEMU_USE_WEVERYTHING
    "Use the -Weverything compiler flag" OFF)

option(CHIPEMU_USE_PEXT_BITBOARD
    "Use pext bitboard instead of old-style magic bitboards" OFF)

option(CHIPEMU_USE_ALIGNAS_64
    "Use 64 byte alignment for the position representation" OFF)

option(CHIPEMU_USE_BOARD_VECTOR_64
    "Use 64 byte vectors to represent the board" OFF)

option(CHIPEMU_USE_PIECE_MAP_VECTOR
    "Use vector operations with bitboard piece maps" OFF)

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
    src/chips.cc
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

CHECK_CXX_SOURCE_RUNS("
int addit(int* restrict a, const int* restrict b) {
  a[0] = b[1];
  return a[0] + b[1];
}
int main() {
  int a[2];
  int b[2];
  a[0] = 1;a[1] = 2;
  b[0] = 1;b[1] = 2;
  return addit(a, b) & 0;
}"
  CHIPEMU_HAS_RESTRICT_KEYWORD)

CHECK_CXX_SOURCE_RUNS("
int addit(int* __restrict__ a, const int* __restrict__ b) {
  a[0] = b[1];
  return a[0] + b[1];
}
int main() {
  int a[2];
  int b[2];
  a[0] = 1;a[1] = 2;
  b[0] = 1;b[1] = 2;
  return addit(a, b) & 0;
}"
  CHIPEMU_HAS__RESTRICT__KEYWORD)

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

# This is needed to instruct GCC not to clobber the SIMD
# registers used as permanent storage
#
if(CHIPEMU_HAS_GCC_GLOBAL_REGISTER_VARIABLE_XMM
   AND CHIPEMU_HAS_X64_128BIT_BUILTINS)
  if(CHIPEMU_HAS_GCC_GLOBAL_REGISTER_VARIABLE_YMM
     AND CHIPEMU_HAS_X64_256BIT_BUILTINS)
    if(CHIPEMU_HAS_GCC_GLOBAL_REGISTER_VARIABLE_ZMM
       AND CHIPEMU_HAS_X64_512BIT_BUILTINS)
      target_compile_options(chipemu PUBLIC -ffixed-xmm29)
      target_compile_options(chipemu PUBLIC -ffixed-xmm30)
      target_compile_options(chipemu PUBLIC -ffixed-xmm31)
    else()
    target_compile_options(chipemu PUBLIC -ffixed-xmm13)
    target_compile_options(chipemu PUBLIC -ffixed-xmm14)
    target_compile_options(chipemu PUBLIC -ffixed-xmm15)
    endif()
  else()
  target_compile_options(chipemu PUBLIC -ffixed-xmm5)
  target_compile_options(chipemu PUBLIC -ffixed-xmm6)
  target_compile_options(chipemu PUBLIC -ffixed-xmm7)
  endif()
endif()


configure_file(cmake_config.h.in config.h)

include_directories(include)

