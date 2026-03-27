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
#   /Zc:inline      - Remove unreferenced inline functions
#   /Gy             - Enable function-level linking (COMDAT folding)
#
# GCC/Clang:
#   -w                          - Disable all warnings
#   -fvisibility=hidden         - Hide all symbols by default
#   -fvisibility-inlines-hidden - Hide C++ inline functions
#   -fstack-protector-strong    - Stack canary protection (when ENABLE_HARDENING=ON)
#
#---------------------------------------------------------------------------------------------------

if(MSVC)
  set(LIB_FLAGS
    /w                          # Suppress all warnings
    /Zc:inline                  # Remove unreferenced inline functions
    /Gy                         # Enable function-level linking
  )
  if(ENABLE_HARDENING)
    list(APPEND LIB_FLAGS /GS)  # Buffer security check
  endif()
else()
  set(LIB_FLAGS
    -w                          # Suppress all warnings
    -fvisibility=hidden         # Hide all symbols
    -fvisibility-inlines-hidden # Hide C++ inline function symbols
  )

  if(ENABLE_HARDENING)
    list(APPEND LIB_FLAGS -fstack-protector-strong)
  endif()
  set(LIB_LINK_FLAGS
    -fvisibility=hidden         # Hide all symbols at link time
  )
endif()

#---------------------------------------------------------------------------------------------------
# Internal third-party libraries (bundled, not available via vcpkg)
#---------------------------------------------------------------------------------------------------
#
# These libraries are bundled locally because they have no vcpkg port or
# depend on the project's specific Qt installation:
#
# QCodeEditor (lib/QCodeEditor/):
#   - Syntax-highlighted code editor widget
#   - Requires: Qt6::Core, Qt6::Widgets, Qt6::Gui
#
# QSimpleUpdater (lib/QSimpleUpdater/):
#   - Auto-update framework for Qt applications
#   - Requires: Qt6::Core, Qt6::Gui, Qt6::Network, Qt6::Widgets
#
# mdflib (lib/mdflib/):
#   - MDF4/MF4 file format parser (ASAM Measurement Data Format)
#   - Requires: ZLIB::ZLIB, expat::expat (provided by vcpkg)
#   - No vcpkg port available
#
#---------------------------------------------------------------------------------------------------

add_subdirectory(QCodeEditor)
add_subdirectory(QSimpleUpdater)
add_subdirectory(mdflib)

#---------------------------------------------------------------------------------------------------
# libusb (Pro edition only)
#---------------------------------------------------------------------------------------------------
#
# libusb provides cross-platform access to USB devices via bulk/control
# transfers. Fetched via FetchContent because the vcpkg port requires
# autotools which adds unnecessary build complexity.
#
# CMake target: usb-1.0
#---------------------------------------------------------------------------------------------------

if(BUILD_COMMERCIAL)
  include(FetchContent)
  set(FETCHCONTENT_QUIET ON)

  message(STATUS "Fetching libusb-cmake (libusb 1.0.29) from GitHub...")

  set(LIBUSB_BUILD_SHARED_LIBS        OFF CACHE BOOL "" FORCE)
  set(LIBUSB_BUILD_TESTING            OFF CACHE BOOL "" FORCE)
  set(LIBUSB_BUILD_EXAMPLES           OFF CACHE BOOL "" FORCE)
  set(LIBUSB_INSTALL_TARGETS          OFF CACHE BOOL "" FORCE)
  set(LIBUSB_EXPORT_INSTALL_TARGETS   OFF CACHE BOOL "" FORCE)

  FetchContent_Declare(
    libusb_cmake
    GIT_REPOSITORY https://github.com/libusb/libusb-cmake.git
    GIT_TAG        v1.0.29-0
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   FALSE
  )

  FetchContent_MakeAvailable(libusb_cmake)

  message(STATUS "libusb-cmake configured")
endif()

#---------------------------------------------------------------------------------------------------
# QuaZip (Pro edition only)
#---------------------------------------------------------------------------------------------------
#
# QuaZip provides ZIP archive creation/reading built on top of minizip.
# Used in commercial builds to zip exported image sessions on disconnect.
#
# Kept bundled because the vcpkg port depends on qtbase, which would
# conflict with the project's commercial Qt installation.
#
# License: LGPL-2.1
# CMake target: QuaZip::QuaZip
#---------------------------------------------------------------------------------------------------

if(BUILD_COMMERCIAL)
  message(STATUS "Configuring QuaZip 1.5 (bundled)...")

  set(QUAZIP_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
  set(QUAZIP_INSTALL      OFF CACHE BOOL "" FORCE)
  set(QUAZIP_USE_QT_ZLIB  OFF CACHE BOOL "" FORCE)
  set(QUAZIP_BZIP2        OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS   OFF CACHE BOOL "" FORCE)

  add_subdirectory(QuaZip)

  message(STATUS "QuaZip 1.5 configured")
endif()

#---------------------------------------------------------------------------------------------------
# Apply third-party library flags
#---------------------------------------------------------------------------------------------------
#
# Applies warning suppression and visibility flags to internal libraries.
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

configure_third_party_lib(QCodeEditor)
configure_third_party_lib(QSimpleUpdater)
configure_third_party_lib(mdf)

if(BUILD_COMMERCIAL)
  configure_third_party_lib(usb-1.0)
  configure_third_party_lib(QuaZip)
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
