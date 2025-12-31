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
# Include FetchContent module
#-------------------------------------------------------------------------------

include(FetchContent)
set(FETCHCONTENT_QUIET ON)
if(UNIX)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif()

#-------------------------------------------------------------------------------
# Compiler flags for third-party libraries
#-------------------------------------------------------------------------------

if(MSVC)
  set(LIB_FLAGS
    /w                          # Suppress all warnings
    /Zc:inline                  # Remove unreferenced inline functions
    /Gy                         # Enable function-level linking
  )
else()
  set(LIB_FLAGS
    -w                          # Suppress all warnings
    -fvisibility=hidden         # Hide all symbols
    -fvisibility-inlines-hidden # Hide C++ inline function symbols
  )
  set(LIB_LINK_FLAGS
    -fvisibility=hidden         # Hide all symbols at link time
  )
endif()

#-------------------------------------------------------------------------------
# ZLIB: Use system library or download from GitHub
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
# EXPAT: Use system library or download from GitHub
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
# Compile local libraries
#-------------------------------------------------------------------------------

add_subdirectory(OpenSSL)
add_subdirectory(KissFFT)
add_subdirectory(QCodeEditor)
add_subdirectory(QSimpleUpdater)
add_subdirectory(mdflib)

#-------------------------------------------------------------------------------
# Apply flags to internal libraries
#-------------------------------------------------------------------------------

if(TARGET kissfft)
  target_compile_options(kissfft PRIVATE ${LIB_FLAGS})
  if(NOT MSVC AND LIB_LINK_FLAGS)
    target_link_options(kissfft PRIVATE ${LIB_LINK_FLAGS})
  endif()
endif()

if(TARGET QCodeEditor)
  target_compile_options(QCodeEditor PRIVATE ${LIB_FLAGS})
  if(NOT MSVC AND LIB_LINK_FLAGS)
    target_link_options(QCodeEditor PRIVATE ${LIB_LINK_FLAGS})
  endif()
endif()

if(TARGET QSimpleUpdater)
  target_compile_options(QSimpleUpdater PRIVATE ${LIB_FLAGS})
  if(NOT MSVC AND LIB_LINK_FLAGS)
    target_link_options(QSimpleUpdater PRIVATE ${LIB_LINK_FLAGS})
  endif()
endif()

if(TARGET mdf)
  target_compile_options(mdf PRIVATE ${LIB_FLAGS})
  if(NOT MSVC AND LIB_LINK_FLAGS)
    target_link_options(mdf PRIVATE ${LIB_LINK_FLAGS})
  endif()
endif()

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
