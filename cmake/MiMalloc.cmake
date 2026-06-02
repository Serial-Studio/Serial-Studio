#
# Serial Studio — mimalloc allocator override (Windows/MSVC + Linux)
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
# SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
#
# The frame-parse hotpath allocates many small QString / QByteArray / Lua buffers per frame. On the
# MSVC CRT heap that is markedly slower than glibc (Linux) or the macOS allocator, which is why the
# Windows hotpath benchmark lagged the other platforms (~256 kHz vs ~400 kHz). On Windows mimalloc's
# redirect DLL patches the process-wide malloc at load time; on Linux libmimalloc.so interposes
# malloc/free via ELF symbol resolution. Either way every allocation — including those inside the
# Qt libraries — is served by mimalloc instead of the system heap.
#
# Windows/MSVC and Linux only; macOS opts out (good system allocator, fragile DYLD interposing).
#
# Must be include()d BEFORE Optimization.cmake so mimalloc builds with its own flags and is not
# swept into the application's whole-program-optimization / PGO passes.
#
# Usage: call target_link_mimalloc(<target>) on the executable.
#

include_guard(GLOBAL)

set(SS_MIMALLOC_PLATFORM FALSE)
if((WIN32 AND MSVC) OR (UNIX AND NOT APPLE))
  set(SS_MIMALLOC_PLATFORM TRUE)
endif()

if(SS_MIMALLOC_PLATFORM)
  include(FetchContent)

  set(MI_OVERRIDE     ON  CACHE BOOL "" FORCE)
  set(MI_BUILD_SHARED ON  CACHE BOOL "" FORCE)
  set(MI_BUILD_STATIC OFF CACHE BOOL "" FORCE)
  set(MI_BUILD_OBJECT OFF CACHE BOOL "" FORCE)
  set(MI_BUILD_TESTS  OFF CACHE BOOL "" FORCE)

  if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(MI_USE_CXX ON CACHE BOOL "" FORCE)
  endif()

  FetchContent_Declare(
    mimalloc
    GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
    GIT_TAG        v2.1.9
    GIT_SHALLOW    TRUE
  )
  FetchContent_MakeAvailable(mimalloc)

  if(MSVC)
    target_compile_options(mimalloc PRIVATE /w)
  else()
    target_compile_options(mimalloc PRIVATE -w)
  endif()

  if(WIN32 AND MSVC)
    set(SS_MIMALLOC_REDIRECT_DLL "${mimalloc_SOURCE_DIR}/bin/mimalloc-redirect.dll"
        CACHE INTERNAL "Path to the prebuilt mimalloc-redirect.dll")
  endif()
endif()

function(target_link_mimalloc target)
  if(NOT ((WIN32 AND MSVC) OR (UNIX AND NOT APPLE)))
    return()
  endif()

  if(WIN32 AND MSVC)
    target_link_libraries(${target} PRIVATE mimalloc)
    target_link_options(${target} PRIVATE "/INCLUDE:mi_version")
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              $<TARGET_FILE:mimalloc> $<TARGET_FILE_DIR:${target}>
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${SS_MIMALLOC_REDIRECT_DLL}" $<TARGET_FILE_DIR:${target}>
      VERBATIM)

    install(FILES $<TARGET_FILE:mimalloc> "${SS_MIMALLOC_REDIRECT_DLL}" DESTINATION bin)
    return()
  endif()

  target_link_libraries(${target} PRIVATE "-Wl,--no-as-needed" mimalloc)
endfunction()
