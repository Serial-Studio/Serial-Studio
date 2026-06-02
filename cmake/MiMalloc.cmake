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

include_guard(GLOBAL)

#---------------------------------------------------------------------------------------------------
# mimalloc allocator override
#---------------------------------------------------------------------------------------------------
#
# The frame-parse hotpath allocates many small QString/QByteArray/Lua buffers per frame, which the
# system heap (the MSVC CRT on Windows, glibc on Linux) serves more slowly than mimalloc; glibc also
# thrashes its per-thread arenas under the cross-thread alloc/free pattern (main thread allocates,
# exporter workers free). This module FetchContent-builds mimalloc and wires a process-wide override:
#
#   Windows/MSVC (incl. clang-cl)  shared mimalloc.dll + mimalloc-redirect.dll, which patches malloc
#                                  at load time across the whole process, the prebuilt Qt DLLs
#                                  included. clang-cl builds mimalloc as C++ (MI_USE_CXX) to avoid a
#                                  C-atomics miscompile in segment-map.c.
#   Linux                          shared libmimalloc.so linked with -Wl,--no-as-needed so its
#                                  malloc/free interpose over glibc via ELF symbol resolution.
#   macOS                          opts out (good system allocator, fragile DYLD interposing).
#
# mimalloc's own warnings are silenced (third-party). Include BEFORE Optimization.cmake so it is not
# swept into LTO/PGO. Call target_link_mimalloc(<target>) on the executable.
#
#---------------------------------------------------------------------------------------------------

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

  # Skip the allocator override under ASan/UBSan/TSan: mimalloc interposes
  # malloc/free, which collides with the sanitizer's own allocator and
  # produces shadow-memory corruption, bogus reports, or a pre-main crash.
  if(DEBUG_SANITIZER OR ENABLE_TSAN)
    message(STATUS "mimalloc disabled for sanitizer build (${target})")
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
