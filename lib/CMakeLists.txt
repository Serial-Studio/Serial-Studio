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
# Include FetchContent module
#-------------------------------------------------------------------------------

include(FetchContent)
set(FETCHCONTENT_QUIET ON)
if(UNIX)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif()

#-------------------------------------------------------------------------------
# FetchContent: Download and build ZLIB 1.3.1 as static library
#-------------------------------------------------------------------------------

message(STATUS "Fetching ZLIB 1.3.1 from GitHub...")

FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG        v1.3.1
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   FALSE
)

set(BUILD_SHARED_LIBS   OFF CACHE INTERNAL "Build static libraries")
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL     "Build zlib examples" FORCE)

FetchContent_MakeAvailable(zlib)
if(NOT TARGET ZLIB::ZLIB)
  add_library(ZLIB::ZLIB ALIAS zlibstatic)
  set(ZLIB_FOUND TRUE CACHE INTERNAL "ZLIB found via FetchContent")
  set(ZLIB_INCLUDE_DIRS "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE INTERNAL "ZLIB include directories")
  set(ZLIB_LIBRARIES zlibstatic CACHE INTERNAL "ZLIB libraries")
  set(ZLIB_VERSION_STRING "1.3.1" CACHE INTERNAL "ZLIB version")
endif()

message(STATUS "ZLIB 1.3.1 fetched and configured")

#-------------------------------------------------------------------------------
# FetchContent: Download and build EXPAT 2.6.4 as static library
#-------------------------------------------------------------------------------

message(STATUS "Fetching EXPAT 2.6.4 from GitHub...")

FetchContent_Declare(
  expat
  GIT_REPOSITORY https://github.com/libexpat/libexpat.git
  GIT_TAG        R_2_6_4
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   FALSE
  SOURCE_SUBDIR  expat
)

set(EXPAT_SHARED_LIBS    OFF CACHE BOOL "Build shared libraries" FORCE)
set(EXPAT_BUILD_DOCS     OFF CACHE BOOL "Build documentation"    FORCE)
set(EXPAT_BUILD_EXAMPLES OFF CACHE BOOL "Build examples"         FORCE)
set(EXPAT_BUILD_TESTS    OFF CACHE BOOL "Build tests"            FORCE)
set(EXPAT_BUILD_TOOLS    OFF CACHE BOOL "Build tools"            FORCE)
set(EXPAT_BUILD_FUZZERS  OFF CACHE BOOL "Build fuzzers"          FORCE)

FetchContent_MakeAvailable(expat)

if(TARGET expat AND NOT TARGET expat::expat)
  add_library(expat::expat ALIAS expat)
  set(EXPAT_FOUND TRUE CACHE INTERNAL "EXPAT found via FetchContent")
  set(EXPAT_INCLUDE_DIRS "${expat_SOURCE_DIR}/expat/lib" CACHE INTERNAL "EXPAT include directories")
  set(EXPAT_LIBRARIES expat CACHE INTERNAL "EXPAT libraries")
endif()

message(STATUS "EXPAT 2.6.4 fetched and configured")

#-------------------------------------------------------------------------------
# Compile local libraries
#-------------------------------------------------------------------------------

add_subdirectory(OpenSSL)
add_subdirectory(KissFFT)
add_subdirectory(QCodeEditor)
add_subdirectory(QSimpleUpdater)
add_subdirectory(mdflib)


