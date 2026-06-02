#
# Serial Studio — Security hardening flags
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

option(HARDEN_AUTO_ON_RELEASE "Force ENABLE_HARDENING=ON for Release/RelWithDebInfo builds" ON)
option(ENABLE_INTEGRITY_CHECK "MSVC /INTEGRITYCHECK -- requires a SIGNED binary or it won't load" OFF)

#---------------------------------------------------------------------------------------------------
# Auto-enable hardening for optimized configs (prevents accidentally shipping unhardened)
#---------------------------------------------------------------------------------------------------
#
# ENABLE_HARDENING defaults OFF BY DESIGN (fast, debuggable dev builds), and release CI turns it on
# explicitly. This is purely a belt-and-suspenders safety net for a MANUAL Release build that
# forgets the flag; it is a no-op once ENABLE_HARDENING=ON is already set by CI (or by the Flatpak
# branch in Optimization.cmake). Must run before the if(ENABLE_HARDENING) block below.
#
#---------------------------------------------------------------------------------------------------

if(HARDEN_AUTO_ON_RELEASE AND NOT ENABLE_HARDENING)
  if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(ENABLE_HARDENING ON CACHE BOOL "Auto-enabled for optimized build" FORCE)
    message(STATUS "ENABLE_HARDENING auto-forced ON for ${CMAKE_BUILD_TYPE}")
  endif()
endif()

#---------------------------------------------------------------------------------------------------
# Security hardening flags
#---------------------------------------------------------------------------------------------------
#
# Enables defense-in-depth security features for production builds:
#
# When ENABLE_HARDENING=ON is enabled:
#
# Stack protection:
#   -fstack-protector-strong    - Stack canaries for functions with buffers
#   /GS (MSVC)                  - Buffer security checks
#
# Memory protection:
#   -D_FORTIFY_SOURCE=2         - Runtime buffer overflow checks (requires -O1+)
#   -fstack-clash-protection    - Prevent stack clash attacks (GCC 8+/Clang 6+, Linux only)
#
# Control-flow integrity:
#   -fcf-protection=full        - Intel CET (x86-64 Linux only, GCC/Clang)
#   -fsanitize=cfi              - Clang CFI (ARM64 Linux only, requires LTO)
#   /guard:cf (MSVC)            - Control Flow Guard
#   Note: AppleClang does not support -fsanitize=cfi or -fcf-protection
#
# ASLR (Address Space Layout Randomization):
#   -fPIC                       - Position Independent Code (Linux/macOS)
#   /DYNAMICBASE (MSVC)         - ASLR support
#
# RELRO (Relocation Read-Only):
#   -Wl,-z,relro                - Make GOT read-only after relocation
#   -Wl,-z,now                  - Resolve all symbols at startup (full RELRO)
#
# DEP (Data Execution Prevention):
#   -Wl,-z,noexecstack          - Mark stack as non-executable
#   /NXCOMPAT (MSVC)            - DEP support is enabled by default
#
# Additional security:
#   -Wl,-z,separate-code        - Separate code segments (Linux only)
#   -fno-plt                    - Avoid PLT for better performance with PIC
#   -Wformat-security           - Warn about unsafe format strings
#
# Auto-enabled when:
#   - Sandboxed/Flatpak build detected
#   - Manually set via -DENABLE_HARDENING=ON
#   - HARDEN_AUTO_ON_RELEASE (above) for Release/RelWithDebInfo
#
# Note: Some flags require optimization (-O1 or higher) to be effective
#
# clang-cl (Windows, MSVC ABI) takes the same /GS /guard:cf plus link /GUARD:CF /DYNAMICBASE
# /CETCOMPAT as cl.exe. The per-target /Qspectre /sdl /ZH:SHA_256 added in serial_studio_harden are
# skipped for clang-cl (it rejects them); /NXCOMPAT is likewise applied there, not in this block.
#
#---------------------------------------------------------------------------------------------------

if(ENABLE_HARDENING)
   message(STATUS "Enabling security hardening flags for production builds")

   if(MSVC)
      add_compile_options(
         /GS
         /guard:cf
      )
      add_link_options(
         /GUARD:CF
         /DYNAMICBASE
         /CETCOMPAT
      )
   else()
      add_compile_options(
         -fstack-protector-strong
         -Wformat
         -Wformat-security
         -Werror=format-security
      )

      if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT APPLE)
         add_compile_options(-fstack-clash-protection)
      endif()

      if(PRODUCTION_OPTIMIZATION OR CMAKE_BUILD_TYPE STREQUAL "Release")
         add_compile_options(-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2)
      elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
         add_compile_options(-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2)
      else()
         message(WARNING "FORTIFY_SOURCE disabled: requires optimization flags (-O1 or higher)")
      endif()

      set(_hardening_arches "${CMAKE_SYSTEM_PROCESSOR}")
      if(APPLE AND CMAKE_OSX_ARCHITECTURES)
         set(_hardening_arches "${CMAKE_OSX_ARCHITECTURES}")
      endif()
      set(_hardening_arm OFF)
      set(_hardening_x86 OFF)
      foreach(_arch IN LISTS _hardening_arches)
         if(_arch MATCHES "^(aarch64|arm64|ARM64)$")
            set(_hardening_arm ON)
         elseif(_arch MATCHES "^(x86_64|amd64|AMD64)$")
            set(_hardening_x86 ON)
         endif()
      endforeach()
      if(_hardening_arm AND NOT _hardening_x86)
         add_compile_options(-mbranch-protection=standard)
      endif()

      if(NOT APPLE)
         if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
            add_compile_options(
               -fcf-protection=full
            )
         elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
            if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT DISABLE_LTO)
               add_compile_options(
                  -fsanitize=cfi
                  -fvisibility=hidden
               )
               add_link_options(
                  -fsanitize=cfi
               )
            endif()
         endif()
      endif()

      if(UNIX AND NOT APPLE)
         add_compile_options(
            -fPIC
            -fno-plt
         )
         add_link_options(
            -Wl,-z,relro
            -Wl,-z,now
            -Wl,-z,noexecstack
            -Wl,-z,separate-code
         )
      elseif(APPLE)
         add_compile_options(
            -fPIC
         )
      endif()
   endif()
endif()

#---------------------------------------------------------------------------------------------------
# _ss_add_auto_var_init(<target>) — zero-init automatic vars where the compiler supports it
#---------------------------------------------------------------------------------------------------
#
# Eliminates a whole class of uninitialized-read bugs. GCC 12+ / Clang 16+ / AppleClang 15+.
#
#---------------------------------------------------------------------------------------------------

function(_ss_add_auto_var_init _tgt)
  set(_ok OFF)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12)
    set(_ok ON)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16)
    set(_ok ON)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15)
    set(_ok ON)
  endif()
  if(_ok)
    target_compile_options(${_tgt} PRIVATE -ftrivial-auto-var-init=zero)
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
# serial_studio_harden(<target>) — per-target hardening deltas missing from the block above
#---------------------------------------------------------------------------------------------------
#
# Layered on top of the directory-level ENABLE_HARDENING block. Per-target (PRIVATE) so lib/
# static libraries built with /w or -w are unaffected. No-op unless ENABLE_HARDENING is set.
#
#---------------------------------------------------------------------------------------------------

function(serial_studio_harden _tgt)
  if(NOT ENABLE_HARDENING)
    return()
  endif()
  if(NOT TARGET ${_tgt})
    message(WARNING "serial_studio_harden: '${_tgt}' is not a target")
    return()
  endif()

  if(MSVC)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      target_compile_options(${_tgt} PRIVATE
        /Qspectre
        /sdl
        /ZH:SHA_256
      )
    endif()
    target_link_options(${_tgt} PRIVATE
      /HIGHENTROPYVA
      /NXCOMPAT
    )
    if(ENABLE_INTEGRITY_CHECK)
      target_link_options(${_tgt} PRIVATE /INTEGRITYCHECK)
      message(STATUS "${_tgt}: /INTEGRITYCHECK ON (binary MUST be signed)")
    endif()

  elseif(APPLE)
    target_compile_options(${_tgt} PRIVATE
      -fstack-protector-strong
    )
    target_compile_definitions(${_tgt} PRIVATE
      _LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST
    )
    _ss_add_auto_var_init(${_tgt})

  else()
    if(PRODUCTION_OPTIMIZATION
       OR CMAKE_BUILD_TYPE STREQUAL "Release"
       OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      target_compile_options(${_tgt} PRIVATE -U_FORTIFY_SOURCE)
      target_compile_definitions(${_tgt} PRIVATE _FORTIFY_SOURCE=3)
    endif()
    target_compile_definitions(${_tgt} PRIVATE _GLIBCXX_ASSERTIONS)
    _ss_add_auto_var_init(${_tgt})
  endif()

  message(STATUS "applied hardening deltas to ${_tgt}")
endfunction()
