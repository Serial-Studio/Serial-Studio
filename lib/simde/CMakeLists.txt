#
# Copyright (c) 2024 Alex Spataru <https://github.com/alex-spataru>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

cmake_minimum_required(VERSION 3.19)

# Define the SIMDe interface library
add_library(simde INTERFACE)
target_include_directories(simde INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Compiler and architecture-specific optimizations
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|aarch64)")
 message(STATUS "Detected ARM architecture")

 # Check for NEON support (32-bit ARM) or Advanced SIMD (64-bit ARM)
 include(CheckCXXCompilerFlag)
 check_cxx_compiler_flag("-mfpu=neon" COMPILER_SUPPORTS_NEON)
 check_cxx_compiler_flag("-march=armv8-a+simd" COMPILER_SUPPORTS_ARMV8_SIMD)

 if(COMPILER_SUPPORTS_ARMV8_SIMD)
  target_compile_options(simde INTERFACE -march=armv8-a+simd)
  message(STATUS "Using ARMv8 Advanced SIMD optimizations")
 elseif(COMPILER_SUPPORTS_NEON)
  target_compile_options(simde INTERFACE -mfpu=neon)
  message(STATUS "Using ARM NEON optimizations")
 else()
  message(WARNING "No SIMD support detected on ARM; falling back to portable mode")
  target_compile_definitions(simde INTERFACE SIMDE_NO_NATIVE)
 endif()

elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86|x86_64)")
 message(STATUS "Detected x86 architecture")

 # Enable AVX/AVX2 optimizations if supported
 if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
  target_compile_options(simde INTERFACE -msse2 -mavx -mavx2)
  message(STATUS "Enabled SSE2, AVX, and AVX2 optimizations for GCC/Clang")
 elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(simde INTERFACE /arch:AVX2)
  message(STATUS "Enabled AVX2 optimizations for MSVC")
 endif()

else()
 message(WARNING "Unknown architecture; enabling portable fallback")
 target_compile_definitions(simde INTERFACE SIMDE_NO_NATIVE)
endif()

# Enable runtime CPU detection for SIMDe to adapt to the best available instruction set
target_compile_definitions(simde INTERFACE SIMDE_ENABLE_RUNTIME_CPU_DETECTION)

# Enable OpenMP SIMD support if available
find_package(OpenMP QUIET)
if(OpenMP_CXX_FOUND)
 target_compile_definitions(simde INTERFACE SIMDE_ENABLE_OPENMP)
 target_compile_options(simde INTERFACE ${OpenMP_CXX_FLAGS})
 message(STATUS "OpenMP SIMD support enabled")
else()
 message(WARNING "OpenMP not found; SIMD performance may be suboptimal")
endif()
