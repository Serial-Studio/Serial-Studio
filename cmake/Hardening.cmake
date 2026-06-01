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
#
# Two layers:
#   1. The directory-level ENABLE_HARDENING block, extracted verbatim from the
#      root CMakeLists.txt. Applies to every target (incl. lib/), as before.
#   2. serial_studio_harden(<target>): the per-target (PRIVATE) deltas the
#      shipped binary was missing. Per-target so lib/ static libraries built
#      with /w or -w are unaffected. Call it on the executable in app/.
#
# include() this module AFTER Optimization.cmake (which may FORCE
# ENABLE_HARDENING ON for sandboxed builds) and after ENABLE_HARDENING is
# declared.
#

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
#---------------------------------------------------------------------------------------------------

if(ENABLE_HARDENING)
   message(STATUS "Enabling security hardening flags for production builds")

   if(MSVC)
      add_compile_options(
         /GS                            # Buffer security check
         /guard:cf                      # Control Flow Guard
         /DYNAMICBASE                   # Address Space Layout Randomization
      )
      add_link_options(
         /GUARD:CF                      # Control Flow Guard at link time
         /DYNAMICBASE                   # ASLR
         /CETCOMPAT                     # Enable Intel CET hardware shadow stack
      )
   else()
      add_compile_options(
         -fstack-protector-strong       # Stack canary protection
         -Wformat                       # Format string warnings
         -Wformat-security              # Format security warnings
         -Werror=format-security        # Make format security errors
      )

      # Stack clash protection (GCC 8+ and Clang 6+, not supported on macOS)
      if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT APPLE)
         add_compile_options(-fstack-clash-protection)
      endif()

      # FORTIFY_SOURCE requires optimization. -U first clears any value the toolchain
      # already injected (GCC built with --enable-default-fortify-source defines it at
      # <command-line> scope), which otherwise triggers a redefinition warning.
      if(PRODUCTION_OPTIMIZATION OR CMAKE_BUILD_TYPE STREQUAL "Release")
         add_compile_options(-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2)
      elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
         add_compile_options(-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2)
      else()
         message(WARNING "FORTIFY_SOURCE disabled: requires optimization flags (-O1 or higher)")
      endif()

      # Control-flow integrity (platform-specific):
      # - x86-64 Linux: Intel CET (Control-flow Enforcement Technology) via -fcf-protection
      # - ARM64 Linux:  Clang CFI (Control Flow Integrity) via -fsanitize=cfi
      # Note: ARM64 CFI requires LTO and is only available with upstream LLVM Clang
      # Note: AppleClang does not support -fsanitize=cfi or -fcf-protection, skip on macOS
      # -mbranch-protection=standard (PAC/BTI) is AArch64-only. Resolve the
      # effective target arch from CMAKE_OSX_ARCHITECTURES on Apple — in a
      # cross / universal build it differs from the host CMAKE_SYSTEM_PROCESSOR
      # (an arm64 runner cross-building x86_64 still reports arm64 there). Only
      # add the flag when every target slice is arm64; an x86_64 slice rejects
      # it ("unsupported option '-mbranch-protection='").
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
               -fcf-protection=full        # Control-flow protection (Intel CET)
            )
         elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
            if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT DISABLE_LTO)
               add_compile_options(
                  -fsanitize=cfi            # Control Flow Integrity for ARM64
                  -fvisibility=hidden       # Required for CFI (symbols must be hidden)
               )
               add_link_options(
                  -fsanitize=cfi            # Link with CFI support
               )
            endif()
         endif()
      endif()

      if(UNIX AND NOT APPLE)
         add_compile_options(
            -fPIC                        # Position Independent Code
            -fno-plt                     # Avoid PLT for better performance with PIC
         )
         add_link_options(
            -Wl,-z,relro                 # Make relocation sections read-only after relocation
            -Wl,-z,now                   # Resolve all symbols at startup (full RELRO)
            -Wl,-z,noexecstack           # Mark stack as non-executable
            -Wl,-z,separate-code         # Separate code segments for better security
         )
      elseif(APPLE)
         add_compile_options(
            -fPIC                        # Position Independent Executable
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
    # NOTE: /guard:ehcont (EH continuation guard) is intentionally NOT used. The link-time form
    # requires EVERY linked module to carry EHCONT metadata, but the vendored static libs in lib/
    # (lua54, mdf, QCodeEditor, QSimpleUpdater, libusb, hidapi, zlib, expat) are not built with it,
    # so linking fails with LNK1386 (and /force:guardehcont only emits conservative metadata while
    # leaving ~100 LNK4291 warnings). Revisit only if every dependency is rebuilt with /guard:ehcont.
    #-- MSVC compile deltas -------------------------------------------------------------------
    target_compile_options(${_tgt} PRIVATE
      /Qspectre            # Spectre v1 (load-hardening) mitigations
      /sdl                 # Additional security dev-lifecycle checks + warnings
      /ZH:SHA_256          # SHA-256 source hashing in the PDB (integrity of symbol mapping)
    )
    #-- MSVC link deltas ----------------------------------------------------------------------
    target_link_options(${_tgt} PRIVATE
      /HIGHENTROPYVA       # 64-bit high-entropy ASLR (explicit; default-on but pin it)
      /NXCOMPAT            # DEP (explicit)
    )
    if(ENABLE_INTEGRITY_CHECK)
      # Forces the loader to verify the Authenticode signature of the image AND its dependent
      # DLLs. The binary will FAIL TO LOAD if unsigned or tampered. Only turn on once signing
      # is wired up end-to-end.
      target_link_options(${_tgt} PRIVATE /INTEGRITYCHECK)
      message(STATUS "${_tgt}: /INTEGRITYCHECK ON (binary MUST be signed)")
    endif()

  elseif(APPLE)
    #-- AppleClang (libc++) -------------------------------------------------------------------
    target_compile_options(${_tgt} PRIVATE
      -fstack-protector-strong
    )
    # libc++ hardened mode (libc++ 18+); harmless on older libc++.
    target_compile_definitions(${_tgt} PRIVATE
      _LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST
    )
    _ss_add_auto_var_init(${_tgt})

  else()
    #-- GCC / Clang on Linux ------------------------------------------------------------------
    # Upgrade FORTIFY 2 -> 3 (better object-size coverage), but only where the block above
    # actually set =2 (optimized builds) -- bumping it without -O1+ just emits a warning.
    # -U first avoids a redefinition warning against the =2 set above.
    if(PRODUCTION_OPTIMIZATION
       OR CMAKE_BUILD_TYPE STREQUAL "Release"
       OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      target_compile_options(${_tgt} PRIVATE -U_FORTIFY_SOURCE)
      target_compile_definitions(${_tgt} PRIVATE _FORTIFY_SOURCE=3)
    endif()
    # Hardened libstdc++ (bounds checks in std containers/iterators; low overhead).
    target_compile_definitions(${_tgt} PRIVATE _GLIBCXX_ASSERTIONS)
    _ss_add_auto_var_init(${_tgt})
  endif()

  message(STATUS "applied hardening deltas to ${_tgt}")
endfunction()
