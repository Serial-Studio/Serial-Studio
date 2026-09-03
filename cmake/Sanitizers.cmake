#
# Serial Studio — Sanitizer builds (ASan/UBSan, ThreadSanitizer)
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

option(ENABLE_TSAN "ThreadSanitizer build (mutually exclusive with DEBUG_SANITIZER)" OFF)
option(ENABLE_FUZZERS "Build the libFuzzer entry points under app/tests/fuzz (Clang only)" OFF)

#---------------------------------------------------------------------------------------------------
# Debug sanitizers (AddressSanitizer + UndefinedBehaviorSanitizer)
#---------------------------------------------------------------------------------------------------
#
# Runtime error detection for debug builds:
#
# When DEBUG_SANITIZER=ON is enabled (debug builds only):
#
# AddressSanitizer (ASan):
#   - Detects memory errors: use-after-free, buffer overflows, leaks
#   - Adds instrumentation to memory operations (~2x slowdown)
#   - Requires ~2-3x more memory than normal execution
#
# UndefinedBehaviorSanitizer (UBSan):
#   - Detects undefined behavior: null pointer dereference, integer overflow
#   - Minimal performance impact (~10-20% slowdown)
#
# Additional flags:
#   -g                      - Debug symbols for stack traces
#   -fno-omit-frame-pointer - Preserve frame pointers for accurate traces
#
# Usage:
#   cmake .. -DDEBUG_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
#   ./Serial-Studio-GPL3
#   # If errors are found, ASan/UBSan will print detailed reports to stderr
#
# Note: Only supported on GCC/Clang. Not compatible with MSVC.
# Not recommended for production builds due to performance overhead.
#
#---------------------------------------------------------------------------------------------------

if(DEBUG_SANITIZER)
   add_compile_options(
      -fsanitize=address
      -fsanitize=undefined
      -g
      -fno-omit-frame-pointer
   )
   add_link_options(
      -fsanitize=address
      -fsanitize=undefined
   )
endif()

#---------------------------------------------------------------------------------------------------
# ThreadSanitizer (separate build)
#---------------------------------------------------------------------------------------------------
#
# The hotpath is intentionally lock-free (driver thread -> main via SPSC CircularBuffer) and uses
# Qt::DirectConnection across threads in places. TSan is the only tool that proves those invariants
# hold under load. It cannot coexist with ASan, so it is its own build.
#
# Usage:
#   cmake .. -DENABLE_TSAN=ON -DCMAKE_BUILD_TYPE=Debug   # GCC/Clang only
#
#---------------------------------------------------------------------------------------------------

if(ENABLE_TSAN)
   if(DEBUG_SANITIZER)
      message(FATAL_ERROR "ENABLE_TSAN and DEBUG_SANITIZER (ASan) are mutually exclusive. Pick one build.")
   endif()
   if(MSVC)
      message(FATAL_ERROR "ThreadSanitizer is not supported under MSVC. Use a Clang/GCC build.")
   endif()
   add_compile_options(
      -fsanitize=thread
      -g
      -fno-omit-frame-pointer
      -O1
   )
   add_link_options(
      -fsanitize=thread
   )
   message(STATUS "ThreadSanitizer ENABLED")
endif()

#---------------------------------------------------------------------------------------------------
# libFuzzer instrumentation
#---------------------------------------------------------------------------------------------------
#
# Coverage instrumentation for the fuzz entry points under app/tests/fuzz. -no-link is the
# whole-project half: every translation unit is instrumented, and only the fuzz targets
# themselves add -fsanitize=fuzzer at link time to pull in the driver's main().
#
# It composes with DEBUG_SANITIZER (ASan+UBSan) and that is how the sanitize CI job runs it. When
# this option is OFF -- the default everywhere, including every ctest tier -- ss_add_fuzz_target()
# still compiles the same entry points and replays the checked-in corpus through them under
# QTest, so the seeds keep running on toolchains that have no libFuzzer.
#
# Usage:
#   cmake .. -DENABLE_FUZZERS=ON -DDEBUG_SANITIZER=ON -DSS_BUILD_TESTS=ON \
#            -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug
#
#---------------------------------------------------------------------------------------------------

if(ENABLE_FUZZERS)
   if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      message(FATAL_ERROR
         "ENABLE_FUZZERS requires Clang (libFuzzer ships with it). "
         "Configure with -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++.")
   endif()
   add_compile_options(
      -fsanitize=fuzzer-no-link
      -g
      -fno-omit-frame-pointer
   )
   message(STATUS "libFuzzer instrumentation ENABLED (-fsanitize=fuzzer-no-link)")
endif()
