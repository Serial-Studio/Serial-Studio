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
# Save and clear global link flags
#-------------------------------------------------------------------------------
#
# PIE (-pie) and other hardening link flags are meant for executables, not
# static libraries. Save them here and restore at the end so they only apply
# to the main application executable in app/.
#

set(_SAVED_CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
set(_SAVED_CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
set(_SAVED_CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")
set(_SAVED_CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "")
set(CMAKE_SHARED_LINKER_FLAGS "")
set(CMAKE_MODULE_LINKER_FLAGS "")
set(CMAKE_STATIC_LINKER_FLAGS "")

#-------------------------------------------------------------------------------
# FetchContent module setup
#-------------------------------------------------------------------------------
#
# Configures CMake's FetchContent module for downloading dependencies:
#
# FetchContent:
#   - Modern CMake module for downloading external projects at configure time
#   - Replaces ExternalProject (which downloads at build time)
#   - Allows dependencies to be configured with parent project's settings
#
# FETCHCONTENT_QUIET=ON:
#   - Suppresses verbose output during downloads
#   - Cleaner CMake configuration output
#
# CMAKE_FIND_LIBRARY_SUFFIXES (UNIX only):
#   - Prioritizes static libraries (.a) over shared libraries (.so, .dylib)
#   - Ensures dependencies are statically linked (no runtime .so dependencies)
#   - Simplifies deployment (fewer files to distribute)
#
# Why FetchContent instead of system packages?
#   - Ensures consistent versions across all platforms
#   - Avoids version conflicts with system libraries
#   - Simplifies build process (no manual dependency installation)
#   - Better for sandboxed environments (Flathub, snap, AppImage)
#
# Note: USE_SYSTEM_ZLIB and USE_SYSTEM_EXPAT override FetchContent for
# those libraries when building in sandboxed environments
#
#-------------------------------------------------------------------------------

include(FetchContent)
set(FETCHCONTENT_QUIET ON)
if(UNIX)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif()

#-------------------------------------------------------------------------------
# Third-party library compilation flags
#-------------------------------------------------------------------------------
#
# Defines compiler flags specifically for third-party libraries:
#
# Why separate flags for libraries?
#   - Third-party code often has warnings we can't (or don't want to) fix
#   - We don't need debug symbols or warnings for stable dependencies
#   - Symbol visibility control reduces binary size and link time
#
# MSVC (Visual Studio):
#   /w              - Disable all compiler warnings
#                     (third-party code may not be warning-clean)
#   /Zc:inline      - Remove unreferenced inline functions
#                     (reduces object file bloat)
#   /Gy             - Enable function-level linking (COMDAT folding)
#                     (allows linker to remove unused functions)
#
# GCC/Clang:
#   -w                          - Disable all warnings
#   -fvisibility=hidden         - Hide all symbols by default
#                                 (only export explicitly marked symbols)
#   -fvisibility-inlines-hidden - Hide C++ inline functions
#                                 (prevents inline function bloat in symbol table)
#   -fstack-protector-strong    - Stack canary protection (when ENABLE_HARDENING=ON)
#                                 Applied to third-party libs for defense-in-depth
#
# Benefits of hidden visibility:
#   - Smaller binary size (fewer exported symbols)
#   - Faster linking (less symbol resolution)
#   - Prevents symbol conflicts between libraries
#   - Better optimization (compiler can inline/devirtualize)
#
# These flags are applied to: KissFFT, QCodeEditor, QSimpleUpdater, mdflib,
# ZLIB, and EXPAT (when not using system libraries)
#
#-------------------------------------------------------------------------------

if(MSVC)
  set(LIB_FLAGS
    /w                          # Suppress all warnings
    /Zc:inline                  # Remove unreferenced inline functions
    /Gy                         # Enable function-level linking
  )
  # Add buffer security check for libraries when hardening is enabled
  if(ENABLE_HARDENING)
    list(APPEND LIB_FLAGS /GS)  # Buffer security check
  endif()
else()
  set(LIB_FLAGS
    -w                          # Suppress all warnings
    -fvisibility=hidden         # Hide all symbols
    -fvisibility-inlines-hidden # Hide C++ inline function symbols
  )
  # Add stack protector for libraries when hardening is enabled
  # This ensures third-party code also benefits from stack canary protection
  if(ENABLE_HARDENING)
    list(APPEND LIB_FLAGS -fstack-protector-strong)
  endif()
  set(LIB_LINK_FLAGS
    -fvisibility=hidden         # Hide all symbols at link time
  )
endif()

#-------------------------------------------------------------------------------
# ZLIB compression library
#-------------------------------------------------------------------------------
#
# ZLIB is required for:
#   - MDF4 file decompression (compressed data blocks)
#   - PNG image handling in Qt
#   - HTTP compression (gzip) for network operations
#
# Two acquisition modes:
#
# 1. System library (USE_SYSTEM_ZLIB=ON):
#    - Uses zlib provided by the OS package manager
#    - Required for Flathub builds (sandboxed environment)
#    - Typically /usr/lib/x86_64-linux-gnu/libz.so on Linux
#    - Install: sudo apt install zlib1g-dev (Debian/Ubuntu)
#              brew install zlib (macOS)
#              vcpkg install zlib (Windows)
#
# 2. FetchContent download (USE_SYSTEM_ZLIB=OFF, default):
#    - Downloads ZLIB 1.3.1 from GitHub (madler/zlib)
#    - Builds as static library (zlibstatic)
#    - Embedded in final executable (no runtime dependency)
#    - Better version control and consistency
#
# Configuration:
#   BUILD_SHARED_LIBS=OFF   - Build static library only
#   ZLIB_BUILD_EXAMPLES=OFF - Don't build example programs
#   SKIP_INSTALL_ALL=ON     - Don't install to system (local build only)
#
# Alias target:
#   - Creates ZLIB::ZLIB alias for both system and FetchContent builds
#   - Allows uniform linking: target_link_libraries(target ZLIB::ZLIB)
#
#-------------------------------------------------------------------------------

if(USE_SYSTEM_ZLIB)
  message(STATUS "Using system-provided ZLIB")
  find_package(ZLIB REQUIRED)

  if(NOT TARGET ZLIB::ZLIB)
    message(FATAL_ERROR "System ZLIB not found. Install zlib development package or set USE_SYSTEM_ZLIB=OFF")
  endif()

  message(STATUS "System ZLIB found: ${ZLIB_VERSION_STRING}")
else()
  message(STATUS "Fetching ZLIB 1.3.1 from GitHub...")

  FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG        v1.3.1
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   FALSE
  )

  set(BUILD_SHARED_LIBS   OFF CACHE INTERNAL "Build static libraries" FORCE)
  set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL     "Build zlib examples"    FORCE)
  set(SKIP_INSTALL_ALL    ON  CACHE BOOL     "Skip zlib installation" FORCE)

  FetchContent_MakeAvailable(zlib)

  target_compile_options(zlibstatic PRIVATE ${LIB_FLAGS})
  if(NOT MSVC AND LIB_LINK_FLAGS)
    target_link_options(zlibstatic PRIVATE ${LIB_LINK_FLAGS})
  endif()

  if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
    set(ZLIB_FOUND TRUE CACHE INTERNAL "ZLIB found via FetchContent")
    set(ZLIB_INCLUDE_DIRS "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE INTERNAL "ZLIB include directories")
    set(ZLIB_LIBRARIES zlibstatic CACHE INTERNAL "ZLIB libraries")
    set(ZLIB_VERSION_STRING "1.3.1" CACHE INTERNAL "ZLIB version")
  endif()

  message(STATUS "ZLIB 1.3.1 fetched and configured")
endif()

#-------------------------------------------------------------------------------
# EXPAT XML parser library
#-------------------------------------------------------------------------------
#
# EXPAT is required for:
#   - MDF4 file parsing (XML metadata in MDF4 files)
#   - XML-based configuration files
#   - SVG rendering (Qt SVG uses EXPAT internally)
#
# Two acquisition modes:
#
# 1. System library (USE_SYSTEM_EXPAT=ON):
#    - Uses expat provided by the OS package manager
#    - Required for Flathub builds (sandboxed environment)
#    - Typically /usr/lib/x86_64-linux-gnu/libexpat.so on Linux
#    - Install: sudo apt install libexpat1-dev (Debian/Ubuntu)
#              brew install expat (macOS)
#              vcpkg install expat (Windows)
#
# 2. FetchContent download (USE_SYSTEM_EXPAT=OFF, default):
#    - Downloads EXPAT 2.6.4 from GitHub (libexpat/libexpat)
#    - Builds as static library
#    - Embedded in final executable (no runtime dependency)
#    - Better version control and consistency
#
# Configuration:
#   EXPAT_SHARED_LIBS=OFF     - Build static library only
#   EXPAT_BUILD_DOCS=OFF      - Don't build documentation
#   EXPAT_BUILD_EXAMPLES=OFF  - Don't build example programs
#   EXPAT_BUILD_TESTS=OFF     - Don't build unit tests
#   EXPAT_BUILD_TOOLS=OFF     - Don't build xmlwf tool
#   EXPAT_BUILD_FUZZERS=OFF   - Don't build fuzzing targets
#   EXPAT_BUILD_PKGCONFIG=OFF - Don't generate pkg-config file
#   EXPAT_ENABLE_INSTALL=OFF  - Don't install to system (local build only)
#
# Source subdirectory:
#   SOURCE_SUBDIR expat - EXPAT's CMakeLists.txt is in expat/ subdirectory
#
# Alias target:
#   - Creates expat::expat alias for both system and FetchContent builds
#   - Handles EXPAT::EXPAT → expat::expat alias for system packages
#   - Allows uniform linking: target_link_libraries(target expat::expat)
#
#-------------------------------------------------------------------------------

if(USE_SYSTEM_EXPAT)
  message(STATUS "Using system-provided EXPAT")
  find_package(EXPAT REQUIRED)

  if(NOT TARGET expat::expat)
    if(TARGET EXPAT::EXPAT)
      add_library(expat::expat ALIAS EXPAT::EXPAT)
    else()
      message(FATAL_ERROR "System EXPAT not found. Install expat development package or set USE_SYSTEM_EXPAT=OFF")
    endif()
  endif()

  message(STATUS "System EXPAT found: ${EXPAT_VERSION_STRING}")
else()
  message(STATUS "Fetching EXPAT 2.6.4 from GitHub...")

  FetchContent_Declare(
    expat
    GIT_REPOSITORY https://github.com/libexpat/libexpat.git
    GIT_TAG        R_2_6_4
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   FALSE
    SOURCE_SUBDIR  expat
  )

  set(EXPAT_SHARED_LIBS     OFF CACHE BOOL "Build shared libraries"  FORCE)
  set(EXPAT_BUILD_DOCS      OFF CACHE BOOL "Build documentation"     FORCE)
  set(EXPAT_BUILD_EXAMPLES  OFF CACHE BOOL "Build examples"          FORCE)
  set(EXPAT_BUILD_TESTS     OFF CACHE BOOL "Build tests"             FORCE)
  set(EXPAT_BUILD_TOOLS     OFF CACHE BOOL "Build tools"             FORCE)
  set(EXPAT_BUILD_FUZZERS   OFF CACHE BOOL "Build fuzzers"           FORCE)
  set(EXPAT_BUILD_PKGCONFIG OFF CACHE BOOL "Build pkg-config file"   FORCE)
  set(EXPAT_ENABLE_INSTALL  OFF CACHE BOOL "Skip expat installation" FORCE)

  FetchContent_MakeAvailable(expat)

  target_compile_options(expat PRIVATE ${LIB_FLAGS})
  if(NOT MSVC AND LIB_LINK_FLAGS)
    target_link_options(expat PRIVATE ${LIB_LINK_FLAGS})
  endif()

  if(TARGET expat AND NOT TARGET expat::expat)
    add_library(expat::expat ALIAS expat)
    set(EXPAT_FOUND TRUE CACHE INTERNAL "EXPAT found via FetchContent")
    set(EXPAT_INCLUDE_DIRS "${expat_SOURCE_DIR}/expat/lib" CACHE INTERNAL "EXPAT include directories")
    set(EXPAT_LIBRARIES expat CACHE INTERNAL "EXPAT libraries")
  endif()

  message(STATUS "EXPAT 2.6.4 fetched and configured")
endif()

#-------------------------------------------------------------------------------
# Internal third-party libraries
#-------------------------------------------------------------------------------
#
# Builds local third-party libraries from lib/ subdirectories:
#
# OpenSSL (lib/OpenSSL/):
#   - Precompiled static OpenSSL libraries for Windows, macOS, Linux
#   - Provides: libssl.a, libcrypto.a (or .lib on Windows)
#   - Used for: HTTPS, SSL/TLS, license validation, MQTT encryption
#   - Platform-specific builds for each architecture
#
# KissFFT (lib/KissFFT/):
#   - Fast Fourier Transform library for FFT plot widget
#   - C implementation, header-only with minimal compile-time config
#   - Used for: Real-time spectrum analysis, audio visualization
#   - Optimized with SSE/NEON SIMD instructions when available
#
# QCodeEditor (lib/QCodeEditor/):
#   - Syntax-highlighted code editor widget
#   - Supports: JSON, JavaScript, Python syntax highlighting
#   - Used for: JSON project editor, JavaScript frame parser editor
#   - Based on Qt's QPlainTextEdit with custom syntax highlighter
#
# QSimpleUpdater (lib/QSimpleUpdater/):
#   - Auto-update framework for Qt applications
#   - Checks for updates from JSON appcast file
#   - Used for: Automatic update notifications
#   - Downloads update info from PROJECT_APPCAST URL
#
# mdflib (lib/mdflib/):
#   - MDF4/MF4 file format parser (ASAM Measurement Data Format)
#   - Supports: CAN, LIN, FlexRay, analog channels
#   - Used for: Replay automotive data logs (Pro feature)
#   - Requires: ZLIB (decompression), EXPAT (XML metadata parsing)
#
# Build order:
#   OpenSSL → (no dependencies)
#   KissFFT → (no dependencies)
#   QCodeEditor → Qt6::Widgets, Qt6::Gui
#   QSimpleUpdater → Qt6::Network
#   mdflib → ZLIB::ZLIB, expat::expat
#
#-------------------------------------------------------------------------------

add_subdirectory(OpenSSL)
add_subdirectory(KissFFT)
add_subdirectory(QCodeEditor)
add_subdirectory(QSimpleUpdater)
add_subdirectory(mdflib)

#-------------------------------------------------------------------------------
# Apply third-party library flags
#-------------------------------------------------------------------------------
#
# Applies warning suppression and visibility flags to internal libraries:
#
# Why apply flags after add_subdirectory?
#   - Libraries define their own targets in their CMakeLists.txt
#   - We need to modify target properties after they exist
#   - This ensures our flags override any library defaults
#
# Applied flags (from LIB_FLAGS and LIB_LINK_FLAGS):
#   Compile: -w (suppress warnings), -fvisibility=hidden
#   Link:    -fvisibility=hidden (GCC/Clang only)
#
# Target checks (if(TARGET ...)):
#   - Verifies target exists before modifying it
#   - Prevents errors if library subdirectory is missing
#   - Allows conditional library builds
#
# Libraries configured:
#   kissfft       - FFT library (static, no dependencies)
#   QCodeEditor   - Syntax highlighting editor (Qt-based)
#   QSimpleUpdater - Auto-update framework (Qt-based)
#   mdf           - MDF4 file parser (requires ZLIB, EXPAT)
#
# Not applied to:
#   OpenSSL - Uses precompiled binaries (no compilation flags needed)
#   ZLIB    - Flags applied during FetchContent configuration
#   EXPAT   - Flags applied during FetchContent configuration
#
# Effect:
#   - Cleaner build output (no third-party warnings)
#   - Smaller binaries (hidden symbols)
#   - Faster linking (fewer exported symbols)
#
#-------------------------------------------------------------------------------

# Helper function to apply library flags and mark includes as SYSTEM
# This suppresses warnings when app code includes third-party headers
function(configure_third_party_lib TARGET_NAME)
  if(NOT TARGET ${TARGET_NAME})
    return()
  endif()

  target_compile_options(${TARGET_NAME} PRIVATE ${LIB_FLAGS})
  if(NOT MSVC AND LIB_LINK_FLAGS)
    target_link_options(${TARGET_NAME} PRIVATE ${LIB_LINK_FLAGS})
  endif()

  # Mark include directories as SYSTEM to suppress warnings in consuming targets
  get_target_property(INC_DIRS ${TARGET_NAME} INTERFACE_INCLUDE_DIRECTORIES)
  if(INC_DIRS)
    set_property(TARGET ${TARGET_NAME} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(${TARGET_NAME} SYSTEM PUBLIC ${INC_DIRS})
  endif()
endfunction()

configure_third_party_lib(kissfft)
configure_third_party_lib(QCodeEditor)
configure_third_party_lib(QSimpleUpdater)
configure_third_party_lib(mdf)

#-------------------------------------------------------------------------------
# Restore global link flags
#-------------------------------------------------------------------------------
#
# Restore the hardening link flags saved at the top of this file so they
# apply to the main application executable in app/.
#

set(CMAKE_EXE_LINKER_FLAGS "${_SAVED_CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${_SAVED_CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${_SAVED_CMAKE_MODULE_LINKER_FLAGS}")
set(CMAKE_STATIC_LINKER_FLAGS "${_SAVED_CMAKE_STATIC_LINKER_FLAGS}")
