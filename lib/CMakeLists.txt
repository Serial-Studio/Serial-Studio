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
# For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial

cmake_minimum_required(VERSION 3.20)

#---------------------------------------------------------------------------------------------------
# Save and clear global link flags
#---------------------------------------------------------------------------------------------------
#
# PIE (-pie) and other hardening link flags are meant for executables, not
# static libraries. Save them here and restore at the end so they only apply
# to the main application executable in app/.
#
#---------------------------------------------------------------------------------------------------

set(_SAVED_CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
set(_SAVED_CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
set(_SAVED_CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")
set(_SAVED_CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "")
set(CMAKE_SHARED_LINKER_FLAGS "")
set(CMAKE_MODULE_LINKER_FLAGS "")
set(CMAKE_STATIC_LINKER_FLAGS "")

#---------------------------------------------------------------------------------------------------
# FetchContent module setup
#---------------------------------------------------------------------------------------------------
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
#---------------------------------------------------------------------------------------------------

include(FetchContent)
set(FETCHCONTENT_QUIET ON)
if(UNIX)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif()

#---------------------------------------------------------------------------------------------------
# Third-party library compilation flags
#---------------------------------------------------------------------------------------------------
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
#---------------------------------------------------------------------------------------------------

if(MSVC)
  set(LIB_FLAGS
    /w
    /Zc:inline
    /Gy
  )
  if(ENABLE_HARDENING)
    list(APPEND LIB_FLAGS /GS)
  endif()
else()
  set(LIB_FLAGS
    -w
    -fvisibility=hidden
    -fvisibility-inlines-hidden
  )

  if(ENABLE_HARDENING)
    list(APPEND LIB_FLAGS -fstack-protector-strong)
  endif()
  set(LIB_LINK_FLAGS
    -fvisibility=hidden
  )
endif()

#---------------------------------------------------------------------------------------------------
# ZLIB compression library
#---------------------------------------------------------------------------------------------------
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
#    - Downloads ZLIB 1.3.2 from GitHub (madler/zlib)
#    - Builds as static library (zlibstatic)
#    - Embedded in final executable (no runtime dependency)
#    - Better version control and consistency
#
# Configuration:
#   BUILD_SHARED_LIBS=OFF   - Build static library only
#   ZLIB_BUILD_SHARED=OFF   - Don't build libz.so/.dylib (1.3.2 option name)
#   ZLIB_BUILD_STATIC=ON    - Build zlibstatic
#   ZLIB_BUILD_TESTING=OFF  - Don't build test/ example programs
#   ZLIB_INSTALL=OFF        - Don't install to system (local build only)
#   SKIP_INSTALL_ALL=ON     - Same, under the pre-1.3.2 option name
#
# Alias target:
#   - Creates ZLIB::ZLIB alias for both system and FetchContent builds
#   - Allows uniform linking: target_link_libraries(target ZLIB::ZLIB)
#
#---------------------------------------------------------------------------------------------------

if(USE_SYSTEM_ZLIB)
  message(STATUS "Using system-provided ZLIB")
  find_package(ZLIB REQUIRED)

  if(NOT TARGET ZLIB::ZLIB)
    message(FATAL_ERROR "System ZLIB not found. Install zlib development package or set USE_SYSTEM_ZLIB=OFF")
  endif()

  message(STATUS "System ZLIB found: ${ZLIB_VERSION_STRING}")
else()
  message(STATUS "Fetching ZLIB 1.3.2 from GitHub...")

  FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    # A tag is mutable; the commit it named when this pin was written is not.
    GIT_TAG        da607da739fa6047df13e66a2af6b8bec7c2a498 # v1.3.2
    GIT_PROGRESS   FALSE
  )

  set(BUILD_SHARED_LIBS   OFF CACHE INTERNAL "Build static libraries"  FORCE)
  set(ZLIB_BUILD_SHARED   OFF CACHE BOOL     "Build zlib shared lib"   FORCE)
  set(ZLIB_BUILD_STATIC   ON  CACHE BOOL     "Build zlib static lib"   FORCE)
  set(ZLIB_BUILD_TESTING  OFF CACHE BOOL     "Build zlib examples"     FORCE)
  set(ZLIB_INSTALL        OFF CACHE BOOL     "Skip zlib installation"  FORCE)
  set(SKIP_INSTALL_ALL    ON  CACHE BOOL     "Skip zlib installation"  FORCE)

  FetchContent_MakeAvailable(zlib)

  if(TARGET zlibstatic)
    target_compile_options(zlibstatic PRIVATE ${LIB_FLAGS})
    if(NOT MSVC AND LIB_LINK_FLAGS)
      target_link_options(zlibstatic PRIVATE ${LIB_LINK_FLAGS})
    endif()
  endif()

  if(NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
  endif()

  set(ZLIB_FOUND          TRUE                                    CACHE INTERNAL "ZLIB found via FetchContent")
  set(ZLIB_INCLUDE_DIR    "${zlib_SOURCE_DIR}"                    CACHE INTERNAL "ZLIB include directory")
  set(ZLIB_INCLUDE_DIRS   "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE INTERNAL "ZLIB include directories")
  set(ZLIB_LIBRARY        "zlibstatic"                            CACHE INTERNAL "ZLIB library target")
  set(ZLIB_LIBRARIES      "zlibstatic"                            CACHE INTERNAL "ZLIB libraries")
  set(ZLIB_VERSION_STRING "1.3.2"                                 CACHE INTERNAL "ZLIB version")

  message(STATUS "ZLIB 1.3.2 fetched and configured")
endif()

#---------------------------------------------------------------------------------------------------
# EXPAT XML parser library
#---------------------------------------------------------------------------------------------------
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
#    - Downloads EXPAT 2.8.3 from GitHub (libexpat/libexpat)
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
#---------------------------------------------------------------------------------------------------

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
  message(STATUS "Fetching EXPAT 2.8.3 from GitHub...")

  FetchContent_Declare(
    expat
    GIT_REPOSITORY https://github.com/libexpat/libexpat.git
    # A tag is mutable; the commit it named when this pin was written is not.
    GIT_TAG        92810461043fce37e70079b37ab1f04490a8f039 # R_2_8_3
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

  message(STATUS "EXPAT 2.8.3 fetched and configured")
endif()

#---------------------------------------------------------------------------------------------------
# Internal third-party libraries
#---------------------------------------------------------------------------------------------------
#
# Builds local third-party libraries from lib/ subdirectories:
#
# OpenSSL (lib/OpenSSL/):
#   - target_link_openssl() is a no-op when ENABLE_GRPC=ON: gRPC bundles BoringSSL, which already
#     exports the OpenSSL-compatible crypto/SSL symbols (linking both duplicates them, a hard error
#     under clang-cl's lld-link).
#   - Otherwise Windows MSVC/MinGW and macOS download prebuilt static OpenSSL 4.0.0 from the
#     alex-spataru/OpenSSL-Builder release; Linux links the distro OpenSSL via pkg-config.
#   - Used for: HTTPS, SSL/TLS, license validation, MQTT encryption
#
# KissFFT (lib/KissFFT/):
#   - Fast Fourier Transform library for FFT plot widget
#   - C implementation, header-only with minimal compile-time config
#   - Used for: Real-time spectrum analysis, audio visualization
#   - Optimized with SSE/NEON SIMD instructions when available
#   - Bundled copy; cmake_minimum_required bumped to 3.10 to satisfy modern CMake
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
#---------------------------------------------------------------------------------------------------

set(QSU_QT_VERSION_MAJOR 6)

add_subdirectory(luajit)
add_subdirectory(OpenSSL)
add_subdirectory(KissFFT)
add_subdirectory(QCodeEditor)
add_subdirectory(QSimpleUpdater)
add_subdirectory(mdflib)

#---------------------------------------------------------------------------------------------------
# OPC UA stack: mbedTLS + open62541 (Pro edition only, spec 0067)
#---------------------------------------------------------------------------------------------------
#
# mbedTLS is pinned to the 3.6 LTS line on purpose: open62541's crypto plugin guards its API use
# with MBEDTLS_VERSION_NUMBER checks that stop at 3.x, and mbedTLS 4.x is a breaking PSA-only
# release it does not handle. mbedTLS must be added BEFORE open62541, which links its targets.
#
# ENABLE_PROGRAMS/ENABLE_TESTING are off because neither is vendored (the tree carries only what
# the library itself compiles), and mbedTLS disables its own install/export rules automatically
# when it detects it is a subproject.
#
# SS_BUILD_TESTS builds the stack too, on a GPL configure as well: tst_opcua_marshal links the
# real open62541 to pin the one seam between its C vocabulary and Qt, and CI's unit tier is a GPL
# configure. Building the targets does NOT put them in the application -- target_link_open62541()
# stays a no-op outside BUILD_COMMERCIAL, so the GPL binary is unchanged.
#
if(SS_ENABLE_OPCUA AND (BUILD_COMMERCIAL OR SS_BUILD_TESTS))
  set(ENABLE_TESTING             OFF CACHE BOOL "" FORCE)
  set(ENABLE_PROGRAMS            OFF CACHE BOOL "" FORCE)
  set(MBEDTLS_FATAL_WARNINGS     OFF CACHE BOOL "" FORCE)
  set(GEN_FILES                  OFF CACHE BOOL "" FORCE)
  set(USE_STATIC_MBEDTLS_LIBRARY ON  CACHE BOOL "" FORCE)
  set(USE_SHARED_MBEDTLS_LIBRARY OFF CACHE BOOL "" FORCE)

  add_subdirectory(mbedtls)
  add_subdirectory(open62541)

  #
  # everest and p256m are separate targets under mbedtls/3rdparty, so the three library targets
  # alone left them on the app's warning set and their -Wundef reports (MBEDTLS_GCC_VERSION unset
  # outside GCC) were the only third-party noise left in a clean build.
  #
  foreach(_ss_mbed_target
          mbedtls mbedx509 mbedcrypto
          ${MBEDTLS_TARGET_PREFIX}everest ${MBEDTLS_TARGET_PREFIX}p256m)
    if(TARGET ${_ss_mbed_target})
      if(MSVC)
        target_compile_options(${_ss_mbed_target} PRIVATE /w)
      else()
        target_compile_options(${_ss_mbed_target} PRIVATE -w)
      endif()
    endif()
  endforeach()
endif()

#
# Defined unconditionally: app/CMakeLists.txt calls it in every configuration, and an undefined
# command is a hard configure error in the builds that have no OPC UA stack.
#
if(NOT COMMAND target_link_open62541)
  function(target_link_open62541 TARGET)
  endfunction()
endif()

#---------------------------------------------------------------------------------------------------
# Industrial PLC stacks: libplctag (EtherNet/IP), Pro edition only (spec 0073)
#---------------------------------------------------------------------------------------------------
#
# It is fetched at configure time by its own module rather than vendored, so the repository carries
# only the Serial-Studio-authored CMakeLists that assembles it. It is not built on a GPL configure:
# unlike open62541 it has no unit-tier consumer, so there is nothing to build for a suite that would
# never link it. The S7comm client needs no entry here at all: it is first-party Qt-Core code under
# app/src/IO/Drivers/S7, like the IEC 60870-5-104 stack beside it.
#
if(SS_ENABLE_EIP AND BUILD_COMMERCIAL)
  add_subdirectory(libplctag)
endif()

#
# Defined unconditionally, exactly like target_link_open62541(): app/CMakeLists.txt calls it in
# every configuration, and an undefined command is a hard configure error in the builds that carry
# no PLC stack.
#
if(NOT COMMAND target_link_libplctag)
  function(target_link_libplctag TARGET)
  endfunction()
endif()

#---------------------------------------------------------------------------------------------------
# libusb (Pro edition only)
#---------------------------------------------------------------------------------------------------
#
# libusb provides cross-platform access to USB devices via bulk/control transfers.
# It is only included in BUILD_COMMERCIAL builds — the GPL binary contains no USB code.
#
# Acquisition: FetchContent downloads libusb 1.0.30 from GitHub at configure time.
# libusb ships no CMakeLists.txt, so the libusb-cmake wrapper assembles the target.
#
# Platform source sets:
#   Windows : windows_winusb.c + WinUSB / SetupAPI system libraries
#   macOS   : darwin_usb.c + IOKit / CoreFoundation frameworks
#   Linux   : linux_usbfs.c + linux_netlink.c (udev-less netlink hotplug)
#
#---------------------------------------------------------------------------------------------------

if(BUILD_COMMERCIAL)
  message(STATUS "Fetching libusb-cmake (libusb 1.0.30) from GitHub...")

  set(LIBUSB_BUILD_SHARED_LIBS        OFF CACHE BOOL "" FORCE)
  set(LIBUSB_BUILD_TESTING            OFF CACHE BOOL "" FORCE)
  set(LIBUSB_BUILD_EXAMPLES           OFF CACHE BOOL "" FORCE)
  set(LIBUSB_INSTALL_TARGETS          OFF CACHE BOOL "" FORCE)
  set(LIBUSB_EXPORT_INSTALL_TARGETS   OFF CACHE BOOL "" FORCE)

  FetchContent_Declare(
    libusb_cmake
    GIT_REPOSITORY https://github.com/libusb/libusb-cmake.git
    # A tag is mutable; the commit it named when this pin was written is not.
    GIT_TAG        c8477c10ac2ac6b1718d4d498e102b9f18b776f5 # v1.0.30-0
    GIT_PROGRESS   FALSE
  )

  FetchContent_MakeAvailable(libusb_cmake)

  message(STATUS "libusb-cmake configured")
endif()

#---------------------------------------------------------------------------------------------------
# hidapi (Pro edition only)
#---------------------------------------------------------------------------------------------------
#
# hidapi provides cross-platform HID device access using native OS APIs:
#   Windows : WinAPI (hid.dll)
#   macOS   : IOHIDManager (IOKit)
#   Linux   : hidraw kernel interface (no libusb; avoids symbol conflicts
#             with the usb-1.0 Raw USB driver already in the build)
#
# Acquisition: bundled local copy in lib/hidapi (hidapi 0.14.0), static build,
#              no tests/examples. cmake_minimum_required bumped to 3.10 to satisfy
#              modern CMake's compatibility requirement.
# CMake target exposed: hidapi::hidapi
#---------------------------------------------------------------------------------------------------

if(BUILD_COMMERCIAL)
  message(STATUS "Configuring hidapi 0.14.0 (bundled)...")

  set(HIDAPI_BUILD_HIDTEST   OFF CACHE BOOL "" FORCE)
  set(HIDAPI_WITH_TESTS      OFF CACHE BOOL "" FORCE)
  set(HIDAPI_INSTALL_TARGETS OFF CACHE BOOL "" FORCE)
  set(HIDAPI_PRINT_VERSION   OFF CACHE BOOL "" FORCE)
  set(HIDAPI_WITH_LIBUSB     OFF CACHE BOOL "" FORCE)
  set(HIDAPI_WITH_HIDRAW     ON  CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS      OFF CACHE BOOL "" FORCE)

  add_subdirectory(hidapi)
  if(APPLE)
    set(HIDAPI_CONCRETE_TARGET hidapi_darwin)
  elseif(WIN32)
    set(HIDAPI_CONCRETE_TARGET hidapi_winapi)
  else()
    set(HIDAPI_CONCRETE_TARGET hidapi_hidraw)
  endif()

  message(STATUS "hidapi 0.14.0 configured")
endif()

#---------------------------------------------------------------------------------------------------
# Apply third-party library flags
#---------------------------------------------------------------------------------------------------
#
# Applies warning suppression and visibility flags to internal libraries:
#
# Why apply flags after add_subdirectory?
#   - Libraries define their own targets in their CMakeLists.txt
#   - We need to modify target properties after they exist
#   - This ensures our flags override any library defaults
#
# Applied flags (from LIB_FLAGS and LIB_LINK_FLAGS):
#   Static targets  — Compile: -w, -fvisibility=hidden / Link: -fvisibility=hidden
#   Shared targets  — Compile: -w only (-fvisibility=hidden is deliberately omitted;
#                     applying it to a shared library hides all exported symbols and
#                     causes undefined-symbol errors in the final executable)
#
# Target checks (if(TARGET ...)):
#   - Verifies target exists before modifying it
#   - Prevents errors if library subdirectory is missing
#   - Allows conditional library builds
#
# Libraries configured:
#   kissfft        - FFT library (static)
#   QCodeEditor    - Syntax highlighting editor (static, Qt-based)
#   QSimpleUpdater - Auto-update framework (static, Qt-based)
#   mdf            - MDF4 file parser (static, requires ZLIB, EXPAT)
#   usb-1.0        - Raw USB driver (static, commercial builds only)
#   hidapi         - HID device driver (static, commercial builds only)
#
# Not applied to:
#   OpenSSL - Uses prebuilt binaries fetched at configure time (no compilation flags needed)
#   ZLIB    - Flags applied during FetchContent configuration
#   EXPAT   - Flags applied during FetchContent configuration
#
# Effect:
#   - Cleaner build output (no third-party warnings)
#   - Smaller binaries (hidden symbols on static targets)
#   - Faster linking (fewer exported symbols)
#
#---------------------------------------------------------------------------------------------------

function(configure_third_party_lib TARGET_NAME)
  if(NOT TARGET ${TARGET_NAME})
    return()
  endif()
  get_target_property(_lib_type ${TARGET_NAME} TYPE)
  if(NOT _lib_type STREQUAL "SHARED_LIBRARY")
    target_compile_options(${TARGET_NAME} PRIVATE ${LIB_FLAGS})
    if(NOT MSVC AND LIB_LINK_FLAGS)
      target_link_options(${TARGET_NAME} PRIVATE ${LIB_LINK_FLAGS})
    endif()
  else()
    set(_warn_flags ${LIB_FLAGS})
    list(FILTER _warn_flags EXCLUDE REGEX "fvisibility")
    target_compile_options(${TARGET_NAME} PRIVATE ${_warn_flags})
  endif()

  get_target_property(INC_DIRS ${TARGET_NAME} INTERFACE_INCLUDE_DIRECTORIES)
  if(INC_DIRS)
    set_property(TARGET ${TARGET_NAME} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
    target_include_directories(${TARGET_NAME} SYSTEM PUBLIC ${INC_DIRS})
  endif()
endfunction()

configure_third_party_lib(luajit)
# LuaJIT's stage-1 host tools are separate executable targets, so they inherit the app's
# -Wundef / CRT-deprecation warnings unless suppressed like the library itself.
configure_third_party_lib(luajit-minilua)
configure_third_party_lib(luajit-buildvm)
configure_third_party_lib(kissfft)
configure_third_party_lib(QCodeEditor)
configure_third_party_lib(QSimpleUpdater)
configure_third_party_lib(mdf)

if(BUILD_COMMERCIAL)
  configure_third_party_lib(usb-1.0)
  configure_third_party_lib(${HIDAPI_CONCRETE_TARGET})
endif()

#---------------------------------------------------------------------------------------------------
# Restore global link flags
#---------------------------------------------------------------------------------------------------
#
# Restore the hardening link flags saved at the top of this file so they
# apply to the main application executable in app/.
#
#---------------------------------------------------------------------------------------------------

set(CMAKE_EXE_LINKER_FLAGS "${_SAVED_CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${_SAVED_CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${_SAVED_CMAKE_MODULE_LINKER_FLAGS}")
set(CMAKE_STATIC_LINKER_FLAGS "${_SAVED_CMAKE_STATIC_LINKER_FLAGS}")
