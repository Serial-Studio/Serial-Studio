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
option(PRODUCTION_OPTIMIZATION "Enable production optimization flags" OFF)

#-------------------------------------------------------------------------------
# Project information
#-------------------------------------------------------------------------------

set(PROJECT_DISPNAME            "Serial Studio")
set(PROJECT_EXECUTABLE          "Serial-Studio")
set(PROJECT_VENDOR              "Alex Spataru")
set(PROJECT_CONTACT             "serial-studio.github.io")
set(PROJECT_DESCRIPTION_SUMMARY "Flexible data visualization software for embedded devices and projects")
set(PROJECT_VERSION_MAJOR       "3")
set(PROJECT_VERSION_MINOR       "0")
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
# CPU architecture configuration
#-------------------------------------------------------------------------------

if(APPLE) 
  if(CMAKE_OSX_ARCHITECTURES MATCHES "x86_64")
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
  elseif(CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
    set(CMAKE_SYSTEM_PROCESSOR "arm64")
  endif()
endif()

#-------------------------------------------------------------------------------
# Production optimization flags
#-------------------------------------------------------------------------------

if(PRODUCTION_OPTIMIZATION)
 # MinGW-specific settings
 if(WIN32 AND MINGW)
  add_compile_options(
   -O3                            # Optimize for speed
   -Wall                          # Enable most warning messages
   -Wextra                        # Enable additional warning messages
   -fuse-linker-plugin            # Use LTO plugin
   -ftree-vectorize               # Enable loop vectorization
   -fno-fast-math                 # Standard-compliant floating point math
   -fno-unsafe-math-optimizations # Use safe math only
  )
  add_link_options(
   -Wl,--gc-sections              # Remove unused sections during linking
   -fuse-linker-plugin            # Enable LTO during linking
  )

 # MSVC-specific settings
 elseif(WIN32 AND MSVC)
  add_compile_options(
   /openmp:experimental           # Add support for OpenMP
   /permissive-                   # Enable strict ISO compliance
   /Zc:__cplusplus                # Correct __cplusplus value
   /Zc:preprocessor               # Enable standards-conforming preprocessor
   /MP                            # Multi-processor compilation
   /vmg                           # Use general pointer-to-member representation
   /Ox                            # Optimize for speed
   /W3                            # Warning level 3 messages
   /GL                            # Enable whole program optimization
   /MT                            # Link compiler runtime statically
   /Qvec                          # Enable loop vectorization
   /fp:strict                     # Standard-compliant floating point math
  )
  add_link_options(
   /OPT:REF                       # Remove unreferenced functions/data
   /OPT:ICF                       # Remove identical COMDATs
   /LTCG                          # Enable LTO during linkingn
  )

 # macOS-specific settings
 elseif(APPLE)
  include_directories(
   "/opt/homebrew/opt/libomp/include"
   "/opt/homebrew/opt/llvm/include"
  )
  link_directories(
   "/opt/homebrew/opt/libomp/lib"
   "/opt/homebrew/opt/llvm/lib"
  )

  add_compile_options(
   -O3                            # Optimize for speed
   -Wall                          # Enable most warning messages
   -Wextra                        # Enable additional warning messages
   -fopenmp=libomp                # Add support for OpenMP
   -fvectorize                    # Enable loop vectorization
   -fslp-vectorize                # Enable SLP vectorization
   -fno-fast-math                 # Standard-compliant floating point math
   -fno-unsafe-math-optimizations # Use safe math only
  )
  add_link_options(
   -Wl,-dead_strip                # Remove unused code and data during linking
   -flto=full                     # Link-time optimization
  )

 # Intel LLVM-based C++ Compiler
 elseif(CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
  add_compile_options(
   -O3                            # Optimize for speed
   -Wall                          # Enable most warning messages
   -Wextra                        # Enable additional warning messages
   -qopenmp-simd                  # Add support for OpenMP
   -static-intel                  # Static link compiler dependencies
   -fvectorize                    # Enable loop vectorization
   -fslp-vectorize                # Enable SLP vectorization
   -fp-model=precise              # Standard-compliant floating point math
  )
  add_link_options(
   -Wl,--gc-sections              # Remove unused sections during linking
   -flto=full                     # Link-time optimization
  )

 # Generic UNIX/Linux settings
 elseif(UNIX)
  add_compile_options(
   -O3                            # Optimize for speed
   -Wall                          # Enable most warning messages
   -Wextra                        # Enable additional warning messages
   -fopenmp                       # Add support for OpenMP
   -ftree-vectorize               # Enable loop vectorization
   -fno-fast-math                 # Standard-compliant floating point math
   -fno-unsafe-math-optimizations # Use safe math only
  )
  add_link_options(
   -Wl,--gc-sections              # Remove unused sections during linking
   -flto                          # Link-time optimization
  )
 endif()
endif()

#-------------------------------------------------------------------------------
# Sanitizer flags
#-------------------------------------------------------------------------------

if(DEBUG_SANITIZER)
 add_compile_options(
  -fsanitize=address              # Enable AddressSanitizer
  -fsanitize=undefined            # Enable UndefinedBehaviorSanitizer
  -fsanitize=leak                 # Enable LeakSanitizer
  -g                              # Generate debug symbols
  -fno-omit-frame-pointer         # Preserve frame pointers
 )

 add_link_options(
  -fsanitize=address              # Link AddressSanitizer
  -fsanitize=undefined            # Link UndefinedBehaviorSanitizer
  -fsanitize=leak                 # Link LeakSanitizer
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

