#
# Serial Studio — Optimization flags (production optimization, SIMD, PGO)
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
# Extracted verbatim from the root CMakeLists.txt. include() this module in
# place of the old inline blocks; include() preserves directory scope, so the
# directory-level add_compile_options/add_link_options behave exactly as before.
# Requires the PRODUCTION_OPTIMIZATION and ENABLE_PGO options to be declared.
#

include_guard(GLOBAL)

#---------------------------------------------------------------------------------------------------
# Production optimization flags
#---------------------------------------------------------------------------------------------------
#
# High-performance compilation flags for release builds:
#
# When PRODUCTION_OPTIMIZATION=ON is enabled, the build system applies
# aggressive optimization flags tailored to each compiler and platform:
#
# Common optimizations across all platforms:
#   -O3                    - Maximum optimization level
#   -ftree-vectorize       - Auto-vectorization (SIMD)
#   -funroll-loops         - Loop unrolling for better IPC
#   -fomit-frame-pointer   - Eliminate frame pointer overhead
#   -fno-fast-math         - IEEE 754 compliance (no unsafe math)
#   -finline-functions     - Aggressive function inlining
#   -ffunction-sections    - Separate sections per function
#   -fdata-sections        - Separate sections per data item
#   -flto=auto             - Link-Time Optimization (LTO) with parallelization
#   -Wl,--gc-sections      - Remove unused code/data at link time
#
# Architecture-specific flags:
#   x86-64: -march=x86-64-v2 (SSE4.2, SSSE3, modern CPUs from 2012+)
#   ARM64:  -march=armv8-a (ARMv8 baseline for aarch64)
#   ARMv7:  -march=armv7-a -mfpu=neon (NEON SIMD for 32-bit ARM)
#
# Compiler-specific optimizations:
#   GCC:   -frename-registers, -fstack-reuse=all
#   MSVC:  /GL (whole program optimization), /Gw (COMDAT sections), /Qpar
#   Intel: Same as GCC/Clang with additional Intel-specific opts
#
# Sandboxed builds (Flathub):
#   - Detects FLATPAK_BUILD environment
#   - Disables LTO (causes issues in sandboxed environments)
#   - Auto-enables ENABLE_HARDENING for security compliance
#
# Expected performance gain: 20-40% compared to -O2 without LTO
# Build time impact: +50-100% due to LTO (can be disabled if needed)
#
#---------------------------------------------------------------------------------------------------

if(PRODUCTION_OPTIMIZATION)
   # Detect sandboxed build environments (Flathub) and enable hardening
   if(DEFINED ENV{FLATPAK_ID} OR FLATPAK_BUILD)
      set(DISABLE_LTO ON)
      set(ENABLE_HARDENING ON CACHE BOOL "Auto-enabled hardening for sandboxed builds" FORCE)
      message(STATUS "Sandboxed build detected, disabling LTO and enabling hardening")
   else()
      set(DISABLE_LTO OFF)
   endif()

   # Print status
   message("Enabling production optimization flags...")

   # Add NDEBUG definition for all configurations
   add_compile_definitions(NDEBUG)

   # Force unwind tables on every translation unit, regardless of optimization
   # level. Lua-as-C++ throws lua_longjmp* across the VM call stack on every
   # runtime error; if the parent project applies -ffunction-sections +
   # -dead_strip / --gc-sections + LTO without preserving unwind metadata for
   # functions that only appear on a thrown-exception path, the unwinder hits
   # a missing personality routine and aborts via std::terminate. The crash
   # symptom is __cxa_throw -> demangling_terminate_handler -> abort even
   # though the underlying error is a routine Lua-level type mismatch like
   # "string expected, got table". The flags below ensure every C/C++ object
   # we link carries enough unwind data for the C++ runtime to walk through
   # it cleanly.
   if(NOT MSVC)
      add_compile_options(
         -fexceptions                    # Enable C++ exceptions in all TUs
         -funwind-tables                 # Emit unwind tables for every function
         -fasynchronous-unwind-tables    # ... including async-safe ones
      )
      if(APPLE)
         # macOS: prevent the linker from discarding compact unwind data when
         # paired with -dead_strip + LTO. Without this, throws across stripped
         # functions hit _Unwind_Resume -> terminate.
         add_link_options(-Wl,-keep_dwarf_unwind)
      endif()
   endif()

   # LLVM (MinGW) on Windows
   if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW)
      message(STATUS "Production branch: Clang/MinGW (Windows)")
      add_compile_options(
         -O3                             # Aggressive optimization (implies -ftree-vectorize, -finline-functions)
         -march=x86-64-v2                # Target modern CPUs (2012+) with SSE4.2
         -funroll-loops                  # Loop unrolling (NOT implied by -O3)
         -fomit-frame-pointer            # Free up RBP for the register allocator
         -fno-fast-math                  # IEEE-compliant floating point (telemetry correctness)
         -fno-unsafe-math-optimizations  # Avoid unsafe math assumptions
         -ffunction-sections             # Pair with --gc-sections for dead-code stripping
         -fdata-sections                 # Pair with --gc-sections for dead-data stripping
      )
      if(NOT DISABLE_LTO)
         add_compile_options(-flto=auto) # Link-time optimization with parallelization
      endif()
      add_link_options(
         -Wl,--gc-sections               # Remove unused functions/data from final binary
      )
      if(NOT DISABLE_LTO)
         add_link_options(-flto=auto)    # Enable LTO at link time
      endif()

   # GCC (MinGW) on Windows
   elseif(WIN32 AND MINGW)
      message(STATUS "Production branch: GCC/MinGW (Windows)")
      add_compile_options(
         -O3                             # Aggressive optimization (implies -ftree-vectorize, -finline-functions)
         -march=x86-64-v2                # Target modern CPUs (2012+) with SSE4.2
         -funroll-loops                  # Loop unrolling (NOT implied by -O3)
         -fomit-frame-pointer            # Free up RBP for the register allocator
         -fno-fast-math                  # IEEE-compliant floating point (telemetry correctness)
         -fno-unsafe-math-optimizations  # Avoid unsafe math assumptions
         -fno-semantic-interposition     # Allow inlining/devirt of own symbols (PIC perf win)
         -ffunction-sections             # Pair with --gc-sections for dead-code stripping
         -fdata-sections                 # Pair with --gc-sections for dead-data stripping
      )
      if(NOT DISABLE_LTO)
         add_compile_options(-flto=auto) # Link-time optimization with parallelization
      endif()
      add_link_options(
         -Wl,--gc-sections               # Remove unused code/data at link time
      )
      if(NOT DISABLE_LTO)
         add_link_options(-flto=auto)    # Enable LTO at link time
      endif()

   # clang-cl (Clang with the MSVC driver + ABI). Generates markedly faster code than cl.exe on the
   # scalar parse hotpath while still linking against the prebuilt MSVC Qt. clang-cl has MSVC=ON, so
   # this branch must precede the cl.exe branch below. No /GL or /LTCG (MSVC-only) -- ThinLTO would
   # need lld-link; PGO (handled in the Clang branch of the PGO section) is the main win here.
   elseif(WIN32 AND MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      message(STATUS "Production branch: clang-cl (Windows, MSVC ABI)")

      # CMake injects /Ob2 in *_FLAGS_RELEASE* and /W3 in *_FLAGS; strip them so our /O2 + the
      # per-target /W4 win without a flood of D9025 "overriding" diagnostics.
      foreach(_lang CXX C)
         string(REGEX REPLACE "[/-]W[0-9]" ""
            CMAKE_${_lang}_FLAGS "${CMAKE_${_lang}_FLAGS}")
         foreach(_cfg RELEASE RELWITHDEBINFO MINSIZEREL)
            string(REGEX REPLACE "[/-]Ob[0-9]" ""
               CMAKE_${_lang}_FLAGS_${_cfg} "${CMAKE_${_lang}_FLAGS_${_cfg}}")
            string(REGEX REPLACE "[/-]W[0-9]" ""
               CMAKE_${_lang}_FLAGS_${_cfg} "${CMAKE_${_lang}_FLAGS_${_cfg}}")
         endforeach()
      endforeach()

      add_compile_options(
         /O2                             # clang -O2: aggressive optimization + auto-vectorization
         /Oi                             # Enable intrinsic functions
         /Ot                             # Favor fast code over small code
         /fp:precise                     # IEEE-accurate floating point (telemetry correctness)
         /DNDEBUG                        # Disable assertions
      )
      add_link_options(
         /OPT:REF                        # Remove unreferenced functions/data
         /OPT:ICF                        # Merge identical code/data blocks
      )

   # MSVC (Visual Studio)
   elseif(WIN32 AND MSVC)
      message(STATUS "Production branch: MSVC (Windows)")

      # CMake injects /Ob2 in *_FLAGS_RELEASE / *_FLAGS_RELWITHDEBINFO and /W3
      # in the unsuffixed *_FLAGS. Stripping them lets us force /Ob3 globally
      # and /W4 per-target without 500+ "warning D9025: overriding" messages.
      foreach(_lang CXX C)
         string(REGEX REPLACE "[/-]W[0-9]" ""
            CMAKE_${_lang}_FLAGS "${CMAKE_${_lang}_FLAGS}")
         foreach(_cfg RELEASE RELWITHDEBINFO MINSIZEREL)
            string(REGEX REPLACE "[/-]Ob[0-9]" ""
               CMAKE_${_lang}_FLAGS_${_cfg} "${CMAKE_${_lang}_FLAGS_${_cfg}}")
            string(REGEX REPLACE "[/-]W[0-9]" ""
               CMAKE_${_lang}_FLAGS_${_cfg} "${CMAKE_${_lang}_FLAGS_${_cfg}}")
         endforeach()
      endforeach()

      add_compile_options(
         /permissive-                    # Enforce strict ISO C++ conformance
         /Zc:__cplusplus                 # Set __cplusplus to actual version value
         /Zc:preprocessor                # Standards-compliant preprocessor
         /MP                             # Enable multi-core compilation
         /O2                             # Optimize for speed
         /Ot                             # Favor fast code over small code
         /Oi                             # Enable intrinsic functions
         /Ob3                            # Aggressive inlining (VS2019 16.3+, forced over CMake's /Ob2)
         /fp:precise                     # IEEE-accurate floating-point math
         /Gw                             # Put functions/data into separate COMDATs
         /Gy                             # Function-level linking (pairs with /OPT:REF)
         /DNDEBUG                        # Disable assertions
      )
      # /W4 is applied per-target on PROJECT_EXECUTABLE in app/CMakeLists.txt
      # so third-party libs in lib/ (which use /w) don't trigger D9025 overrides.
      if(NOT DISABLE_LTO)
         add_compile_options(/GL)        # Enable whole program optimization
      endif()
      add_link_options(
         /OPT:REF                        # Remove unreferenced functions/data
         /OPT:ICF                        # Merge identical code/data blocks
      )
      if(NOT DISABLE_LTO)
         add_link_options(/LTCG)         # Link-time optimization
      endif()

   # macOS (Clang/AppleClang)
   elseif(APPLE)
      message(STATUS "Production branch: AppleClang (macOS)")
      add_compile_options(
         -O3                             # Aggressive optimization (implies -ftree-vectorize, -finline-functions)
         -funroll-loops                  # Loop unrolling (NOT implied by -O3)
         -fomit-frame-pointer            # Free up RBP/X29 for the register allocator
         -fno-fast-math                  # IEEE-compliant floating point (telemetry correctness)
         -fno-unsafe-math-optimizations  # Avoid unsafe math assumptions
         -ffunction-sections             # Pair with -dead_strip for dead-code stripping
         -fdata-sections                 # Pair with -dead_strip for dead-data stripping
      )
      if(NOT DISABLE_LTO)
         add_compile_options(-flto=auto) # Link-time optimization with parallelization
      endif()
      add_link_options(
         -Wl,-dead_strip                 # Remove unused functions/data from final binary
      )
      if(NOT DISABLE_LTO)
         add_link_options(-flto=auto)    # Enable Link-Time Optimization
      endif()

      if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
         add_compile_options(-march=x86-64-v2)
      endif()

   # Intel LLVM Compiler (Linux or Windows)
   elseif(CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
      message(STATUS "Production branch: IntelLLVM")
      add_compile_options(
         -O3                             # Aggressive optimization (implies -ftree-vectorize, -finline-functions)
         -march=x86-64-v2                # Target modern CPUs (2012+) with SSE4.2
         -static                         # Include Intel dependencies
         -funroll-loops                  # Loop unrolling (NOT implied by -O3)
         -fomit-frame-pointer            # Free up RBP for the register allocator
         -fno-fast-math                  # IEEE-compliant floating point (telemetry correctness)
         -fno-unsafe-math-optimizations  # Avoid unsafe math assumptions
         -fno-semantic-interposition     # Allow inlining/devirt of own symbols (PIC perf win)
         -ffunction-sections             # Pair with --gc-sections for dead-code stripping
         -fdata-sections                 # Pair with --gc-sections for dead-data stripping
      )
      if(NOT DISABLE_LTO)
         add_compile_options(-flto=auto) # Link-time optimization with parallelization
      endif()
      add_link_options(
         -Wl,--gc-sections               # Discard unused code sections at link time
      )
      if(NOT DISABLE_LTO)
         add_link_options(-flto=auto)    # Enable LTO at link time
      endif()

   # Generic Linux (GCC or Clang)
   elseif(UNIX)
      message(STATUS "Production branch: ${CMAKE_CXX_COMPILER_ID} (Linux/${CMAKE_SYSTEM_PROCESSOR})")
      add_compile_options(
         -O3                             # Aggressive optimization (implies -ftree-vectorize, -finline-functions)
         -funroll-loops                  # Loop unrolling (NOT implied by -O3)
         -fomit-frame-pointer            # Free up RBP/X29 for the register allocator
         -fno-fast-math                  # IEEE-compliant floating point (telemetry correctness)
         -fno-unsafe-math-optimizations  # Avoid unsafe math assumptions
         -ffunction-sections             # Pair with --gc-sections for dead-code stripping
         -fdata-sections                 # Pair with --gc-sections for dead-data stripping
      )
      # GCC-only: -fno-semantic-interposition is silently rejected by Clang on some versions
      if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
         add_compile_options(-fno-semantic-interposition)
      endif()
      if(NOT DISABLE_LTO)
         add_compile_options(-flto=auto) # Link-time optimization with parallelization
      endif()
      add_link_options(
         -Wl,--gc-sections               # Strip unused functions/data
      )
      if(NOT DISABLE_LTO)
         add_link_options(-flto=auto)    # Enable LTO at link time
      endif()

      if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
         add_compile_options(-march=x86-64-v2)
      elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
         add_compile_options(-march=armv8-a)
         add_link_options(-latomic)
      elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
         # Pin hardfloat ABI explicitly so a soft-float toolchain can't silently
         # produce an ABI-mismatched binary that runs but corrupts FP registers.
         add_compile_options(-march=armv7-a -mfpu=neon -mfloat-abi=hard)
         add_link_options(-latomic)
      endif()
   endif()

   # Effective LTO state (after Flatpak/sandboxed override)
   if(DISABLE_LTO)
      message(STATUS "LTO: DISABLED (sandboxed build)")
   else()
      message(STATUS "LTO: ENABLED")
   endif()
else()
   message("Disabling production optimization flags...")
endif()

#---------------------------------------------------------------------------------------------------
# Platform-specific SIMD instructions
#---------------------------------------------------------------------------------------------------
#
# Enables SIMD (Single Instruction Multiple Data) optimizations for x86-64:
#
# SSE4.1 support:
#   - Enabled on x86-64 processors (Intel/AMD 64-bit)
#   - Provides vectorized operations for DSP, graphics, data processing
#   - Required by some Qt 6 modules and KissFFT optimizations
#
# Why SSE4.1?
#   - Widely available (Intel since Core 2, AMD since Bulldozer)
#   - Safe baseline for modern systems (2008+ CPUs)
#   - Significantly faster than SSE2 for floating-point operations
#
# Platform notes:
#   - MSVC x64: SSE2 is always enabled (x64 ABI requirement), no flag needed
#   - GCC/Clang: Must explicitly enable SSE4.1 with -msse4.1
#   - ARM: SIMD handled separately via NEON (see PRODUCTION_OPTIMIZATION)
#
# Impact:
#   - ~20-30% performance boost for FFT calculations
#   - ~10-15% improvement in signal processing pipelines
#
#---------------------------------------------------------------------------------------------------

if(NOT MSVC AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
  message(STATUS "Enabling SSE4.1 optimizations")
  add_compile_options(-msse4.1)
endif()

#---------------------------------------------------------------------------------------------------
# Profile-Guided Optimization (PGO)
#---------------------------------------------------------------------------------------------------
#
# PGO is a two-stage optimization process:
#
# Stage 1 - GENERATE (build with instrumentation):
#   cmake .. -DENABLE_PGO=ON -DPGO_STAGE=GENERATE -DCMAKE_BUILD_TYPE=Release
#   cmake --build . -j$(nproc)
#
# Stage 2 - PROFILE (run the application with typical workloads):
#   ./Serial-Studio-GPL3  # Use the app normally to generate profile data
#   # Profile data (.gcda files for GCC, .profraw for Clang) is written to build dir
#
# Stage 3 - USE (rebuild with profile data):
#   cmake .. -DENABLE_PGO=ON -DPGO_STAGE=USE -DCMAKE_BUILD_TYPE=Release
#   cmake --build . -j$(nproc)
#
# Expected performance gain: 10-20% for typical workloads
#---------------------------------------------------------------------------------------------------

set(PGO_STAGE "GENERATE" CACHE STRING "PGO stage: GENERATE or USE")
set(PGO_PROFILE_DIR "${CMAKE_BINARY_DIR}/pgo-profiles" CACHE STRING "Directory for PGO profile data")

if(ENABLE_PGO)
   if(NOT PRODUCTION_OPTIMIZATION)
      message(WARNING "PGO requires PRODUCTION_OPTIMIZATION=ON for best results")
   endif()

   # Create profile directory if it doesn't exist
   file(MAKE_DIRECTORY ${PGO_PROFILE_DIR})

   if(PGO_STAGE STREQUAL "GENERATE")
      message(STATUS "PGO: GENERATE stage - building with profile instrumentation")
      message(STATUS "PGO: Profile data will be written to: ${PGO_PROFILE_DIR}")
      message(STATUS "PGO: After running the app, rebuild with -DPGO_STAGE=USE")

      if(MSVC AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
         add_compile_options(/GL)                           # Whole program optimization
         add_link_options(
            /LTCG                                           # Link-time code generation
            /GENPROFILE                                     # Generate profile data
            /PGD:${PGO_PROFILE_DIR}/profile.pgd             # Profile database location
         )
      elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
         add_compile_options(
            -fprofile-generate=${PGO_PROFILE_DIR}          # Generate profile data (Clang)
         )
         add_link_options(
            -fprofile-generate=${PGO_PROFILE_DIR}          # Link with profiling
         )
      elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
         add_compile_options(
            -fprofile-generate                             # Generate profile data (GCC)
            -fprofile-dir=${PGO_PROFILE_DIR}               # Profile data directory
         )
         add_link_options(
            -fprofile-generate                             # Link with profiling
            -fprofile-dir=${PGO_PROFILE_DIR}               # Profile data directory
         )
      else()
         message(FATAL_ERROR "PGO not supported for compiler: ${CMAKE_CXX_COMPILER_ID}")
      endif()

   elseif(PGO_STAGE STREQUAL "USE")
      message(STATUS "PGO: USE stage - building with profile optimization")
      message(STATUS "PGO: Using profile data from: ${PGO_PROFILE_DIR}")

      # Check if profile data exists
      if(MSVC AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
         if(NOT EXISTS "${PGO_PROFILE_DIR}/profile.pgd")
            message(FATAL_ERROR "PGO profile data not found at ${PGO_PROFILE_DIR}/profile.pgd\nRun with -DPGO_STAGE=GENERATE first")
         endif()
      elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
         # Clang stores .profraw files that need to be merged into .profdata
         file(GLOB PROFRAW_FILES "${PGO_PROFILE_DIR}/*.profraw")
         if(NOT PROFRAW_FILES)
            message(FATAL_ERROR "No .profraw files found in ${PGO_PROFILE_DIR}\nRun the instrumented binary first")
         endif()

         # Merge profile data
         find_program(LLVM_PROFDATA llvm-profdata)
         if(NOT LLVM_PROFDATA)
            message(FATAL_ERROR "llvm-profdata not found. Install LLVM tools to use PGO with Clang")
         endif()

         set(PROFDATA_FILE "${PGO_PROFILE_DIR}/merged.profdata")
         if(NOT EXISTS ${PROFDATA_FILE})
            message(STATUS "Merging profile data: ${PROFRAW_FILES}")
            execute_process(
               COMMAND ${LLVM_PROFDATA} merge -output=${PROFDATA_FILE} ${PROFRAW_FILES}
               RESULT_VARIABLE MERGE_RESULT
            )
            if(NOT MERGE_RESULT EQUAL 0)
               message(FATAL_ERROR "Failed to merge profile data")
            endif()
         endif()
      elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
         file(GLOB GCDA_FILES "${PGO_PROFILE_DIR}/*.gcda")
         if(NOT GCDA_FILES)
            message(FATAL_ERROR "No .gcda files found in ${PGO_PROFILE_DIR}\nRun the instrumented binary first")
         endif()
      endif()

      if(MSVC AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
         add_compile_options(/GL)                           # Whole program optimization
         add_link_options(
            /LTCG                                           # Link-time code generation
            /USEPROFILE:PGD=${PGO_PROFILE_DIR}/profile.pgd  # Use profile data
         )
      elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
         add_compile_options(
            -fprofile-use=${PROFDATA_FILE}                 # Use merged profile data
         )
         add_link_options(
            -fprofile-use=${PROFDATA_FILE}                 # Link with profile optimization
         )
      elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
         add_compile_options(
            -fprofile-use                                  # Use profile data (GCC)
            -fprofile-dir=${PGO_PROFILE_DIR}               # Profile data directory
            -fprofile-correction                           # Handle profile inconsistencies
         )
         add_link_options(
            -fprofile-use                                  # Link with profile optimization
            -fprofile-dir=${PGO_PROFILE_DIR}               # Profile data directory
         )
      endif()

   else()
      message(FATAL_ERROR "Invalid PGO_STAGE: ${PGO_STAGE}. Must be GENERATE or USE")
   endif()
endif()
