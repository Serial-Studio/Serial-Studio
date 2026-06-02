#
# Serial Studio — mimalloc allocator override (Windows / MSVC only)
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
# Windows hotpath benchmark lagged the other platforms (~256 kHz vs ~400 kHz). mimalloc's redirect
# DLL patches the process-wide malloc at load time, so every allocation — including those made
# inside the prebuilt Qt DLLs — is served by mimalloc instead of the CRT heap.
#
# Windows/MSVC only; every other platform already clears the gate, so this is a no-op there.
#
# Must be include()d BEFORE Optimization.cmake so mimalloc builds with its own flags and is not
# swept into the application's whole-program-optimization / PGO passes.
#
# Usage: call serial_studio_link_mimalloc(<target>) on the executable.
#

include_guard(GLOBAL)

if(WIN32 AND MSVC)
  include(FetchContent)

  # Build only the override shared DLL; skip the static/object libs and the test executables.
  set(MI_OVERRIDE     ON  CACHE BOOL "" FORCE)
  set(MI_BUILD_SHARED ON  CACHE BOOL "" FORCE)
  set(MI_BUILD_STATIC OFF CACHE BOOL "" FORCE)
  set(MI_BUILD_OBJECT OFF CACHE BOOL "" FORCE)
  set(MI_BUILD_TESTS  OFF CACHE BOOL "" FORCE)

  FetchContent_Declare(
    mimalloc
    GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
    GIT_TAG        v2.1.9
    GIT_SHALLOW    TRUE
  )
  FetchContent_MakeAvailable(mimalloc)

  # Cache the redirect-DLL path so the link helper (called from a different scope) can find it.
  set(SS_MIMALLOC_REDIRECT_DLL "${mimalloc_SOURCE_DIR}/bin/mimalloc-redirect.dll"
      CACHE INTERNAL "Path to the prebuilt mimalloc-redirect.dll")
endif()

#
# @brief Links mimalloc into <target> and deploys its two DLLs next to the executable.
#
function(serial_studio_link_mimalloc target)
  if(NOT (WIN32 AND MSVC))
    return()
  endif()

  target_link_libraries(${target} PRIVATE mimalloc)

  # Force the linker to keep the mimalloc import so mimalloc.dll loads at process start; only then
  # can mimalloc-redirect.dll patch the CRT before the first allocation happens.
  target_link_options(${target} PRIVATE "/INCLUDE:mi_version")

  # Copy mimalloc.dll (built) + mimalloc-redirect.dll (shipped in the repo's bin/) into the build
  # output so build-tree runs — like the CI hotpath benchmark — pick them up automatically.
  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:mimalloc> $<TARGET_FILE_DIR:${target}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SS_MIMALLOC_REDIRECT_DLL}" $<TARGET_FILE_DIR:${target}>
    VERBATIM)

  # Ship both DLLs in the installer alongside the executable.
  install(FILES $<TARGET_FILE:mimalloc> "${SS_MIMALLOC_REDIRECT_DLL}" DESTINATION bin)
endfunction()
