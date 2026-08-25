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
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial

include_guard(GLOBAL)

#---------------------------------------------------------------------------------------------------
# Production optimization flags
#---------------------------------------------------------------------------------------------------
#
# PRODUCTION_OPTIMIZATION=ON applies aggressive, per-toolchain release flags. Exactly one branch
# runs per build:
#
#   clang-cl (Windows, MSVC ABI)  /O2 /Oi /Ot /Gy /Gw /clang:-march=x86-64-v2 /fp:precise /DNDEBUG;
#                                 link /OPT:REF /OPT:ICF. Unless LTO is disabled, enables ThinLTO by
#                                 passing /clang:-flto=thin on the compile step only: that makes clang-cl
#                                 emit LLVM bitcode .obj files, and lld-link (which CMake already invokes
#                                 directly to link this target) auto-detects bitcode and runs the ThinLTO
#                                 backend with no extra link flag. The only link-side LTO flag is the
#                                 native lld-link /lldltocache:<dir> incremental cache. Driver-style link
#                                 flags (-fuse-ld=lld, -flto=thin) are NOT passed here: CMake links via
#                                 lld-link directly, not through the clang-cl driver, so lld-link would
#                                 reject /clang:-flto=thin as a bogus input file. /Gy /Gw give
#                                 per-function/per-data sections so /OPT:REF and ThinLTO can dead-strip.
#                                 -march=x86-64-v2 (SSE4.2) matches the conservative baseline of the
#                                 other x86-64 branches. /fp:precise is kept (no fast-math) so telemetry
#                                 output is bit-stable. No /GL or /LTCG (those are cl.exe only). clang-cl
#                                 reports MSVC=ON, so this branch precedes cl.exe.
#   MSVC cl.exe (Windows)         /permissive- /Zc:* /MP /O2 /Ot /Oi /Ob3 /fp:precise /Gw /Gy, plus
#                                 /GL and /LTCG whole-program codegen unless LTO is disabled.
#   GCC/Clang (Linux), AppleClang (macOS), Clang/GCC MinGW, IntelLLVM
#                                 -O3 -funroll-loops -fomit-frame-pointer, IEEE math (-fno-fast-math,
#                                 -fno-unsafe-math-optimizations), -ffunction-sections/-fdata-sections
#                                 paired with --gc-sections (-dead_strip on macOS), and -flto=auto
#                                 unless disabled. -fno-semantic-interposition on GCC/IntelLLVM only.
#                                 macOS is the exception: it keeps the frame pointer (the Apple arm64
#                                 ABI walks x29 chains).
#
# Both Windows branches first strip CMake's injected /Ob2 (the *_RELEASE* flags) and /W3 (base
# flags) so our inlining and per-target /W4 win without a flood of D9025 "overriding" diagnostics.
#
# Non-MSVC builds force -fexceptions + -funwind-tables + -fasynchronous-unwind-tables on every TU:
# the LuaJIT runtime (spec 0051; replaced the Lua-5.4-as-C++ design) delivers Lua errors through
# external frame unwinding on the unwind-external targets and interoperates with C++ exceptions
# there, so every linked object must still carry unwind metadata for the error path to walk out
# of the VM into the host's pcall/catch guards. On macOS, Xcode 26's ld (ld-1267) drops DWARF
# (__eh_frame) exception unwind under -flto for functions that fall back from compact unwind
# (llvm/llvm-project#135888, open upstream). With pac-ret gated off Apple (Hardening.cmake) the app
# rides compact unwind, so that fallback population is tiny; the luajit target additionally opts
# out of LTO entirely (lib/luajit/CMakeLists.txt: assembly VM + generated tables). Remaining
# guards: per-TU unwind tables here, protected bootstraps around every VM setup path, and pcall
# around every script entry.
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
   add_compile_definitions(SS_OPT_PRODUCTION)

   # Gate the optimization/LTO flags below to optimized configs. On a multi-config generator
   # (Visual Studio) CMAKE_BUILD_TYPE is ignored and a bare `cmake --build` builds Debug; without
   # this gate PRODUCTION_OPTIMIZATION would inject /O2 into the Debug config, which already carries
   # MSVC's /RTC1, and that combination is rejected by the compiler and aborts the build. Only the
   # MSVC branches consume it; other toolchains tolerate -O3 alongside -g without a hard error.
   set(SS_OPT_CONFIGS "$<CONFIG:Release,RelWithDebInfo,MinSizeRel>")

   if(NOT MSVC)
      add_compile_options(
         -fexceptions
         -funwind-tables
         -fasynchronous-unwind-tables
      )
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

      # /clang:-O3 follows /O2 (last -O wins in clang) so this branch matches the -O3 used by
      # every other production toolchain instead of clang-cl's /O2 -> -O2 mapping. The /O and LTO
      # flags are gated to optimized configs (SS_OPT_CONFIGS) so they never collide with Debug's
      # /RTC1 on a multi-config generator.
      #
      # /Z7 embeds CodeView in the objects and lld-link's /DEBUG folds it into an external .pdb
      # so CI can symbolize crash minidumps; codegen is untouched and cpack never installs the
      # .pdb, so shipped packages are unchanged.
      add_compile_options(
         $<${SS_OPT_CONFIGS}:/O2>
         $<${SS_OPT_CONFIGS}:/Oi>
         $<${SS_OPT_CONFIGS}:/Ot>
         $<${SS_OPT_CONFIGS}:/clang:-O3>
         /Gy
         /Gw
         /Z7
         /clang:-march=x86-64-v2
         /fp:precise
         /DNDEBUG
      )
      add_link_options(
         /DEBUG
         /OPT:REF
         /OPT:ICF
      )

      if(NOT DISABLE_LTO)
         # -fwhole-program-vtables devirtualizes app-internal interfaces (hidden LTO visibility
         # on COFF); dllimported Qt classes keep public visibility, so plugins are unaffected.
         add_compile_options(
            $<${SS_OPT_CONFIGS}:/clang:-flto=thin>
            $<${SS_OPT_CONFIGS}:/clang:-fwhole-program-vtables>
         )
         add_link_options($<${SS_OPT_CONFIGS}:/lldltocache:${CMAKE_BINARY_DIR}/lto.cache>)
      endif()

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

      # The /O and whole-program (/GL, /LTCG) flags are gated to optimized configs (SS_OPT_CONFIGS)
      # so they never collide with Debug's /RTC1 on a multi-config generator; the conformance,
      # /MP, /fp:precise and section flags stay unconditional.
      add_compile_options(
         /permissive-
         /Zc:__cplusplus
         /Zc:preprocessor
         /MP
         $<${SS_OPT_CONFIGS}:/O2>
         $<${SS_OPT_CONFIGS}:/Ot>
         $<${SS_OPT_CONFIGS}:/Oi>
         $<${SS_OPT_CONFIGS}:/Ob3>
         /fp:precise
         /Gw
         /Gy
         /DNDEBUG
      )
      if(NOT DISABLE_LTO)
         add_compile_options($<${SS_OPT_CONFIGS}:/GL>)
      endif()
      add_link_options(
         /OPT:REF
         /OPT:ICF
      )
      if(NOT DISABLE_LTO)
         add_link_options($<${SS_OPT_CONFIGS}:/LTCG>)
      endif()

   elseif(APPLE)
      message(STATUS "Production branch: AppleClang (macOS)")

      # LTO is on: Xcode's ld drops DWARF exception unwind under -flto for compact-unwind
      # fallback functions (llvm/llvm-project#135888), but the unwind-across-LTO hazard is
      # confined to the Lua error path, and the luajit target opts out of LTO entirely
      # (lib/luajit/CMakeLists.txt). The 2026-07 CI hang that once implicated LTO was
      # root-caused to an API::Server socket ABA race, so the blanket disable is gone.
      # Frame pointers stay on: the Apple arm64 ABI walks x29 chains — but only
      # non-leaf frames must keep them, so -momit-leaf-frame-pointer frees x29 in leaf loops
      # (tokenizer/DSP). Hidden visibility stays on for tighter symbol binding.
      add_compile_options(
         -O3
         -funroll-loops
         -fno-omit-frame-pointer
         -momit-leaf-frame-pointer
         -fvisibility=hidden
         -fvisibility-inlines-hidden
         -fno-fast-math
         -fno-unsafe-math-optimizations
         -ffunction-sections
         -fdata-sections
      )
      if(NOT DISABLE_LTO)
         add_compile_options(
            -flto=auto
            -fwhole-program-vtables
         )
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
      # Clang parity with the clang-cl/AppleClang branches: hidden visibility plus
      # -fwhole-program-vtables devirtualizes app-internal interfaces under LTO (safe: single
      # monolithic executable). GCC has no equivalent flag, so its branch is unchanged.
      if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
         add_compile_options(
            -fvisibility=hidden
            -fvisibility-inlines-hidden
         )
         if(NOT DISABLE_LTO)
            add_compile_options(-fwhole-program-vtables)
         endif()
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
      message(STATUS "LTO: DISABLED")
   else()
      add_compile_definitions(SS_OPT_LTO)
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
#   Clang (Linux) / AppleClang  -fprofile-generate -> *.profraw, merged by llvm-profdata into
#                               merged.profdata, consumed with -fprofile-use. These link through the
#                               clang driver, so the -fprofile-* flags go on BOTH compile and link
#                               (the driver links clang_rt.profile and feeds the profile into LTO).
#   clang-cl (Windows)          Same *.profraw -> merged.profdata flow, but the flag is the clang-cl
#                               passthrough /clang:-fprofile-generate= / /clang:-fprofile-use= and it
#                               goes on the COMPILE step ONLY. CMake links via lld-link directly, not
#                               the clang-cl driver, so a -fprofile-* link flag would be rejected as an
#                               unknown argument; the instrumented objects embed a /INCLUDE: directive
#                               that pulls in clang_rt.profile automatically, and -fprofile-use shapes
#                               codegen entirely at compile time, so the link step needs no PGO flag.
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

      # Counter updates are atomic on every toolchain (-fprofile-update=atomic /
      # /GENPROFILE:EXACT): the training workload is heavily multithreaded (CSV/MDF4/Sessions/
      # API/gRPC export workers next to the producer), and the default racy increments tear
      # counts, making worker-side hot code look cold to the USE stage.
      if(MSVC AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
         add_compile_options(/GL)
         add_link_options(
            /LTCG
            /GENPROFILE:EXACT
            /PGD:${PGO_PROFILE_DIR}/profile.pgd
         )
      elseif(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
         add_compile_options(
            /clang:-fprofile-generate=${PGO_PROFILE_DIR}
            /clang:-fprofile-update=atomic
         )
      elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
         add_compile_options(
            -fprofile-generate=${PGO_PROFILE_DIR}
            -fprofile-update=atomic
         )
         add_link_options(
            -fprofile-generate=${PGO_PROFILE_DIR}
         )
      elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
         add_compile_options(
            -fprofile-generate
            -fprofile-dir=${PGO_PROFILE_DIR}
            -fprofile-update=atomic
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

      add_compile_definitions(SS_PGO_USE)

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

         # Always re-merge: a stale merged.profdata from an earlier configure would otherwise
         # silently win over freshly written .profraw files in an incremental build dir.
         set(PROFDATA_FILE "${PGO_PROFILE_DIR}/merged.profdata")
         message(STATUS "Merging profile data: ${PROFRAW_FILES}")
         execute_process(
            COMMAND ${LLVM_PROFDATA} merge -output=${PROFDATA_FILE} ${PROFRAW_FILES}
            RESULT_VARIABLE MERGE_RESULT
         )
         if(NOT MERGE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to merge profile data")
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
      elseif(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
         # GENERATE/USE drift shows up as -Wbackend-plugin "function control flow change
         # detected (hash mismatch)" warnings, one per function, with the discarded count.
         # -Wprofile-instr-out-of-date never fires here: it belongs to frontend
         # instrumentation (-fprofile-instr-use), not the IR PGO (-fprofile-use) used below.
         # Not promoted to -Werror: COMDAT template copies (QList<T>::reserve) legitimately
         # hash differently across unity TUs and report "0 count discarded".
         add_compile_options(
            /clang:-fprofile-use=${PROFDATA_FILE}
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

#---------------------------------------------------------------------------------------------------
# Build-time host tools: opt out of PGO/LTO
#---------------------------------------------------------------------------------------------------
#
# PGO and LTO flags are added at directory scope, so every target in the tree inherits them --
# including code generators that are built AND executed during the build (LuaJIT's minilua and
# buildvm). Instrumenting those is pure loss:
#
#   - They run at build time, so an instrumented minilua floods the build log with
#     "LLVM Profile Warning: Unable to track new values: Running out of static counters"
#     (a bytecode interpreter blows past the per-site value-profile budget).
#   - Each run drops .profraw files into PGO_PROFILE_DIR that have nothing to do with the
#     application workload the USE stage is supposed to be shaped by.
#
# CMake initializes a target's COMPILE_OPTIONS / LINK_OPTIONS from the directory properties at
# target-creation time, so the flags can be filtered back out per target afterwards.
#
#---------------------------------------------------------------------------------------------------

function(ss_exclude_from_pgo)
  foreach(target ${ARGN})
    if(NOT TARGET ${target})
      message(WARNING "ss_exclude_from_pgo: no such target: ${target}")
      continue()
    endif()

    foreach(property COMPILE_OPTIONS LINK_OPTIONS)
      get_target_property(options ${target} ${property})
      if(options)
        list(FILTER options EXCLUDE REGEX "profile|GENPROFILE|USEPROFILE|PGD:|^/GL$|^/LTCG$")
        set_target_properties(${target} PROPERTIES ${property} "${options}")
      endif()
    endforeach()

    set_target_properties(${target} PROPERTIES INTERPROCEDURAL_OPTIMIZATION FALSE)
  endforeach()
endfunction()
