#
# Serial Studio
# https://serial-studio.com/
#
# Copyright (C) 2020–2025 Alex Spataru
#
# This file is dual-licensed:
#
# - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
# - Under the Serial Studio Commercial License for builds that include
#   any Pro functionality.
#
# You must comply with the terms of one of these licenses, depending
# on your use case.
#
# For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
# For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
#
# SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
#

cmake_minimum_required(VERSION 3.20)

#-------------------------------------------------------------------------------
# Define project name & find Qt packages for correct CPack calls
#-------------------------------------------------------------------------------

project(Serial-Studio LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
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

option(BUILD_GPL3              "Force GPLv3-only build"                 ON)
option(DEBUG_SANITIZER         "Enable sanitizers for debug builds"    OFF)
option(PRODUCTION_OPTIMIZATION "Enable production optimization flags"  OFF)
option(BUILD_COMMERCIAL        "Enable commercial features"            OFF)

set(ARCGIS_API_KEY $ENV{ARCGIS_API_KEY} CACHE STRING "API Key for ArcGIS map")
set(SERIAL_STUDIO_LICENSE_KEY $ENV{SERIAL_STUDIO_LICENSE_KEY} CACHE STRING "License key for commercial build")
set(SERIAL_STUDIO_INSTANCE_ID $ENV{SERIAL_STUDIO_INSTANCE_ID} CACHE STRING "Instance ID for license validation")

#-------------------------------------------------------------------------------
# Project information
#-------------------------------------------------------------------------------

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

if(BUILD_COMMERCIAL)
   set(PROJECT_DISPNAME "Serial Studio Pro")
   set(PROJECT_FILE_LICENSE "${PROJECT_ROOT_DIR}/LICENSES/LicenseRef-SerialStudio-Commercial.txt")
   if (UNIX AND NOT APPLE)
      set(PROJECT_EXECUTABLE "serial-studio-pro")
   else()
      set(PROJECT_EXECUTABLE "Serial-Studio-Pro")
   endif()
else()
   set(PROJECT_DISPNAME "Serial Studio GPLv3")
   set(PROJECT_FILE_LICENSE "${PROJECT_ROOT_DIR}/LICENSES/GPL-3.0-only.txt")
   if (UNIX AND NOT APPLE)
      set(PROJECT_EXECUTABLE "serial-studio-gpl3")
   else()
      set(PROJECT_EXECUTABLE "Serial-Studio-GPL3")
   endif()
endif()

set(PROJECT_VENDOR              "Alex Spataru")
set(PROJECT_CONTACT             "serial-studio.github.io")
set(PROJECT_DESCRIPTION_SUMMARY "Flexible data visualization software for embedded devices and projects")
set(PROJECT_VERSION_MAJOR       "3")
set(PROJECT_VERSION_MINOR       "1")
set(PROJECT_VERSION_PATCH       "10")
set(PROJECT_VERSION             "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(PROJECT_APPCAST             "https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/master/updates.json")
set(PROJECT_DESCRIPTION_FILE    "${PROJECT_ROOT_DIR}/README.md")
set(PROJECT_FILE_NAME           "${PROJECT_EXECUTABLE}-v${PROJECT_VERSION}")

#-------------------------------------------------------------------------------
# Allow source code to access project information
#-------------------------------------------------------------------------------

add_definitions(-DPROJECT_VENDOR="${PROJECT_VENDOR}")
add_definitions(-DPROJECT_CONTACT="${PROJECT_CONTACT}")
add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
add_definitions(-DPROJECT_APPCAST="${PROJECT_APPCAST}")
add_definitions(-DPROJECT_DISPNAME="${PROJECT_DISPNAME}")

#-------------------------------------------------------------------------------
# Licensing and validation
#-------------------------------------------------------------------------------

if(BUILD_GPL3)
   if(BUILD_COMMERCIAL)
      message(FATAL_ERROR
         "You cannot enable commercial features (BUILD_COMMERCIAL=ON) when BUILD_GPL3=ON.\n"
         "Set -DBUILD_GPL3=OFF to build with commercial modules.")
   endif()

   set(BUILD_COMMERCIAL OFF CACHE BOOL "Commercial features disabled due to GPLv3 build mode" FORCE)
   message(STATUS "BUILD_GPL3=ON — enforcing BUILD_COMMERCIAL=OFF for license compliance")
endif()

if(BUILD_COMMERCIAL)
    add_compile_definitions(BUILD_COMMERCIAL)
    add_definitions(-DARCGIS_API_KEY="${ARCGIS_API_KEY}")
    message(STATUS "BUILD_COMMERCIAL=ON — validating license with Lemon Squeezy")

    if(NOT SERIAL_STUDIO_LICENSE_KEY OR NOT SERIAL_STUDIO_INSTANCE_ID)
        message(FATAL_ERROR
            "Missing SERIAL_STUDIO_LICENSE_KEY or SERIAL_STUDIO_INSTANCE_ID.\n"
            "Pass them via -D flags or set them as environment variables.")
    endif()

    find_program(CURL_EXECUTABLE curl)
    if(NOT CURL_EXECUTABLE)
        message(FATAL_ERROR "curl not found in PATH — required to validate license.")
    endif()

    execute_process(
        COMMAND curl -s -X POST https://api.lemonsqueezy.com/v1/licenses/validate
        -H "Accept: application/vnd.api+json"
        -H "Content-Type: application/vnd.api+json"
        -d "{\"license_key\": \"${SERIAL_STUDIO_LICENSE_KEY}\", \"instance_id\": \"${SERIAL_STUDIO_INSTANCE_ID}\"}"
        OUTPUT_VARIABLE LS_RESPONSE
        ERROR_VARIABLE LS_ERROR
        RESULT_VARIABLE LS_RESULT
    )

    if(NOT LS_RESULT EQUAL 0)
        message(FATAL_ERROR "License validation failed: ${LS_ERROR}")
    endif()

    string(FIND "${LS_RESPONSE}" "\"valid\":true" VALID_POS)
    if(VALID_POS EQUAL -1)
        message(FATAL_ERROR
            "Commercial build denied: License not valid or not active.\n"
            "Response: ${LS_RESPONSE}")
    endif()

    message(STATUS "License validation succeeded — building with commercial features")
endif()

#-------------------------------------------------------------------------------
# Log CPU type
#-------------------------------------------------------------------------------

message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")

#-------------------------------------------------------------------------------
# Set UNIX friendly name for app & fix OpenSUSE builds
#-------------------------------------------------------------------------------

if (UNIX AND NOT APPLE)
   set(CMAKE_C_COMPILER_AR "/usr/bin/ar")
   set(CMAKE_CXX_COMPILER_AR "/usr/bin/ar")
   set(CMAKE_C_COMPILER_RANLIB "/usr/bin/ranlib")
   set(CMAKE_CXX_COMPILER_RANLIB "/usr/bin/ranlib")
endif()

#-------------------------------------------------------------------------------
# Set compiler warnings (MSVC excluded... it’s allergic to standards)
#-------------------------------------------------------------------------------

if(NOT MSVC)
  add_compile_options(
    -Wall                          # Enable common warning messages
    -Wextra                        # Enable extra warnings
    -Wno-unused-function           # Silence miniaudio warnings
    -Wno-cast-align                # Silence KISS FFT warnings
  )
endif()

#-------------------------------------------------------------------------------
# Production optimization flags
#-------------------------------------------------------------------------------

if(PRODUCTION_OPTIMIZATION)
   # Print status
   message("Enabling production optimization flags...")

   # LLVM (MinGW) on Windows
   if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW)
      add_compile_options(
         -O2                             # Optimize for speed
         -ftree-vectorize                # Enable automatic vectorization
         -funroll-loops                  # Improve loop performance by unrolling
         -fomit-frame-pointer            # Optimize function call overhead
         -fno-fast-math                  # IEEE-compliant floating point
         -fno-unsafe-math-optimizations  # Avoid unsafe math assumptions
         -finline-functions              # Inline functions where possible
         -ffunction-sections             # Place each function in its own section
         -fdata-sections                 # Place each data item in its own section
      )
      add_link_options(
         -Wl,--gc-sections               # Remove unused functions/data from final binary
      )
   
   # GCC (MinGW) on Windows
   elseif(WIN32 AND MINGW)
      add_compile_options(
         -O2                             # Optimize for speed
         -ftree-vectorize                # Enable automatic vectorization
         -funroll-loops                  # Improve loop performance by unrolling
         -fomit-frame-pointer            # Optimize function call overhead
         -frename-registers              # Better instruction scheduling
         -fno-fast-math                  # IEEE-compliant floating point
         -fno-unsafe-math-optimizations  # Avoid unsafe math assumptions
         -fstack-reuse=all               # Reuse stack slots
         -finline-functions              # Inline functions where possible
         -ffunction-sections             # Place each function in its own section
         -fdata-sections                 # Place each data item in its own section
      )
      add_link_options(
         -Wl,--gc-sections               # Remove unused code/data at link time
      )

   # MSVC (Visual Studio)
   elseif(WIN32 AND MSVC)
      add_compile_options(
         /permissive-                    # Enforce strict ISO C++ conformance
         /Zc:__cplusplus                 # Set __cplusplus to actual version value
         /Zc:preprocessor                # Standards-compliant preprocessor
         /MP                             # Enable multi-core compilation
         /O2                             # Optimize for speed
         /W4                             # High warning level
         /GL                             # Enable whole program optimization
         /fp:precise                     # IEEE-accurate floating-point math
         /Qpar                           # Enable automatic parallelization of loops
         /Gw                             # Put functions/data into separate COMDATs
      )
      add_link_options(
         /OPT:REF                        # Remove unreferenced functions/data
         /OPT:ICF                        # Merge identical code/data blocks
         /LTCG                           # Link-time optimization
      )

   # macOS (Clang/AppleClang)
   elseif(APPLE)
      add_compile_options(
         -O2                             # Optimize for speed
         -ftree-vectorize                # Enable automatic vectorization
         -funroll-loops                  # Improve loop performance by unrolling
         -fomit-frame-pointer            # Optimize function call overhead
         -fno-fast-math                  # IEEE-compliant floating point
         -fno-unsafe-math-optimizations  # Avoid unsafe math assumptions
         -flto                           # Enable Link-Time optimization
         -finline-functions              # Inline functions where possible
         -ffunction-sections             # Place each function in its own section
         -fdata-sections                 # Place each data item in its own section
      )
      add_link_options(
         -Wl,-dead_strip                 # Remove unused functions/data from final binary
         -flto                           # Enable Link-Time Optimization
      )

   # Intel LLVM Compiler (Linux or Windows)
   elseif(CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
      add_compile_options(
         -O2                             # Optimize for speed
         -static                         # Include Intel dependencies
         -ftree-vectorize                # Enable automatic vectorization
         -funroll-loops                  # Improve loop performance by unrolling
         -fomit-frame-pointer            # Optimize function call overhead
         -frename-registers              # Better instruction scheduling
         -fno-fast-math                  # IEEE-compliant floating point
         -fno-unsafe-math-optimizations  # Avoid unsafe math assumptions
         -fstack-reuse=all               # Reuse stack slots
         -finline-functions              # Inline functions where possible
         -ffunction-sections             # Place each function in its own section
         -fdata-sections                 # Place each data item in its own section
      )
      add_link_options(
         -Wl,--gc-sections               # Discard unused code sections at link time
      )

   # Generic Linux (GCC or Clang)
   elseif(UNIX)
      add_compile_options(
         -O2                             # Optimize for speed
         -ftree-vectorize                # Enable automatic vectorization
         -funroll-loops                  # Improve loop performance by unrolling
         -fomit-frame-pointer            # Optimize function call overhead
         -frename-registers              # Better instruction scheduling
         -fno-fast-math                  # IEEE-compliant floating point
         -fno-unsafe-math-optimizations  # Avoid unsafe math assumptions
         -fstack-reuse=all               # Reuse stack slots
         -finline-functions              # Inline functions where possible
         -ffunction-sections             # Place each function in its own section
         -fdata-sections                 # Place each data item in its own section
      )
      add_link_options(
         -Wl,--gc-sections               # Strip unused functions/data
      )
      if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
         add_link_options(-latomic)      # Required for MiniAudio
      endif()
   endif()
else()
   message("Disabling production optimization flags...")
endif()

#-------------------------------------------------------------------------------
# Enable platform-specific SIMD
#-------------------------------------------------------------------------------

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64)$")
  message(STATUS "Enabling SSE4.1 optimizations")
  if(MSVC)
    add_compile_options(/arch:SSE)
  else()
    add_compile_options(-msse4.1)
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

