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
# Project definition and C++ standard
#-------------------------------------------------------------------------------
#
# Sets up the project name and C++ language standard requirements:
# - Project name: Serial-Studio
# - Language: C++ only (no C or other languages)
# - C++ standard: C++20 (required for modern language features)
# - Standard compliance: Strictly required (no compiler extensions fallback)
#
# Qt6 Core and Qml packages are found early to ensure correct CPack calls
# when generating installers and packages later in the build process.
#
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
# Build configuration options
#-------------------------------------------------------------------------------
#
# Available build options and their default values:
#
# Licensing:
#   BUILD_GPL3=ON (default)        - Build GPLv3-only version
#   BUILD_COMMERCIAL=OFF (default) - Enable Pro features (requires license)
#
# Performance:
#   PRODUCTION_OPTIMIZATION=OFF (default) - Enable O3, LTO, vectorization
#   ENABLE_PGO=OFF (default)              - Profile-Guided Optimization
#
# Security:
#   ENABLE_HARDENING=OFF (default) - Security hardening flags (FORTIFY, RELRO)
#
# Debugging:
#   DEBUG_SANITIZER=OFF (default) - AddressSanitizer + UBSan for debug builds
#
# Dependencies:
#   USE_SYSTEM_ZLIB=OFF (default)  - Use system zlib (required for Flathub)
#   USE_SYSTEM_EXPAT=OFF (default) - Use system expat (required for Flathub)
#
# Note: USE_SYSTEM_ZLIB or USE_SYSTEM_EXPAT auto-enables ENABLE_HARDENING
#
#-------------------------------------------------------------------------------

option(BUILD_GPL3              "Force GPLv3-only build"                 ON)
option(DEBUG_SANITIZER         "Enable sanitizers for debug builds"    OFF)
option(PRODUCTION_OPTIMIZATION "Enable production optimization flags"  OFF)
option(ENABLE_HARDENING        "Enable security hardening flags"       OFF)
option(ENABLE_PGO              "Enable Profile-Guided Optimization"    OFF)
option(BUILD_COMMERCIAL        "Enable commercial features"            OFF)
option(USE_SYSTEM_ZLIB         "Use system-provided zlib"              OFF)
option(USE_SYSTEM_EXPAT        "Use system-provided expat"             OFF)

#-------------------------------------------------------------------------------
# Commercial build credentials
#-------------------------------------------------------------------------------
#
# Commercial builds require valid API keys and license credentials:
#
# ARCGIS_API_KEY:
#   - API key for ArcGIS mapping features in Pro version
#   - Can be set via environment variable or -D flag
#   - Only required when BUILD_COMMERCIAL=ON
#
# SERIAL_STUDIO_LICENSE_KEY:
#   - License key for Serial Studio Pro (from Lemon Squeezy)
#   - Required for commercial builds
#   - Validated against Lemon Squeezy API at configure time
#
# SERIAL_STUDIO_INSTANCE_ID:
#   - Unique instance identifier for license activation
#   - Required for commercial builds
#   - Used with license key for validation
#
# These can be passed via:
#   - Environment variables: export SERIAL_STUDIO_LICENSE_KEY="..."
#   - CMake flags: -DSERIAL_STUDIO_LICENSE_KEY="..."
#
#-------------------------------------------------------------------------------

set(ARCGIS_API_KEY
  $ENV{ARCGIS_API_KEY}
  CACHE STRING "API Key for ArcGIS map")
set(SERIAL_STUDIO_LICENSE_KEY
  $ENV{SERIAL_STUDIO_LICENSE_KEY}
  CACHE STRING "License key for commercial build")
set(SERIAL_STUDIO_INSTANCE_ID
  $ENV{SERIAL_STUDIO_INSTANCE_ID}
  CACHE STRING "Instance ID for license validation")

#-------------------------------------------------------------------------------
# Project metadata and branding
#-------------------------------------------------------------------------------
#
# Configures project metadata used throughout the build system:
#
# Version control:
#   - PROJECT_VERSION_MAJOR: Major version (breaking changes)
#   - PROJECT_VERSION_MINOR: Minor version (new features)
#   - PROJECT_VERSION_PATCH: Patch version (bug fixes)
#   - PROJECT_VERSION: Combined semantic version string
#
# Branding (varies by BUILD_COMMERCIAL flag):
#   Commercial build:
#     - Display name: "Serial Studio Pro"
#     - Executable: Serial-Studio-Pro (or serial-studio-pro on Linux)
#     - License: Commercial license file
#   GPL build:
#     - Display name: "Serial Studio GPLv3"
#     - Executable: Serial-Studio-GPL3 (or serial-studio-gpl3 on Linux)
#     - License: GPL-3.0-only.txt
#
# Other metadata:
#   - PROJECT_VENDOR: Author/copyright holder
#   - PROJECT_CONTACT: Website for support
#   - PROJECT_DESCRIPTION_SUMMARY: Short description for installers
#   - PROJECT_APPCAST: URL for auto-update checks (QSimpleUpdater)
#   - PROJECT_DESCRIPTION_FILE: README for package metadata
#   - PROJECT_FILE_NAME: Base filename for installers/packages
#
# Note: Linux uses lowercase executable names following FHS conventions
#
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
set(PROJECT_VERSION_MINOR       "2")
set(PROJECT_VERSION_PATCH       "3")
set(PROJECT_VERSION             "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(PROJECT_APPCAST             "https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/master/updates.json")
set(PROJECT_DESCRIPTION_FILE    "${PROJECT_ROOT_DIR}/README.md")
set(PROJECT_FILE_NAME           "${PROJECT_EXECUTABLE}-v${PROJECT_VERSION}")

#-------------------------------------------------------------------------------
# Expose project metadata to source code
#-------------------------------------------------------------------------------
#
# Makes project metadata available as preprocessor definitions in C++ code:
#
# Available macros:
#   - PROJECT_VENDOR: Displayed in About dialog and copyright notices
#   - PROJECT_CONTACT: Used for support links in UI
#   - PROJECT_VERSION: Displayed in title bar, About dialog, logs
#   - PROJECT_APPCAST: URL for QSimpleUpdater to check for new versions
#   - PROJECT_DISPNAME: Application display name in UI
#   - PROJECT_DESCRIPTION_SUMMARY: Short description for About/Help
#
# These can be accessed in C++ as string literals:
#   const char *version = PROJECT_VERSION;
#   const char *name = PROJECT_DISPNAME;
#
#-------------------------------------------------------------------------------

add_definitions(-DPROJECT_VENDOR="${PROJECT_VENDOR}")
add_definitions(-DPROJECT_CONTACT="${PROJECT_CONTACT}")
add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
add_definitions(-DPROJECT_APPCAST="${PROJECT_APPCAST}")
add_definitions(-DPROJECT_DISPNAME="${PROJECT_DISPNAME}")
add_definitions(-DPROJECT_DESCRIPTION_SUMMARY="${PROJECT_DESCRIPTION_SUMMARY}")

#-------------------------------------------------------------------------------
# License enforcement and validation
#-------------------------------------------------------------------------------
#
# Enforces dual-licensing model and validates commercial licenses:
#
# GPL builds (BUILD_GPL3=ON):
#   - Ensures BUILD_COMMERCIAL is OFF for license compliance
#   - Prevents accidental inclusion of Pro features in GPL builds
#   - No license validation required
#
# Commercial builds (BUILD_COMMERCIAL=ON):
#   - Adds BUILD_COMMERCIAL preprocessor definition to enable Pro features
#   - Requires valid SERIAL_STUDIO_LICENSE_KEY and SERIAL_STUDIO_INSTANCE_ID
#   - Validates license against Lemon Squeezy API at configure time
#   - Build fails if license is invalid, expired, or not activated
#
# License validation process:
#   1. Check for required credentials (license key + instance ID)
#   2. Verify curl is available in PATH
#   3. POST validation request to Lemon Squeezy API
#   4. Parse JSON response to verify "valid":true
#   5. Fail build if validation fails or returns invalid
#
# API endpoint: https://api.lemonsqueezy.com/v1/licenses/validate
# Documentation: https://docs.lemonsqueezy.com/api/license-keys
#
#-------------------------------------------------------------------------------

if(BUILD_GPL3)
   if(BUILD_COMMERCIAL)
      message(FATAL_ERROR
         "You cannot enable commercial features (BUILD_COMMERCIAL=ON) when BUILD_GPL3=ON.\n"
         "Set -DBUILD_GPL3=OFF to build with commercial modules.")
   endif()

   set(BUILD_COMMERCIAL OFF CACHE BOOL "Commercial features disabled due to GPLv3 build mode" FORCE)
   message(STATUS "BUILD_GPL3=ON, enforcing BUILD_COMMERCIAL=OFF for license compliance")
endif()

if(BUILD_COMMERCIAL)
    add_compile_definitions(BUILD_COMMERCIAL)
    add_definitions(-DARCGIS_API_KEY="${ARCGIS_API_KEY}")
    message(STATUS "BUILD_COMMERCIAL=ON, validating license with Lemon Squeezy")

    if(NOT SERIAL_STUDIO_LICENSE_KEY OR NOT SERIAL_STUDIO_INSTANCE_ID)
        message(FATAL_ERROR
            "Missing SERIAL_STUDIO_LICENSE_KEY or SERIAL_STUDIO_INSTANCE_ID.\n"
            "Pass them via -D flags or set them as environment variables.")
    endif()

    find_program(CURL_EXECUTABLE curl)
    if(NOT CURL_EXECUTABLE)
        message(FATAL_ERROR "curl not found in PATH, required to validate license.")
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

    message(STATUS "License validation succeeded, building with commercial features")
endif()

#-------------------------------------------------------------------------------
# System architecture detection
#-------------------------------------------------------------------------------
#
# Logs the target CPU architecture for debugging and optimization reference:
#   - x86_64 / amd64: 64-bit Intel/AMD processors
#   - aarch64: 64-bit ARM processors (Apple Silicon, Raspberry Pi 4+)
#   - armv7l: 32-bit ARM processors (older Raspberry Pi models)
#
# This information is used later to select appropriate optimization flags
# (e.g., -march=x86-64-v2, -march=armv8-a, SSE4.1, NEON)
#
#-------------------------------------------------------------------------------

message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")

#-------------------------------------------------------------------------------
# Linux toolchain configuration
#-------------------------------------------------------------------------------
#
# Configures archiver and ranlib paths for Linux builds:
#
# Purpose:
#   - Fixes OpenSUSE builds where CMake may not find correct AR/RANLIB paths
#   - Ensures consistent static library creation across distributions
#   - Avoids issues with compiler-specific wrappers (gcc-ar vs ar)
#
# Tools configured:
#   - CMAKE_C_COMPILER_AR: Archive tool for C static libraries
#   - CMAKE_CXX_COMPILER_AR: Archive tool for C++ static libraries
#   - CMAKE_C_COMPILER_RANLIB: Index generator for C static libraries
#   - CMAKE_CXX_COMPILER_RANLIB: Index generator for C++ static libraries
#
# Only applied to Linux (UNIX AND NOT APPLE) to avoid breaking macOS builds
# which use different archiver tools (libtool, ar with different flags)
#
#-------------------------------------------------------------------------------

if (UNIX AND NOT APPLE)
   set(CMAKE_C_COMPILER_AR "/usr/bin/ar")
   set(CMAKE_CXX_COMPILER_AR "/usr/bin/ar")
   set(CMAKE_C_COMPILER_RANLIB "/usr/bin/ranlib")
   set(CMAKE_CXX_COMPILER_RANLIB "/usr/bin/ranlib")
endif()

#-------------------------------------------------------------------------------
# Compiler warnings configuration
#-------------------------------------------------------------------------------
#
# Configures compiler warnings to catch potential bugs while avoiding noise:
#
# MSVC (Visual Studio):
#   /wd4324 - Disable C4324 (structure padding)
#             Triggered by alignment attributes, not actionable
#   /wd4267 - Disable C4267 (size_t to int conversion)
#             Common in Qt code, usually safe in practice
#
# GCC/Clang/MinGW:
#   -Wall     - Enable all common warning categories
#               (uninitialized variables, implicit conversions, etc.)
#   -Wextra   - Enable additional warnings beyond -Wall
#               (unused parameters, sign comparison, etc.)
#   -pedantic - Enforce strict ISO C++ conformance
#               (reject non-standard compiler extensions)
#   -Wno-unused-function - Suppress unused function warnings
#               Required for miniaudio.h single-header library
#               which defines many internal functions
#
# Note: Third-party libraries in lib/ use -w to suppress all warnings
#
#-------------------------------------------------------------------------------

if(MSVC)
  add_compile_options(
    /wd4324                        # Structure padding warning
    /wd4267                        # size_t to int conversion warning
  )
else()
  add_compile_options(
    -Wall                          # Enable common warning messages
    -Wextra                        # Enable extra warnings
    -pedantic                      # Enforce strict ISO C++ conformance
    -Wno-unused-function           # Silence miniaudio warnings
  )
endif()

#-------------------------------------------------------------------------------
# Production optimization flags
#-------------------------------------------------------------------------------
#
# High-performance compilation flags for release builds:
#
# When PRODUCTION_OPTIMIZATION=ON is enabled, the build system applies
# aggressive optimization flags tailored to each compiler and platform:
#
# Common optimizations across all platforms:
#   -O3                    - Maximum optimization level
#   -ftree-vectorize       - Auto-vectorization (SIMD)
#   -funroll-loops         - Loop unrolling for better IPC
#   -fomit-frame-pointer   - Eliminate frame pointer overhead
#   -fno-fast-math         - IEEE 754 compliance (no unsafe math)
#   -finline-functions     - Aggressive function inlining
#   -ffunction-sections    - Separate sections per function
#   -fdata-sections        - Separate sections per data item
#   -flto=auto             - Link-Time Optimization (LTO) with parallelization
#   -Wl,--gc-sections      - Remove unused code/data at link time
#
# Architecture-specific flags:
#   x86-64: -march=x86-64-v2 (SSE4.2, SSSE3, modern CPUs from 2012+)
#   ARM64:  -march=armv8-a (ARMv8 baseline for aarch64)
#   ARMv7:  -march=armv7-a -mfpu=neon (NEON SIMD for 32-bit ARM)
#
# Compiler-specific optimizations:
#   GCC:   -frename-registers, -fstack-reuse=all
#   MSVC:  /GL (whole program optimization), /Gw (COMDAT sections), /Qpar
#   Intel: Same as GCC/Clang with additional Intel-specific opts
#
# Sandboxed builds (Flathub):
#   - Detects USE_SYSTEM_ZLIB or USE_SYSTEM_EXPAT
#   - Disables LTO (causes issues in sandboxed environments)
#   - Auto-enables ENABLE_HARDENING for security compliance
#
# Expected performance gain: 20-40% compared to -O2 without LTO
# Build time impact: +50-100% due to LTO (can be disabled if needed)
#
#-------------------------------------------------------------------------------

if(PRODUCTION_OPTIMIZATION)
   # Detect sandboxed build environments (Flathub) and enable hardening
   if(USE_SYSTEM_ZLIB OR USE_SYSTEM_EXPAT)
      set(DISABLE_LTO ON)
      set(ENABLE_HARDENING ON CACHE BOOL "Auto-enabled hardening for sandboxed builds" FORCE)
      message(STATUS "Sandboxed build detected (USE_SYSTEM_ZLIB or USE_SYSTEM_EXPAT), disabling LTO and enabling hardening")
   else()
      set(DISABLE_LTO OFF)
   endif()

   # Print status
   message("Enabling production optimization flags...")

   # Add NDEBUG definition for all configurations
   add_compile_definitions(NDEBUG)

   # LLVM (MinGW) on Windows
   if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW)
      add_compile_options(
         -O3                             # Aggressive optimization
         -march=x86-64-v2                # Target modern CPUs (2012+) with SSE4.2
         -ftree-vectorize                # Enable automatic vectorization
         -funroll-loops                  # Improve loop performance by unrolling
         -fomit-frame-pointer            # Optimize function call overhead
         -fno-fast-math                  # IEEE-compliant floating point
         -fno-unsafe-math-optimizations  # Avoid unsafe math assumptions
         -finline-functions              # Inline functions where possible
         -ffunction-sections             # Place each function in its own section
         -fdata-sections                 # Place each data item in its own section
      )
      if(NOT DISABLE_LTO)
         add_compile_options(-flto=auto) # Link-time optimization with parallelization
      endif()
      add_link_options(
         -Wl,--gc-sections               # Remove unused functions/data from final binary
      )
      if(NOT DISABLE_LTO)
         add_link_options(-flto=auto)    # Enable LTO at link time
      endif()
   
   # GCC (MinGW) on Windows
   elseif(WIN32 AND MINGW)
      add_compile_options(
         -O3                             # Aggressive optimization
         -march=x86-64-v2                # Target modern CPUs (2012+) with SSE4.2
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
      if(NOT DISABLE_LTO)
         add_compile_options(-flto=auto) # Link-time optimization with parallelization
      endif()
      add_link_options(
         -Wl,--gc-sections               # Remove unused code/data at link time
      )
      if(NOT DISABLE_LTO)
         add_link_options(-flto=auto)    # Enable LTO at link time
      endif()

   # MSVC (Visual Studio)
   elseif(WIN32 AND MSVC)
      add_compile_options(
         /permissive-                    # Enforce strict ISO C++ conformance
         /Zc:__cplusplus                 # Set __cplusplus to actual version value
         /Zc:preprocessor                # Standards-compliant preprocessor
         /MP                             # Enable multi-core compilation
         /O2                             # Optimize for speed
         /Ot                             # Favor fast code over small code
         /W4                             # High warning level
         /fp:precise                     # IEEE-accurate floating-point math
         /Qpar                           # Enable automatic parallelization of loops
         /Gw                             # Put functions/data into separate COMDATs
         /DNDEBUG                        # Disable assertions
      )
      if(NOT DISABLE_LTO)
         add_compile_options(/GL)        # Enable whole program optimization
      endif()
      add_link_options(
         /OPT:REF                        # Remove unreferenced functions/data
         /OPT:ICF                        # Merge identical code/data blocks
      )
      if(NOT DISABLE_LTO)
         add_link_options(/LTCG)         # Link-time optimization
      endif()

   # macOS (Clang/AppleClang)
   elseif(APPLE)
      add_compile_options(
         -O3                             # Aggressive optimization
         -ftree-vectorize                # Enable automatic vectorization
         -funroll-loops                  # Improve loop performance by unrolling
         -fomit-frame-pointer            # Optimize function call overhead
         -fno-fast-math                  # IEEE-compliant floating point
         -fno-unsafe-math-optimizations  # Avoid unsafe math assumptions
         -finline-functions              # Inline functions where possible
         -ffunction-sections             # Place each function in its own section
         -fdata-sections                 # Place each data item in its own section
      )
      if(NOT DISABLE_LTO)
         add_compile_options(-flto=auto) # Link-time optimization with parallelization
      endif()
      add_link_options(
         -Wl,-dead_strip                 # Remove unused functions/data from final binary
      )
      if(NOT DISABLE_LTO)
         add_link_options(-flto=auto)    # Enable Link-Time Optimization
      endif()

      if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
         add_compile_options(-march=x86-64-v2)
      endif()

   # Intel LLVM Compiler (Linux or Windows)
   elseif(CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
      add_compile_options(
         -O3                             # Aggressive optimization
         -march=x86-64-v2                # Target modern CPUs (2012+) with SSE4.2
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
      if(NOT DISABLE_LTO)
         add_compile_options(-flto=auto) # Link-time optimization with parallelization
      endif()
      add_link_options(
         -Wl,--gc-sections               # Discard unused code sections at link time
      )
      if(NOT DISABLE_LTO)
         add_link_options(-flto=auto)    # Enable LTO at link time
      endif()

   # Generic Linux (GCC or Clang)
   elseif(UNIX)
      add_compile_options(
         -O3                             # Aggressive optimization
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
      if(NOT DISABLE_LTO)
         add_compile_options(-flto=auto) # Link-time optimization with parallelization
      endif()
      add_link_options(
         -Wl,--gc-sections               # Strip unused functions/data
      )
      if(NOT DISABLE_LTO)
         add_link_options(-flto=auto)    # Enable LTO at link time
      endif()

      if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
         add_compile_options(-march=x86-64-v2)
      elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
         add_compile_options(-march=armv8-a)
         add_link_options(-latomic)
      elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
         add_compile_options(-march=armv7-a -mfpu=neon)
         add_link_options(-latomic)
      endif()
   endif()
else()
   message("Disabling production optimization flags...")
endif()

#-------------------------------------------------------------------------------
# Platform-specific SIMD instructions
#-------------------------------------------------------------------------------
#
# Enables SIMD (Single Instruction Multiple Data) optimizations for x86-64:
#
# SSE4.1 support:
#   - Enabled on x86-64 processors (Intel/AMD 64-bit)
#   - Provides vectorized operations for DSP, graphics, data processing
#   - Required by some Qt 6 modules and KissFFT optimizations
#
# Why SSE4.1?
#   - Widely available (Intel since Core 2, AMD since Bulldozer)
#   - Safe baseline for modern systems (2008+ CPUs)
#   - Significantly faster than SSE2 for floating-point operations
#
# Platform notes:
#   - MSVC x64: SSE2 is always enabled (x64 ABI requirement), no flag needed
#   - GCC/Clang: Must explicitly enable SSE4.1 with -msse4.1
#   - ARM: SIMD handled separately via NEON (see PRODUCTION_OPTIMIZATION)
#
# Impact:
#   - ~20-30% performance boost for FFT calculations
#   - ~10-15% improvement in signal processing pipelines
#
#-------------------------------------------------------------------------------

if(NOT MSVC AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
  message(STATUS "Enabling SSE4.1 optimizations")
  add_compile_options(-msse4.1)
endif()

#-------------------------------------------------------------------------------
# Debug sanitizers (AddressSanitizer + UndefinedBehaviorSanitizer)
#-------------------------------------------------------------------------------
#
# Runtime error detection for debug builds:
#
# When DEBUG_SANITIZER=ON is enabled (debug builds only):
#
# AddressSanitizer (ASan):
#   - Detects memory errors: use-after-free, buffer overflows, leaks
#   - Adds instrumentation to memory operations (~2x slowdown)
#   - Requires ~2-3x more memory than normal execution
#
# UndefinedBehaviorSanitizer (UBSan):
#   - Detects undefined behavior: null pointer dereference, integer overflow
#   - Minimal performance impact (~10-20% slowdown)
#
# Additional flags:
#   -g                      - Debug symbols for stack traces
#   -fno-omit-frame-pointer - Preserve frame pointers for accurate traces
#
# Usage:
#   cmake .. -DDEBUG_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
#   ./Serial-Studio-GPL3
#   # If errors are found, ASan/UBSan will print detailed reports to stderr
#
# Note: Only supported on GCC/Clang. Not compatible with MSVC.
# Not recommended for production builds due to performance overhead.
#
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
# Profile-Guided Optimization (PGO)
#-------------------------------------------------------------------------------
#
# PGO is a two-stage optimization process:
#
# Stage 1 - GENERATE (build with instrumentation):
#   cmake .. -DENABLE_PGO=ON -DPGO_STAGE=GENERATE -DCMAKE_BUILD_TYPE=Release
#   cmake --build . -j$(nproc)
#
# Stage 2 - PROFILE (run the application with typical workloads):
#   ./Serial-Studio-GPL3  # Use the app normally to generate profile data
#   # Profile data (.gcda files for GCC, .profraw for Clang) is written to build dir
#
# Stage 3 - USE (rebuild with profile data):
#   cmake .. -DENABLE_PGO=ON -DPGO_STAGE=USE -DCMAKE_BUILD_TYPE=Release
#   cmake --build . -j$(nproc)
#
# Expected performance gain: 10-20% for typical workloads
#-------------------------------------------------------------------------------

set(PGO_STAGE "GENERATE" CACHE STRING "PGO stage: GENERATE or USE")
set(PGO_PROFILE_DIR "${CMAKE_BINARY_DIR}/pgo-profiles" CACHE STRING "Directory for PGO profile data")

if(ENABLE_PGO)
   if(NOT PRODUCTION_OPTIMIZATION)
      message(WARNING "PGO requires PRODUCTION_OPTIMIZATION=ON for best results")
   endif()

   # Create profile directory if it doesn't exist
   file(MAKE_DIRECTORY ${PGO_PROFILE_DIR})

   if(PGO_STAGE STREQUAL "GENERATE")
      message(STATUS "PGO: GENERATE stage - building with profile instrumentation")
      message(STATUS "PGO: Profile data will be written to: ${PGO_PROFILE_DIR}")
      message(STATUS "PGO: After running the app, rebuild with -DPGO_STAGE=USE")

      if(MSVC)
         add_compile_options(/GL)                           # Whole program optimization
         add_link_options(
            /LTCG                                           # Link-time code generation
            /GENPROFILE                                     # Generate profile data
            /PGD:${PGO_PROFILE_DIR}/profile.pgd             # Profile database location
         )
      elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
         add_compile_options(
            -fprofile-generate=${PGO_PROFILE_DIR}          # Generate profile data (Clang)
         )
         add_link_options(
            -fprofile-generate=${PGO_PROFILE_DIR}          # Link with profiling
         )
      elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
         add_compile_options(
            -fprofile-generate                             # Generate profile data (GCC)
            -fprofile-dir=${PGO_PROFILE_DIR}               # Profile data directory
         )
         add_link_options(
            -fprofile-generate                             # Link with profiling
            -fprofile-dir=${PGO_PROFILE_DIR}               # Profile data directory
         )
      else()
         message(FATAL_ERROR "PGO not supported for compiler: ${CMAKE_CXX_COMPILER_ID}")
      endif()

   elseif(PGO_STAGE STREQUAL "USE")
      message(STATUS "PGO: USE stage - building with profile optimization")
      message(STATUS "PGO: Using profile data from: ${PGO_PROFILE_DIR}")

      # Check if profile data exists
      if(MSVC)
         if(NOT EXISTS "${PGO_PROFILE_DIR}/profile.pgd")
            message(FATAL_ERROR "PGO profile data not found at ${PGO_PROFILE_DIR}/profile.pgd\nRun with -DPGO_STAGE=GENERATE first")
         endif()
      elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
         # Clang stores .profraw files that need to be merged into .profdata
         file(GLOB PROFRAW_FILES "${PGO_PROFILE_DIR}/*.profraw")
         if(NOT PROFRAW_FILES)
            message(FATAL_ERROR "No .profraw files found in ${PGO_PROFILE_DIR}\nRun the instrumented binary first")
         endif()

         # Merge profile data
         find_program(LLVM_PROFDATA llvm-profdata)
         if(NOT LLVM_PROFDATA)
            message(FATAL_ERROR "llvm-profdata not found. Install LLVM tools to use PGO with Clang")
         endif()

         set(PROFDATA_FILE "${PGO_PROFILE_DIR}/merged.profdata")
         if(NOT EXISTS ${PROFDATA_FILE})
            message(STATUS "Merging profile data: ${PROFRAW_FILES}")
            execute_process(
               COMMAND ${LLVM_PROFDATA} merge -output=${PROFDATA_FILE} ${PROFRAW_FILES}
               RESULT_VARIABLE MERGE_RESULT
            )
            if(NOT MERGE_RESULT EQUAL 0)
               message(FATAL_ERROR "Failed to merge profile data")
            endif()
         endif()
      elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
         file(GLOB GCDA_FILES "${PGO_PROFILE_DIR}/*.gcda")
         if(NOT GCDA_FILES)
            message(FATAL_ERROR "No .gcda files found in ${PGO_PROFILE_DIR}\nRun the instrumented binary first")
         endif()
      endif()

      if(MSVC)
         add_compile_options(/GL)                           # Whole program optimization
         add_link_options(
            /LTCG                                           # Link-time code generation
            /USEPROFILE:PGD=${PGO_PROFILE_DIR}/profile.pgd  # Use profile data
         )
      elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
         add_compile_options(
            -fprofile-use=${PROFDATA_FILE}                 # Use merged profile data
         )
         add_link_options(
            -fprofile-use=${PROFDATA_FILE}                 # Link with profile optimization
         )
      elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
         add_compile_options(
            -fprofile-use                                  # Use profile data (GCC)
            -fprofile-dir=${PGO_PROFILE_DIR}               # Profile data directory
            -fprofile-correction                           # Handle profile inconsistencies
         )
         add_link_options(
            -fprofile-use                                  # Link with profile optimization
            -fprofile-dir=${PGO_PROFILE_DIR}               # Profile data directory
         )
      endif()

   else()
      message(FATAL_ERROR "Invalid PGO_STAGE: ${PGO_STAGE}. Must be GENERATE or USE")
   endif()
endif()

#-------------------------------------------------------------------------------
# Security hardening flags
#-------------------------------------------------------------------------------
#
# Enables defense-in-depth security features for production builds:
#
# When ENABLE_HARDENING=ON is enabled:
#
# Stack protection:
#   -fstack-protector-strong    - Stack canaries for functions with buffers
#   /GS (MSVC)                  - Buffer security checks
#
# Memory protection:
#   -D_FORTIFY_SOURCE=2         - Runtime buffer overflow checks (requires -O1+)
#   -fstack-clash-protection    - Prevent stack clash attacks (GCC 8+/Clang 6+, Linux only)
#
# Control-flow integrity:
#   -fcf-protection=full        - Intel CET (x86-64 Linux only, GCC/Clang)
#   -fsanitize=cfi              - Clang CFI (ARM64 Linux only, requires LTO)
#   /guard:cf (MSVC)            - Control Flow Guard
#   Note: AppleClang does not support -fsanitize=cfi or -fcf-protection
#
# ASLR (Address Space Layout Randomization):
#   -fPIC                       - Position Independent Code (Linux/macOS)
#   /DYNAMICBASE (MSVC)         - ASLR support
#
# RELRO (Relocation Read-Only):
#   -Wl,-z,relro                - Make GOT read-only after relocation
#   -Wl,-z,now                  - Resolve all symbols at startup (full RELRO)
#
# DEP (Data Execution Prevention):
#   -Wl,-z,noexecstack          - Mark stack as non-executable
#   /NXCOMPAT (MSVC)            - DEP support is enabled by default
#
# Additional security:
#   -Wl,-z,separate-code        - Separate code segments (Linux only)
#   -fno-plt                    - Avoid PLT for better performance with PIC
#   -Wformat-security           - Warn about unsafe format strings
#
# Auto-enabled when:
#   - USE_SYSTEM_ZLIB=ON or USE_SYSTEM_EXPAT=ON (Flathub builds)
#   - Manually set via -DENABLE_HARDENING=ON
#
# Note: Some flags require optimization (-O1 or higher) to be effective
#
#-------------------------------------------------------------------------------

if(ENABLE_HARDENING)
   message(STATUS "Enabling security hardening flags for production builds")

   if(MSVC)
      add_compile_options(
         /GS                            # Buffer security check
         /guard:cf                      # Control Flow Guard
         /DYNAMICBASE                   # Address Space Layout Randomization
      )
      add_link_options(
         /GUARD:CF                      # Control Flow Guard at link time
         /DYNAMICBASE                   # ASLR
      )
   else()
      add_compile_options(
         -fstack-protector-strong       # Stack canary protection
         -Wformat                       # Format string warnings
         -Wformat-security              # Format security warnings
         -Werror=format-security        # Make format security errors
      )

      # Stack clash protection (GCC 8+ and Clang 6+, not supported on macOS)
      if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT APPLE)
         add_compile_options(-fstack-clash-protection)
      endif()

      # FORTIFY_SOURCE requires optimization
      if(PRODUCTION_OPTIMIZATION OR CMAKE_BUILD_TYPE STREQUAL "Release")
         add_compile_options(-D_FORTIFY_SOURCE=2)
      elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
         add_compile_options(-D_FORTIFY_SOURCE=2)
      else()
         message(WARNING "FORTIFY_SOURCE disabled: requires optimization flags (-O1 or higher)")
      endif()

      # Control-flow integrity (platform-specific):
      # - x86-64 Linux: Intel CET (Control-flow Enforcement Technology) via -fcf-protection
      # - ARM64 Linux:  Clang CFI (Control Flow Integrity) via -fsanitize=cfi
      # Note: ARM64 CFI requires LTO and is only available with upstream LLVM Clang
      # Note: AppleClang does not support -fsanitize=cfi or -fcf-protection, skip on macOS
      if(NOT APPLE)
         if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
            add_compile_options(
               -fcf-protection=full        # Control-flow protection (Intel CET)
            )
         elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
            if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT DISABLE_LTO)
               add_compile_options(
                  -fsanitize=cfi            # Control Flow Integrity for ARM64
                  -fvisibility=hidden       # Required for CFI (symbols must be hidden)
               )
               add_link_options(
                  -fsanitize=cfi            # Link with CFI support
               )
            endif()
         endif()
      endif()

      if(UNIX AND NOT APPLE)
         add_compile_options(
            -fPIC                        # Position Independent Code
            -fno-plt                     # Avoid PLT for better performance with PIC
         )
         add_link_options(
            -Wl,-z,relro                 # Make relocation sections read-only after relocation
            -Wl,-z,now                   # Resolve all symbols at startup (full RELRO)
            -Wl,-z,noexecstack           # Mark stack as non-executable
            -Wl,-z,separate-code         # Separate code segments for better security
         )
      elseif(APPLE)
         add_compile_options(
            -fPIC                        # Position Independent Executable
            -fno-plt                     # Avoid PLT for better performance
         )
      endif()
   endif()
endif()

#-------------------------------------------------------------------------------
# Subdirectory build order
#-------------------------------------------------------------------------------
#
# Builds project components in dependency order:
#
# 1. lib/ (third-party libraries and dependencies):
#    - KissFFT: Fast Fourier Transform library for FFT plots
#    - QCodeEditor: Syntax-highlighted code editor for JSON/JS
#    - QSimpleUpdater: Auto-update framework
#    - mdflib: MDF4/MF4 file format parser
#    - OpenSSL: Cryptography and SSL/TLS (static linking)
#    - ZLIB: Compression library (FetchContent or system)
#    - EXPAT: XML parser (FetchContent or system)
#
# 2. app/ (main application):
#    - Serial Studio executable (links against lib/ targets)
#    - QML UI files and resources
#    - Platform-specific deployment (Windows RC, macOS plist, Linux desktop)
#
# Note: lib/ must be built first because app/ depends on its targets
#
#-------------------------------------------------------------------------------

add_subdirectory(lib)
add_subdirectory(app)

#-------------------------------------------------------------------------------
# Build configuration summary
#-------------------------------------------------------------------------------
#
# Logs active compiler flags for debugging and verification:
#
# LIB Compile Options:
#   - Shows flags applied to third-party libraries (lib/ subdirectory)
#   - Typically includes: -w (suppress warnings), -fvisibility=hidden
#
# APP Compile Options:
#   - Shows flags applied to main application (app/ subdirectory)
#   - Includes: optimization flags, warnings, sanitizers, hardening, etc.
#
# This output helps verify that:
#   - Optimization flags are correctly applied
#   - Security hardening is enabled (if requested)
#   - Platform-specific flags are set appropriately
#   - LTO and PGO are active (if enabled)
#
# Check CMake output during configuration to see these flags
#
#-------------------------------------------------------------------------------

get_directory_property(SUBDIRECTORY_COMPILE_OPTIONS DIRECTORY lib COMPILE_OPTIONS)
message(STATUS "LIB Compile Options: ${SUBDIRECTORY_COMPILE_OPTIONS}")

get_directory_property(SUBDIRECTORY_COMPILE_OPTIONS DIRECTORY app COMPILE_OPTIONS)
message(STATUS "APP Compile Options: ${SUBDIRECTORY_COMPILE_OPTIONS}")

