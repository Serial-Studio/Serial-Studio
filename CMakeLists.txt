#
# Serial Studio - https://github.com/alex-spataru/serial-studio
#
# Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

cmake_minimum_required(VERSION 3.20)

#-------------------------------------------------------------------------------
# Define project name & find Qt packages for correct CPack calls
#-------------------------------------------------------------------------------

project(Serial-Studio LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(
   Qt6 REQUIRED
   COMPONENTS
   Core
   Qml
)

#-------------------------------------------------------------------------------
# Options for build types
#-------------------------------------------------------------------------------

option(DEBUG_SANITIZER "Enable sanitizers for debug builds" OFF)
option(PRODUCTION_OPTIMIZATION "Enable production optimization flags" ON)

#-------------------------------------------------------------------------------
# Project information
#-------------------------------------------------------------------------------

set(PROJECT_DISPNAME            "Serial Studio")
set(PROJECT_EXECUTABLE          "Serial-Studio")
set(PROJECT_VENDOR              "Alex Spataru")
set(PROJECT_CONTACT             "serial-studio.github.io")
set(PROJECT_DESCRIPTION_SUMMARY "Flexible data visualization software for embedded devices and projects")
set(PROJECT_VERSION_MAJOR       "3")
set(PROJECT_VERSION_MINOR       "1")
set(PROJECT_VERSION_PATCH       "5")
set(PROJECT_VERSION             "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(PROJECT_APPCAST             "https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/master/updates.json")

set(PROJECT_ROOT_DIR            ${CMAKE_CURRENT_LIST_DIR})
set(PROJECT_DESCRIPTION_FILE    "${PROJECT_ROOT_DIR}/README.md")
set(PROJECT_FILE_NAME           "${PROJECT_EXECUTABLE}-v${PROJECT_VERSION}")
set(PROJECT_FILE_LICENSE        "${PROJECT_ROOT_DIR}/LICENSE.md")

#-------------------------------------------------------------------------------
# Allow source code to access project information
#-------------------------------------------------------------------------------

add_definitions(-DPROJECT_VENDOR="${PROJECT_VENDOR}")
add_definitions(-DPROJECT_CONTACT="${PROJECT_CONTACT}")
add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
add_definitions(-DPROJECT_APPCAST="${PROJECT_APPCAST}")
add_definitions(-DPROJECT_DISPNAME="${PROJECT_DISPNAME}")

#-------------------------------------------------------------------------------
# Automatically enable or disable commercial Qt modules
#-------------------------------------------------------------------------------

option(BUILD_GPL3 "Force GPLv3-only build (disables commercial features regardless of Qt modules found)" OFF)

if(BUILD_GPL3)
   set(USE_QT_COMMERCIAL OFF CACHE BOOL "Commercial features disabled due to GPLv3 build mode" FORCE)
   message(STATUS "BUILD_GPL3 is ON — forcing USE_QT_COMMERCIAL=OFF for GPL compliance")
else()
   if(NOT DEFINED USE_QT_COMMERCIAL)
      find_package(Qt6 COMPONENTS Mqtt SerialBus OPTIONAL_COMPONENTS)

      if(TARGET Qt6::Mqtt AND TARGET Qt6::SerialBus)
         set(USE_QT_COMMERCIAL ON CACHE BOOL "Enable features that require Qt Commercial" FORCE)
         message(STATUS "Qt6::Mqtt and Qt6::SerialBus found — enabling USE_QT_COMMERCIAL")
      else()
         set(USE_QT_COMMERCIAL OFF CACHE BOOL "Enable features that require Qt Commercial" FORCE)
         message(WARNING "Qt6::Mqtt and/or Qt6::SerialBus NOT found — disabling USE_QT_COMMERCIAL")
      endif()
   else()
      message(STATUS "USE_QT_COMMERCIAL explicitly set to '${USE_QT_COMMERCIAL}' — skipping auto-detection")
   endif()
endif()

if(USE_QT_COMMERCIAL)
   add_compile_definitions(USE_QT_COMMERCIAL)
   message(STATUS "Defining USE_QT_COMMERCIAL for compiler")
else()
   message(STATUS "USE_QT_COMMERCIAL is OFF — commercial features excluded")
endif()

message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")

#-------------------------------------------------------------------------------
# Set UNIX friendly name for app & fix OpenSUSE builds
#-------------------------------------------------------------------------------

if (UNIX AND NOT APPLE)
   set(PROJECT_EXECUTABLE "serial-studio")
   set(CMAKE_C_COMPILER_AR "/usr/bin/ar")
   set(CMAKE_CXX_COMPILER_AR "/usr/bin/ar")
   set(CMAKE_C_COMPILER_RANLIB "/usr/bin/ranlib")
   set(CMAKE_CXX_COMPILER_RANLIB "/usr/bin/ranlib")
endif()

#-------------------------------------------------------------------------------
# Production optimization flags
#-------------------------------------------------------------------------------

if(PRODUCTION_OPTIMIZATION)
   # GCC (MinGW) on Windows
   if(WIN32 AND MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      add_compile_options(
         -O2                            # Optimize for speed
         -Wall                          # Enable most warning messages
         -Wextra                        # Enable additional warnings
         -fuse-linker-plugin            # Required for LTO with GCC
         -ftree-vectorize               # Enable loop vectorization
         -funroll-loops                 # Unroll loops to reduce branching overhead
         -fomit-frame-pointer           # Free up register, faster stack ops
         -msse2                         # Use SSE2 (baseline for x86_64 CPUs)
         -fno-fast-math                 # Disable unsafe floating point optimizations
         -fno-unsafe-math-optimizations # Prevent invalid math assumptions
      )
      add_link_options(
         -Wl,--gc-sections              # Remove unused code/data at link time
         -fuse-linker-plugin            # Required for LTO with GCC
      )

   # Clang (MinGW) on Windows
   elseif(WIN32 AND MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      add_compile_options(
         -O2                            # Optimize for speed
         -Wall                          # Enable most warnings
         -Wextra                        # Extra warnings
         -fvectorize                    # Enable loop vectorization
         -fslp-vectorize                # Enable straight-line vectorization
         -funroll-loops                 # Unroll loops for performance
         -fomit-frame-pointer           # Free a register, speed up stack ops
         -msse2                         # Target SSE2 (safe, compatible baseline)
         -fno-fast-math                 # Avoid unsafe FP optimizations
         -fno-unsafe-math-optimizations # Accurate and safe math semantics
         -flto                          # Enable link-time optimization
      )
      add_link_options(
         -Wl,-dead_strip                # Strip unused symbols from final binary
         -flto=full                     # Full LTO (more aggressive)
      )

   # MSVC (Visual Studio)
   elseif(WIN32 AND MSVC)
      add_compile_options(
         /permissive-                   # Enforce strict ISO C++ conformance
         /Zc:__cplusplus                # Set __cplusplus to actual version value
         /Zc:preprocessor               # Standards-compliant preprocessor
         /openmp:experimental           # Enable OpenMP support
         /MP                            # Enable multi-core compilation
         /O2                            # Optimize for speed
         /W4                            # High warning level
         /GL                            # Enable whole program optimization
         /fp:precise                    # IEEE-accurate floating-point math
         /Qpar                          # Enable automatic parallelization of loops
      )
      add_link_options(
         /OPT:REF                       # Remove unreferenced functions/data
         /OPT:ICF                       # Merge identical code/data blocks
         /LTCG                          # Enable link-time code generation (LTO)
      )

   # macOS (Clang/AppleClang)
   elseif(APPLE)
      add_compile_options(
         -O2                            # Optimize for speed
         -Wall                          # Enable most warning messages
         -Wextra                        # Enable additional warning messages
         -fvectorize                    # Enable basic loop vectorization
         -fslp-vectorize                # Enable straight-line (basic block) vectorization
         -funroll-loops                 # Unroll loops to reduce branching overhead
         -fomit-frame-pointer           # Omit frame pointer to free a register (faster function calls)
         -fno-fast-math                 # Prevent aggressive FP optimizations (retain IEEE compliance)
         -fno-unsafe-math-optimizations # Avoid math-related UB or precision loss
         -flto                          # Enable Link Time Optimization
      )
      add_link_options(
         -Wl,-dead_strip                # Remove unused functions/data from final binary
         -flto=full                     # Full link-time optimization across all TU
      )

   # Intel LLVM Compiler (Linux or Windows)
   elseif(CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
      add_compile_options(
         -O2                            # Optimize for speed
         -Wall                          # Enable most warning messages
         -Wextra                        # Enable additional warning messages
         -qopenmp-simd                  # Enable OpenMP SIMD loop support
         -static                        # Statically link Intel runtime dependencies
         -fvectorize                    # Enable loop vectorization
         -fslp-vectorize                # Straight-line vectorization
         -funroll-loops                 # Unroll simple loops for throughput
         -fomit-frame-pointer           # Free up a general-purpose register
         -msse2                         # Target SSE2 instruction set (safe baseline)
         -fp-model=precise              # Disable aggressive FP optimization, retain precision
      )
      add_link_options(
         -Wl,--gc-sections              # Discard unused code sections at link time
         -flto=full                     # Enable aggressive full link-time optimization
      )

   # Linux on ARM64 (e.g., Raspberry Pi 4 or other AArch64)
   elseif(UNIX AND CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
      add_compile_options(
         -O2                            # Optimize for speed
         -Wall                          # Enable most warning messages
         -Wextra                        # Enable additional warning messages
         -fopenmp                       # Enable OpenMP multithreading support
         -ftree-vectorize               # Enable GCC’s automatic loop vectorizer
         -funroll-loops                 # Improve loop performance
         -fno-fast-math                 # Disable unsafe math optimizations
         -fno-unsafe-math-optimizations # Use correct IEEE behavior
         -flto                          # Link-time optimization for smaller binaries
      )
      add_link_options(
         -Wl,--gc-sections              # Remove unused code/data
         -flto                          # Link-time optimization
      )

   # Generic x86_64 Linux (GCC or Clang)
   elseif(UNIX)
      add_compile_options(
         -O2                            # Optimize for speed
         -Wall                          # Enable most warning messages
         -Wextra                        # Enable additional warning messages
         -fopenmp                       # Enable multithreading via OpenMP
         -ftree-vectorize               # Enable automatic vectorization
         -funroll-loops                 # Improve loop performance by unrolling
         -fomit-frame-pointer           # Optimize function call overhead
         -msse2                         # Use SSE2 (supported on all x86_64 CPUs)
         -fno-fast-math                 # IEEE-compliant floating point
         -fno-unsafe-math-optimizations # Avoid unsafe math assumptions
         -flto                          # Enable link-time optimization
      )
      add_link_options(
         -Wl,--gc-sections              # Strip unused functions/data
         -flto                          # Link-time optimization
      )
   endif()
endif()

#-------------------------------------------------------------------------------
# Sanitizer flags
#-------------------------------------------------------------------------------

if(DEBUG_SANITIZER)
   add_compile_options(
      -fsanitize=address                # Enable AddressSanitizer
      -fsanitize=undefined              # Enable UndefinedBehaviorSanitizer
      -g                                # Generate debug symbols
      -fno-omit-frame-pointer           # Preserve frame pointers
   )
   add_link_options(
      -fsanitize=address                # Link AddressSanitizer
      -fsanitize=undefined              # Link UndefinedBehaviorSanitizer
   )
endif()

#-------------------------------------------------------------------------------
# Add subdirectories
#-------------------------------------------------------------------------------

add_subdirectory(lib)
add_subdirectory(app)

#-------------------------------------------------------------------------------
# Log compiler and linker flags
#-------------------------------------------------------------------------------

get_directory_property(SUBDIRECTORY_COMPILE_OPTIONS DIRECTORY lib COMPILE_OPTIONS)
message(STATUS "LIB Compile Options: ${SUBDIRECTORY_COMPILE_OPTIONS}")

get_directory_property(SUBDIRECTORY_COMPILE_OPTIONS DIRECTORY app COMPILE_OPTIONS)
message(STATUS "APP Compile Options: ${SUBDIRECTORY_COMPILE_OPTIONS}")

