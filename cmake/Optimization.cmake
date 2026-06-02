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

include_guard(GLOBAL)

#---------------------------------------------------------------------------------------------------
# Production optimization flags
#---------------------------------------------------------------------------------------------------
#
# PRODUCTION_OPTIMIZATION=ON applies aggressive, per-toolchain release flags. Exactly one branch
# runs per build:
#
#   clang-cl (Windows, MSVC ABI)  /O2 /Oi /Ot /fp:precise /DNDEBUG; link /OPT:REF /OPT:ICF. Preferred
#                                 over cl.exe for faster scalar parse-hotpath codegen while still
#                                 linking the prebuilt MSVC Qt. No /GL or /LTCG (those are cl.exe
#                                 only). clang-cl reports MSVC=ON, so this branch precedes cl.exe.
#   MSVC cl.exe (Windows)         /permissive- /Zc:* /MP /O2 /Ot /Oi /Ob3 /fp:precise /Gw /Gy, plus
#                                 /GL and /LTCG whole-program codegen unless LTO is disabled.
#   GCC/Clang (Linux), AppleClang (macOS), Clang/GCC MinGW, IntelLLVM
#                                 -O3 -funroll-loops -fomit-frame-pointer, IEEE math (-fno-fast-math,
#                                 -fno-unsafe-math-optimizations), -ffunction-sections/-fdata-sections
#                                 paired with --gc-sections (-dead_strip on macOS), and -flto=auto
#                                 unless disabled. -fno-semantic-interposition on GCC/IntelLLVM only.
#
# Both Windows branches first strip CMake's injected /Ob2 (the *_RELEASE* flags) and /W3 (base
# flags) so our inlining and per-target /W4 win without a flood of D9025 "overriding" diagnostics.
#
# Non-MSVC builds force -fexceptions + -funwind-tables + -fasynchronous-unwind-tables on every TU:
# Lua is compiled as C++ and throws across the VM stack on any runtime error, so every linked object
# must carry unwind metadata, or --gc-sections/-dead_strip + LTO can drop a personality routine and
# turn a routine Lua type error into a std::terminate. macOS also keeps -Wl,-keep_dwarf_unwind.
#
# Architecture baselines: x86-64 -> -march=x86-64-v2 (SSE4.2, 2012+ CPUs); aarch64 -> armv8-a (plus
# -latomic); armv7l -> armv7-a -mfpu=neon -mfloat-abi=hard (hardfloat pinned so a soft-float
# toolchain cannot silently emit an ABI-mismatched binary).
#
# Sandboxed builds (Flatpak/Flathub, detected via FLATPAK_ID) disable LTO and auto-enable hardening.
#
#---------------------------------------------------------------------------------------------------

if(PRODUCTION_OPTIMIZATION)
   if(DEFINED ENV{FLATPAK_ID} OR FLATPAK_BUILD)
      set(DISABLE_LTO ON)
      set(ENABLE_HARDENING ON CACHE BOOL "Auto-enabled hardening for sandboxed builds" FORCE)
      message(STATUS "Sandboxed build detected, disabling LTO and enabling hardening")
   else()
      set(DISABLE_LTO OFF)
   endif()

   message("Enabling production optimization flags...")

   add_compile_definitions(NDEBUG)

   if(NOT MSVC)
      add_compile_options(
         -fexceptions
         -funwind-tables
         -fasynchronous-unwind-tables
      )
      if(APPLE)
         add_link_options(-Wl,-keep_dwarf_unwind)
      endif()
   endif()

   if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND MINGW)
      message(STATUS "Production branch: Clang/MinGW (Windows)")
      add_compile_options(
         -O3
         -march=x86-64-v2
         -funroll-loops
         -fomit-frame-pointer
         -fno-fast-math
         -fno-unsafe-math-optimizations
         -ffunction-sections
         -fdata-sections
      )
      if(NOT DISABLE_LTO)
         add_compile_options(-flto=auto)
      endif()
      add_link_options(
         -Wl,--gc-sections
      )
      if(NOT DISABLE_LTO)
         add_link_options(-flto=auto)
      endif()

   elseif(WIN32 AND MINGW)
      message(STATUS "Production branch: GCC/MinGW (Windows)")
      add_compile_options(
         -O3
         -march=x86-64-v2
         -funroll-loops
         -fomit-frame-pointer
         -fno-fast-math
         -fno-unsafe-math-optimizations
         -fno-semantic-interposition
         -ffunction-sections
         -fdata-sections
      )
      if(NOT DISABLE_LTO)
         add_compile_options(-flto=auto)
      endif()
      add_link_options(
         -Wl,--gc-sections
      )
      if(NOT DISABLE_LTO)
         add_link_options(-flto=auto)
      endif()

   elseif(WIN32 AND MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      message(STATUS "Production branch: clang-cl (Windows, MSVC ABI)")

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
         /O2
         /Oi
         /Ot
         /fp:precise
         /DNDEBUG
      )
      add_link_options(
         /OPT:REF
         /OPT:ICF
      )

   elseif(WIN32 AND MSVC)
      message(STATUS "Production branch: MSVC (Windows)")

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
         /permissive-
         /Zc:__cplusplus
         /Zc:preprocessor
         /MP
         /O2
         /Ot
         /Oi
         /Ob3
         /fp:precise
         /Gw
         /Gy
         /DNDEBUG
      )
      if(NOT DISABLE_LTO)
         add_compile_options(/GL)
      endif()
      add_link_options(
         /OPT:REF
         /OPT:ICF
      )
      if(NOT DISABLE_LTO)
         add_link_options(/LTCG)
      endif()

   elseif(APPLE)
      message(STATUS "Production branch: AppleClang (macOS)")
      add_compile_options(
         -O3
         -funroll-loops
         -fomit-frame-pointer
         -fno-fast-math
         -fno-unsafe-math-optimizations
         -ffunction-sections
         -fdata-sections
      )
      if(NOT DISABLE_LTO)
         add_compile_options(-flto=auto)
      endif()
      add_link_options(
         -Wl,-dead_strip
      )
      if(NOT DISABLE_LTO)
         add_link_options(-flto=auto)
      endif()

      if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
         add_compile_options(-march=x86-64-v2)
      endif()

   elseif(CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
      message(STATUS "Production branch: IntelLLVM")
      add_compile_options(
         -O3
         -march=x86-64-v2
         -static
         -funroll-loops
         -fomit-frame-pointer
         -fno-fast-math
         -fno-unsafe-math-optimizations
         -fno-semantic-interposition
         -ffunction-sections
         -fdata-sections
      )
      if(NOT DISABLE_LTO)
         add_compile_options(-flto=auto)
      endif()
      add_link_options(
         -Wl,--gc-sections
      )
      if(NOT DISABLE_LTO)
         add_link_options(-flto=auto)
      endif()

   elseif(UNIX)
      message(STATUS "Production branch: ${CMAKE_CXX_COMPILER_ID} (Linux/${CMAKE_SYSTEM_PROCESSOR})")
      add_compile_options(
         -O3
         -funroll-loops
         -fomit-frame-pointer
         -fno-fast-math
         -fno-unsafe-math-optimizations
         -ffunction-sections
         -fdata-sections
      )
      if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
         add_compile_options(-fno-semantic-interposition)
      endif()
      if(NOT DISABLE_LTO)
         add_compile_options(-flto=auto)
      endif()
      add_link_options(
         -Wl,--gc-sections
      )
      if(NOT DISABLE_LTO)
         add_link_options(-flto=auto)
      endif()

      if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
         add_compile_options(-march=x86-64-v2)
      elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
         add_compile_options(-march=armv8-a)
         add_link_options(-latomic)
      elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
         add_compile_options(-march=armv7-a -mfpu=neon -mfloat-abi=hard)
         add_link_options(-latomic)
      endif()
   endif()

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
# On non-MSVC x86-64 toolchains (GCC/Clang/AppleClang/IntelLLVM) an explicit -msse4.1 is added for
# vectorized DSP/FFT work. MSVC and clang-cl are skipped: the x64 ABI already guarantees an SSE2
# baseline and neither is handed an explicit SSE4.1 flag here. ARM SIMD (NEON) comes from the
# -march flags in the production-optimization section above.
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
# Two stages, selected by PGO_STAGE (GENERATE | USE); profile data lives in PGO_PROFILE_DIR
# (default ${CMAKE_BINARY_DIR}/pgo-profiles):
#
#   GENERATE  Build instrumented, then run the binary on a representative workload (in CI the
#             --headless --benchmark-hotpath run) to emit profile data.
#   USE       Rebuild consuming that data.
#
# Instrumentation and data format are per compiler:
#   MSVC cl.exe                 /GL + /LTCG /GENPROFILE -> profile.pgd, consumed with /USEPROFILE.
#   Clang / clang-cl / AppleClang  -fprofile-generate -> *.profraw, merged by llvm-profdata into
#                               merged.profdata, consumed with -fprofile-use.
#   GCC                         -fprofile-generate -> *.gcda, consumed with -fprofile-use plus
#                               -fprofile-correction.
#
# Requires PRODUCTION_OPTIMIZATION=ON for meaningful results.
#
#---------------------------------------------------------------------------------------------------

set(PGO_STAGE "GENERATE" CACHE STRING "PGO stage: GENERATE or USE")
set(PGO_PROFILE_DIR "${CMAKE_BINARY_DIR}/pgo-profiles" CACHE STRING "Directory for PGO profile data")

if(ENABLE_PGO)
   if(NOT PRODUCTION_OPTIMIZATION)
      message(WARNING "PGO requires PRODUCTION_OPTIMIZATION=ON for best results")
   endif()

   file(MAKE_DIRECTORY ${PGO_PROFILE_DIR})

   if(PGO_STAGE STREQUAL "GENERATE")
      message(STATUS "PGO: GENERATE stage - building with profile instrumentation")
      message(STATUS "PGO: Profile data will be written to: ${PGO_PROFILE_DIR}")
      message(STATUS "PGO: After running the app, rebuild with -DPGO_STAGE=USE")

      add_compile_definitions(SS_PGO_INSTRUMENT)

      if(MSVC AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
         add_compile_options(/GL)
         add_link_options(
            /LTCG
            /GENPROFILE
            /PGD:${PGO_PROFILE_DIR}/profile.pgd
         )
      elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
         add_compile_options(
            -fprofile-generate=${PGO_PROFILE_DIR}
         )
         add_link_options(
            -fprofile-generate=${PGO_PROFILE_DIR}
         )
      elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
         add_compile_options(
            -fprofile-generate
            -fprofile-dir=${PGO_PROFILE_DIR}
         )
         add_link_options(
            -fprofile-generate
            -fprofile-dir=${PGO_PROFILE_DIR}
         )
      else()
         message(FATAL_ERROR "PGO not supported for compiler: ${CMAKE_CXX_COMPILER_ID}")
      endif()

   elseif(PGO_STAGE STREQUAL "USE")
      message(STATUS "PGO: USE stage - building with profile optimization")
      message(STATUS "PGO: Using profile data from: ${PGO_PROFILE_DIR}")

      if(MSVC AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
         if(NOT EXISTS "${PGO_PROFILE_DIR}/profile.pgd")
            message(FATAL_ERROR "PGO profile data not found at ${PGO_PROFILE_DIR}/profile.pgd\nRun with -DPGO_STAGE=GENERATE first")
         endif()
      elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
         file(GLOB PROFRAW_FILES "${PGO_PROFILE_DIR}/*.profraw")
         if(NOT PROFRAW_FILES)
            message(FATAL_ERROR "No .profraw files found in ${PGO_PROFILE_DIR}\nRun the instrumented binary first")
         endif()

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
         add_compile_options(/GL)
         add_link_options(
            /LTCG
            /USEPROFILE:PGD=${PGO_PROFILE_DIR}/profile.pgd
         )
      elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
         add_compile_options(
            -fprofile-use=${PROFDATA_FILE}
         )
         add_link_options(
            -fprofile-use=${PROFDATA_FILE}
         )
      elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
         add_compile_options(
            -fprofile-use
            -fprofile-dir=${PGO_PROFILE_DIR}
            -fprofile-correction
         )
         add_link_options(
            -fprofile-use
            -fprofile-dir=${PGO_PROFILE_DIR}
         )
      endif()

   else()
      message(FATAL_ERROR "Invalid PGO_STAGE: ${PGO_STAGE}. Must be GENERATE or USE")
   endif()
endif()
