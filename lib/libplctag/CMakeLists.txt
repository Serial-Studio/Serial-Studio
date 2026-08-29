#
# Serial Studio
# https://serial-studio.com/
#
# Copyright (C) 2020–2026 Alex Spataru
#
# This file is dual-licensed:
#
# - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
# - Under the Serial Studio Commercial License for builds that include
#   any Pro functionality.
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
#

cmake_minimum_required(VERSION 3.20)

#---------------------------------------------------------------------------------------------------
# libplctag (EtherNet/IP + CIP symbolic tag client), spec 0073
#---------------------------------------------------------------------------------------------------
#
# libplctag is FETCHED rather than vendored: it is a self-contained C library nothing but the
# EtherNet/IP driver touches, and keeping the sources out of the repository keeps the REUSE
# declaration to this file alone.
#
# Upstream ships a well-formed CMake project, so it is added as a subproject and its own static
# target is reused. The target NAME has changed across upstream releases, so the concrete one is
# resolved from a candidate list instead of being hardcoded: a renamed target is then a
# configure-time message naming the fix, not an unresolved symbol at link time.
#
# libplctag is MPL-2.0 (LICENSES/MPL-2.0.txt already ships for open62541). Static linking is
# unencumbered under the MPL's file-level copyleft as long as the library's own sources stay
# available, which the pin below records.
#
# Version pinning: SS_LIBPLCTAG_GIT_REPOSITORY / SS_LIBPLCTAG_GIT_TAG are cache variables so a
# maintainer can move the pin (or point at an internal mirror) without editing this file.
#
#---------------------------------------------------------------------------------------------------

set(SS_LIBPLCTAG_GIT_REPOSITORY "https://github.com/libplctag/libplctag.git"
    CACHE STRING "Upstream libplctag repository")
set(SS_LIBPLCTAG_GIT_TAG "v2.6.0"
    CACHE STRING "Upstream libplctag revision (tag or commit)")
set(SS_LIBPLCTAG_VERSION "2.6.0" CACHE INTERNAL "libplctag release this pin tracks")

include(FetchContent)

message(STATUS "Fetching libplctag ${SS_LIBPLCTAG_VERSION} (${SS_LIBPLCTAG_GIT_TAG})...")

set(BUILD_EXAMPLES     OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS        OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING      OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS  OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  libplctag
  GIT_REPOSITORY ${SS_LIBPLCTAG_GIT_REPOSITORY}
  GIT_TAG        ${SS_LIBPLCTAG_GIT_TAG}
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   FALSE
)

FetchContent_MakeAvailable(libplctag)

#
# Resolve the concrete static target once. Upstream has shipped it as plctag_static and as
# plctag across the 2.x line; a future rename lands here as a readable configure error.
#
set(SS_LIBPLCTAG_TARGET "")
foreach(_candidate plctag_static plctag libplctag_static libplctag)
  if(TARGET ${_candidate} AND NOT SS_LIBPLCTAG_TARGET)
    get_target_property(_candidate_type ${_candidate} TYPE)
    if(_candidate_type STREQUAL "STATIC_LIBRARY")
      set(SS_LIBPLCTAG_TARGET ${_candidate})
    endif()
  endif()
endforeach()

if(NOT SS_LIBPLCTAG_TARGET)
  message(FATAL_ERROR
    "libplctag did not expose a static library target. Add the upstream target name to the "
    "candidate list in lib/libplctag/CMakeLists.txt, or configure with SS_ENABLE_EIP=OFF.")
endif()

set(SS_LIBPLCTAG_TARGET ${SS_LIBPLCTAG_TARGET} CACHE INTERNAL "Resolved libplctag static target")

if(MSVC)
  target_compile_options(${SS_LIBPLCTAG_TARGET} PRIVATE /w)
else()
  target_compile_options(${SS_LIBPLCTAG_TARGET} PRIVATE -w)
endif()

#
# libplctag.h lives in src/lib; upstream's own target exports it, but the include directory is
# restated SYSTEM so its diagnostics never reach the driver's translation unit.
#
if(EXISTS "${libplctag_SOURCE_DIR}/src/lib")
  target_include_directories(${SS_LIBPLCTAG_TARGET} SYSTEM PUBLIC "${libplctag_SOURCE_DIR}/src/lib")
endif()

#
# Links the client into an application target; the BUILD_COMMERCIAL guard mirrors
# target_link_open62541().
#
function(target_link_libplctag TARGET)
  if(NOT SS_LIBPLCTAG_TARGET OR NOT BUILD_COMMERCIAL)
    return()
  endif()

  target_compile_definitions(${TARGET} PRIVATE SS_EIP_ACTIVE=1)
  target_link_libraries(${TARGET} PRIVATE ${SS_LIBPLCTAG_TARGET})
endfunction()
