#
# Serial Studio — mimalloc allocator override (Windows/MSVC + Linux + macOS)
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
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial

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
#   macOS                          static libmimalloc.a linked with -Wl,-force_load so its
#                                  __DATA,__interpose entries override malloc process-wide with no
#                                  injected dylib, surviving SIP / hardened runtime / notarization.
#                                  force_load also pulls the operator new/delete overrides.
#
# mimalloc's own warnings are silenced (third-party). Include BEFORE Optimization.cmake so it is not
# swept into LTO/PGO. Call target_link_mimalloc(<target>) on the executable.
#
# Set -DSS_USE_MIMALLOC=OFF to skip the GitHub FetchContent entirely (e.g. Flathub/offline builds
# with no network access); target_link_mimalloc() then becomes a no-op and the system heap is used.
#
#---------------------------------------------------------------------------------------------------

option(SS_MIMALLOC_ENABLE_APPLE "Enable mimalloc static override on macOS (advanced opt-out)" ON)
mark_as_advanced(SS_MIMALLOC_ENABLE_APPLE)

set(SS_MIMALLOC_PLATFORM FALSE)
if(SS_USE_MIMALLOC AND ((WIN32 AND MSVC)
                        OR (UNIX AND NOT APPLE)
                        OR (APPLE AND SS_MIMALLOC_ENABLE_APPLE)))
  set(SS_MIMALLOC_PLATFORM TRUE)
endif()

if(SS_MIMALLOC_PLATFORM)
  include(FetchContent)

  set(MI_OVERRIDE     ON  CACHE BOOL "" FORCE)
  set(MI_BUILD_OBJECT OFF CACHE BOOL "" FORCE)
  set(MI_BUILD_TESTS  OFF CACHE BOOL "" FORCE)

  # Skip the final heap collect at process exit: a long capture session can hold gigabytes of
  # cold (paged-out) history, and touching every page just to free it makes quitting slow. The
  # OS reclaims the whole address space anyway.
  set(MI_SKIP_COLLECT_ON_EXIT ON CACHE BOOL "" FORCE)

  if(WIN32 AND MSVC)
    # Pin mimalloc's per-thread heap pointer to a fixed TLS slot. mimalloc.dll loads before the
    # app (import-time redirect), so TlsAlloc() lands on a low slot and every malloc/free skips
    # the dynamic TLS lookup. Only safe because we ship the DLL with the app.
    set(MI_WIN_USE_FIXED_TLS ON CACHE BOOL "" FORCE)
  endif()

  if(APPLE)
    # armv8.1-a LSE atomics for the malloc fast path. Auto-enabled for plain arm64 builds, but a
    # universal (x86_64;arm64) build detects as x64 and silently drops it; forcing ON keeps the
    # arm64 slice fast either way (mimalloc scopes the flag with -Xarch_arm64).
    set(MI_OPT_ARCH ON CACHE BOOL "" FORCE)
  endif()

  if(APPLE)
    set(MI_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(MI_BUILD_STATIC ON  CACHE BOOL "" FORCE)
  else()
    set(MI_BUILD_SHARED ON  CACHE BOOL "" FORCE)
    set(MI_BUILD_STATIC OFF CACHE BOOL "" FORCE)
  endif()

  if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(MI_USE_CXX ON CACHE BOOL "" FORCE)
  endif()

  FetchContent_Declare(
    mimalloc
    GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
    # A tag is mutable; the commit it named when this pin was written is not.
    GIT_TAG        acf2fdd329f9dc2a7ffe3f12a133fe7175e39378 # v3.4.5
  )
  FetchContent_MakeAvailable(mimalloc)

  if(APPLE)
    target_compile_options(mimalloc-static PRIVATE -w)
  elseif(MSVC)
    target_compile_options(mimalloc PRIVATE /w)
  else()
    target_compile_options(mimalloc PRIVATE -w)
  endif()

  if(WIN32 AND MSVC)
    # Mirror mimalloc's own per-arch redirect selection (MIMALLOC_REDIRECT_SUFFIX in its
    # CMakeLists): x64 -> "", x86 -> "32", arm64 -> "-arm64", arm64ec -> "-arm64ec". Shipping the
    # x64 DLL on another arch makes load-time patching fail silently and the CRT heap is used.
    # Both VS (CMAKE_GENERATOR_PLATFORM) and Ninja (CMAKE_SYSTEM_PROCESSOR) builds are covered.
    if(CMAKE_GENERATOR_PLATFORM STREQUAL "arm64ec")
      set(_ss_mi_redirect_suffix "-arm64ec")
    elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^(ARM64|arm64)$"
           OR (NOT CMAKE_GENERATOR_PLATFORM
               AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$"))
      set(_ss_mi_redirect_suffix "-arm64")
    elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^(x86|Win32)$"
           OR (NOT CMAKE_GENERATOR_PLATFORM AND CMAKE_SIZEOF_VOID_P EQUAL 4))
      set(_ss_mi_redirect_suffix "32")
    else()
      set(_ss_mi_redirect_suffix "")
    endif()
    set(SS_MIMALLOC_REDIRECT_DLL
        "${mimalloc_SOURCE_DIR}/bin/mimalloc-redirect${_ss_mi_redirect_suffix}.dll"
        CACHE INTERNAL "Path to the prebuilt mimalloc-redirect DLL for the target architecture")
  endif()
endif()

function(target_link_mimalloc target)
  if(NOT SS_MIMALLOC_PLATFORM)
    return()
  endif()

  if(DEBUG_SANITIZER OR ENABLE_TSAN)
    message(STATUS "mimalloc disabled for sanitizer build (${target})")
    return()
  endif()

  # The Windows redirect DLL patches malloc at load time and must initialize before the CRT. A
  # Debug build loads the debug CRT (ucrtbased) first, so the redirect loses the race, falls back
  # to the system heap, and a half-redirected allocator over the debug CRT crashes on the first
  # cross-boundary free. mimalloc is a release-hotpath optimization only, so skip it here rather
  # than ship a Debug build that fails to launch.
  if(WIN32 AND MSVC AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "mimalloc disabled for Windows Debug build (${target})")
    return()
  endif()

  # Expose <mimalloc.h> so the app can tune runtime options (see main.cpp). The macro gates those
  # calls out of builds where the override is absent (SS_USE_MIMALLOC=OFF, sanitizers, other OSes).
  target_compile_definitions(${target} PRIVATE SS_MIMALLOC_ACTIVE=1)
  target_include_directories(${target} PRIVATE "${mimalloc_SOURCE_DIR}/include")

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

  if(APPLE)
    add_dependencies(${target} mimalloc-static)
    target_link_options(${target} PRIVATE "-Wl,-force_load,$<TARGET_FILE:mimalloc-static>")
    return()
  endif()

  target_link_libraries(${target} PRIVATE "-Wl,--no-as-needed" mimalloc)
endfunction()
