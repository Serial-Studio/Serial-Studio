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

cmake_minimum_required(VERSION 3.20)

#---------------------------------------------------------------------------------------------------
# Project definition and C++ standard
#---------------------------------------------------------------------------------------------------
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
#---------------------------------------------------------------------------------------------------

project(Serial-Studio LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(
   Qt6 REQUIRED
   COMPONENTS
   Core
   Qml
)

#---------------------------------------------------------------------------------------------------
# Build configuration options
#---------------------------------------------------------------------------------------------------
#
# Available build options and their default values:
#
# Licensing:
#   BUILD_GPL3=ON (default)        - Build GPLv3-only version
#   BUILD_COMMERCIAL=OFF (default) - Enable Pro features (requires license)
#
# Performance:
#   PRODUCTION_OPTIMIZATION=OFF (default) - Set to ON to enable O3, LTO, vectorization
#   ENABLE_PGO=OFF (default)              - Profile-Guided Optimization
#
# Security:
#   ENABLE_HARDENING=OFF (default) - Security hardening flags (FORTIFY, RELRO)
#
# Debugging:
#   DEBUG_SANITIZER=OFF (default) - AddressSanitizer + UBSan for debug builds
#
# Networking:
#   ENABLE_GRPC=OFF (default) - Enable gRPC server on port 8888
#
# Dependencies (zlib, expat, KissFFT, libusb, hidapi) are vendored in lib/ or fetched via
# FetchContent. OpenSSL is fetched as a prebuilt at configure time, and skipped entirely when
# ENABLE_GRPC=ON (gRPC's bundled BoringSSL supplies the crypto). gRPC/protobuf are found via
# find_package when ENABLE_GRPC=ON (install via system package manager).
#
#---------------------------------------------------------------------------------------------------

option(BUILD_GPL3              "Force GPLv3-only build"                 ON)
option(DEBUG_SANITIZER         "Enable sanitizers for debug builds"    OFF)
option(PRODUCTION_OPTIMIZATION "Enable production optimization flags"  OFF)
option(ENABLE_HARDENING        "Enable security hardening flags"       OFF)
option(ENABLE_PGO              "Enable Profile-Guided Optimization"    OFF)
option(BUILD_COMMERCIAL        "Enable commercial features"            OFF)
option(ENABLE_GRPC             "Enable gRPC server support"            OFF)
option(SS_USE_MIMALLOC         "Use the mimalloc allocator override (FetchContent from GitHub)" ON)

option(WITH_WEBENGINE          "Build against Qt WebEngine (markdown viewers, PDF reports)" ON)

if(WITH_WEBENGINE)
   add_compile_definitions(SERIAL_STUDIO_WITH_WEBENGINE)
   message(STATUS "WITH_WEBENGINE=ON, building with Qt WebEngine support")
else()
   message(STATUS "WITH_WEBENGINE=OFF, markdown viewers will use Qt rich-text and reports will be HTML-only")
endif()

#---------------------------------------------------------------------------------------------------
# Commercial build credentials
#---------------------------------------------------------------------------------------------------
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
#---------------------------------------------------------------------------------------------------

set(ARCGIS_API_KEY
  $ENV{ARCGIS_API_KEY}
  CACHE STRING "API Key for ArcGIS map")
set(SERIAL_STUDIO_LICENSE_KEY
  $ENV{SERIAL_STUDIO_LICENSE_KEY}
  CACHE STRING "License key for commercial build")
set(SERIAL_STUDIO_INSTANCE_ID
  $ENV{SERIAL_STUDIO_INSTANCE_ID}
  CACHE STRING "Instance ID for license validation")

#---------------------------------------------------------------------------------------------------
# Project metadata and branding
#---------------------------------------------------------------------------------------------------
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
#---------------------------------------------------------------------------------------------------

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
set(PROJECT_VERSION_MAJOR       "4")
set(PROJECT_VERSION_MINOR       "0")
set(PROJECT_VERSION_PATCH       "1")
set(PROJECT_VERSION             "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(PROJECT_APPCAST             "https://raw.githubusercontent.com/Serial-Studio/Serial-Studio/master/updates.json")
set(PROJECT_DESCRIPTION_FILE    "${PROJECT_ROOT_DIR}/README.md")
set(PROJECT_FILE_NAME           "${PROJECT_EXECUTABLE}-v${PROJECT_VERSION}")

#---------------------------------------------------------------------------------------------------
# Expose project metadata to source code
#---------------------------------------------------------------------------------------------------
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
#---------------------------------------------------------------------------------------------------

add_definitions(-DPROJECT_VENDOR="${PROJECT_VENDOR}")
add_definitions(-DPROJECT_CONTACT="${PROJECT_CONTACT}")
add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
add_definitions(-DPROJECT_APPCAST="${PROJECT_APPCAST}")
add_definitions(-DPROJECT_DISPNAME="${PROJECT_DISPNAME}")
add_definitions(-DPROJECT_DESCRIPTION_SUMMARY="${PROJECT_DESCRIPTION_SUMMARY}")

#---------------------------------------------------------------------------------------------------
# License enforcement and validation
#---------------------------------------------------------------------------------------------------
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
#---------------------------------------------------------------------------------------------------

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

    if(NOT DEFINED COMMERCIAL_BUILD_SALT)
        string(RANDOM LENGTH 18 ALPHABET "123456789" _SALT_FIRST)
        string(SUBSTRING "${_SALT_FIRST}" 0 1 _SALT_FIRST)
        string(RANDOM LENGTH 17 ALPHABET "0123456789" _SALT_REST)
        set(COMMERCIAL_BUILD_SALT "${_SALT_FIRST}${_SALT_REST}" CACHE INTERNAL "")
    endif()
    add_compile_definitions(COMMERCIAL_BUILD_SALT=${COMMERCIAL_BUILD_SALT}ULL)

    message(STATUS "License validation succeeded, building with commercial features")
    include(${CMAKE_SOURCE_DIR}/cmake/GenLicenseGuards.cmake)
endif()

#---------------------------------------------------------------------------------------------------
# System architecture detection
#---------------------------------------------------------------------------------------------------
#
# Logs the target CPU architecture for debugging and optimization reference:
#   - x86_64 / amd64: 64-bit Intel/AMD processors
#   - aarch64: 64-bit ARM processors (Apple Silicon, Raspberry Pi 4+)
#   - armv7l: 32-bit ARM processors (older Raspberry Pi models)
#
# This information is used later to select appropriate optimization flags
# (e.g., -march=x86-64-v2, -march=armv8-a, SSE4.1, NEON)
#
#---------------------------------------------------------------------------------------------------

message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")

#---------------------------------------------------------------------------------------------------
# Linux toolchain configuration
#---------------------------------------------------------------------------------------------------
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
#---------------------------------------------------------------------------------------------------

if (UNIX AND NOT APPLE)
   set(CMAKE_C_COMPILER_AR "/usr/bin/ar")
   set(CMAKE_CXX_COMPILER_AR "/usr/bin/ar")
   set(CMAKE_C_COMPILER_RANLIB "/usr/bin/ranlib")
   set(CMAKE_CXX_COMPILER_RANLIB "/usr/bin/ranlib")
endif()

#---------------------------------------------------------------------------------------------------
# Compiler warnings configuration
#---------------------------------------------------------------------------------------------------
#
# Per-toolchain warning/conformance switches (one branch runs):
#
#   clang-cl (Windows, MSVC ABI)  /wd4324 /wd4267 + /Zc:inline. The cl.exe-only conformance switches
#                                 /Zc:lambda and /Zc:throwingNew are dropped: clang-cl ignores them
#                                 and warns "argument unused during compilation".
#   MSVC cl.exe (Windows)         /wd4324 (C4324 alignment padding) /wd4267 (C4267 size_t->int) plus
#                                 /Zc:inline /Zc:lambda /Zc:throwingNew conformance switches.
#   GCC/Clang/MinGW               -Wall -Wextra -pedantic.
#
# Third-party libraries in lib/ build with -w (or /w), so these are effectively app-only.
#
#---------------------------------------------------------------------------------------------------

if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(
    /wd4324
    /wd4267
    /Zc:inline
  )
elseif(MSVC)
  add_compile_options(
    /wd4324
    /wd4267
    /Zc:inline
    /Zc:lambda
    /Zc:throwingNew
  )
else()
  add_compile_options(
    -Wall
    -Wextra
    -pedantic
  )
endif()

#---------------------------------------------------------------------------------------------------
# Build flags: optimization, SIMD, PGO, MiMalloc, sanitizers
#---------------------------------------------------------------------------------------------------
#
# The optimization/SIMD/PGO flags and the ASan/UBSan/TSan sanitizer flags now live in dedicated
# modules under cmake/. include() preserves directory scope, so the directory-level
# add_compile_options/add_link_options apply to every target exactly as the old inline blocks did.
#
#---------------------------------------------------------------------------------------------------

include(${CMAKE_SOURCE_DIR}/cmake/MiMalloc.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Optimization.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Sanitizers.cmake)

#---------------------------------------------------------------------------------------------------
# Debug build usability -- disable MSVC checked iterators
#---------------------------------------------------------------------------------------------------
#
# MSVC's debug STL ships with _ITERATOR_DEBUG_LEVEL=2 by default, which wraps every iterator
# operation in a checked iterator. On the FrameReader / FrameBuilder / Dashboard hotpath this
# turns each QMap / std::map / std::vector access into a bounds-checked tree walk and drops
# throughput by an order of magnitude under Debug. Disabling the checks restores usable Debug
# performance while leaving Q_ASSERT, debug symbols, and stepping intact.
#
# ABI consequence: every translation unit (including lib/) must agree on the iterator macros,
# so these are project-wide compile definitions rather than per-target.
#---------------------------------------------------------------------------------------------------

if(MSVC)
   add_compile_definitions(
      $<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>
      $<$<CONFIG:Debug>:_HAS_ITERATOR_DEBUGGING=0>
   )
endif()

#---------------------------------------------------------------------------------------------------
# Build flags: security hardening and code signing
#---------------------------------------------------------------------------------------------------
#
# Hardening.cmake carries the directory-level ENABLE_HARDENING block (unchanged) plus the
# per-target serial_studio_harden() deltas. Signing.cmake defines serial_studio_sign(). Both are
# called on the executable in app/CMakeLists.txt. Must be included before add_subdirectory().
#
#---------------------------------------------------------------------------------------------------

include(${CMAKE_SOURCE_DIR}/cmake/Hardening.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Signing.cmake)

#---------------------------------------------------------------------------------------------------
# Subdirectory build order
#---------------------------------------------------------------------------------------------------
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
#---------------------------------------------------------------------------------------------------

add_subdirectory(lib)
add_subdirectory(app)

#---------------------------------------------------------------------------------------------------
# Build configuration summary
#---------------------------------------------------------------------------------------------------
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
#---------------------------------------------------------------------------------------------------

get_directory_property(SUBDIRECTORY_COMPILE_OPTIONS DIRECTORY lib COMPILE_OPTIONS)
message(STATUS "LIB Compile Options: ${SUBDIRECTORY_COMPILE_OPTIONS}")

get_directory_property(SUBDIRECTORY_COMPILE_OPTIONS DIRECTORY app COMPILE_OPTIONS)
message(STATUS "APP Compile Options: ${SUBDIRECTORY_COMPILE_OPTIONS}")
