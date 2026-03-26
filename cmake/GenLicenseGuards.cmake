#
# Serial Studio — Build-time license guard generator
#
# Generates LicenseGuards.generated.h with N unique guard functions.
# Each function validates a different mathematical transform of the
# build salt against a compile-time expected value. The function names,
# check logic, and constants change on every build, so no static patch
# can survive across releases.
#
# Usage: include() after COMMERCIAL_BUILD_SALT is set.
#
# SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
#

if(NOT BUILD_COMMERCIAL OR NOT COMMERCIAL_BUILD_SALT)
  return()
endif()

set(_LG_OUTPUT "${CMAKE_BINARY_DIR}/generated/LicenseGuards.generated.h")

#---------------------------------------------------------------------------------------------------
# Helper: pseudo-random number from salt + seed
#---------------------------------------------------------------------------------------------------

macro(_lg_hash_seed INPUT OUTPUT)
  string(MD5 _md5 "${COMMERCIAL_BUILD_SALT}_${INPUT}")
  # Extract 8 hex chars and convert to decimal
  string(SUBSTRING "${_md5}" 0 8 _hex8)
  math(EXPR ${OUTPUT} "0x${_hex8}" OUTPUT_FORMAT DECIMAL)
endmacro()

#---------------------------------------------------------------------------------------------------
# Generate guard function names (randomized per build)
#---------------------------------------------------------------------------------------------------

set(_LG_COUNT 24)
set(_LG_NAMES "")
set(_LG_BODIES "")

# Name fragments — mixed with domain terms to blend in
set(_LG_PREFIXES
  "verify" "check" "validate" "confirm" "ensure" "assert"
  "test" "inspect" "audit" "probe" "scan" "review"
)
set(_LG_MIDDLES
  "Buffer" "Frame" "Config" "Stream" "Token" "State"
  "Module" "Context" "Session" "Channel" "Pipeline" "Calibration"
)
set(_LG_SUFFIXES
  "Integrity" "Alignment" "Coherence" "Consistency" "Signature" "Binding"
  "Checksum" "Validity" "Status" "Health" "Sync" "Parity"
)

list(LENGTH _LG_PREFIXES _n_pre)
list(LENGTH _LG_MIDDLES _n_mid)
list(LENGTH _LG_SUFFIXES _n_suf)

# Build a set of unique function names and check bodies
foreach(_i RANGE 0 23)
  # Derive indices from salt + iteration
  _lg_hash_seed("name_pre_${_i}" _idx_pre)
  math(EXPR _idx_pre "${_idx_pre} % ${_n_pre}")
  list(GET _LG_PREFIXES ${_idx_pre} _pre)

  _lg_hash_seed("name_mid_${_i}" _idx_mid)
  math(EXPR _idx_mid "${_idx_mid} % ${_n_mid}")
  list(GET _LG_MIDDLES ${_idx_mid} _mid)

  _lg_hash_seed("name_suf_${_i}" _idx_suf)
  math(EXPR _idx_suf "${_idx_suf} % ${_n_suf}")
  list(GET _LG_SUFFIXES ${_idx_suf} _suf)

  # Append iteration number to guarantee uniqueness
  set(_fname "${_pre}${_mid}${_suf}${_i}")
  list(APPEND _LG_NAMES "${_fname}")

  # Generate the expected constant for this guard
  # Each guard uses a different transform of the salt
  _lg_hash_seed("const_a_${_i}" _ca)
  _lg_hash_seed("const_b_${_i}" _cb)

  # Compute expected value: (salt ^ ca) + cb, truncated to fit
  math(EXPR _salt_trunc "${COMMERCIAL_BUILD_SALT} % 2147483647")
  math(EXPR _ca_trunc "${_ca} % 2147483647")
  math(EXPR _cb_trunc "${_cb} % 65536")
  math(EXPR _expected "(${_salt_trunc} ^ ${_ca_trunc}) + ${_cb_trunc}")
  # Ensure positive
  if(_expected LESS 0)
    math(EXPR _expected "${_expected} * -1")
  endif()

  # Pick which style of check to generate (varies per guard)
  math(EXPR _style "${_i} % 6")

  if(_style EQUAL 0)
    # Style 0: XOR check
    set(_body
"  const auto s = static_cast<qint64>(COMMERCIAL_BUILD_SALT % 2147483647LL)\;
  const auto v = (s ^ static_cast<qint64>(${_ca_trunc})) + static_cast<qint64>(${_cb_trunc})\;
  return v == static_cast<qint64>(${_expected})\;"
    )
  elseif(_style EQUAL 1)
    # Style 1: Modular arithmetic
    math(EXPR _mod_base "${_ca_trunc} % 9973 + 7")
    math(EXPR _mod_expected "${_salt_trunc} % ${_mod_base}")
    set(_body
"  constexpr qint64 base = ${_mod_base}LL\;
  const auto r = static_cast<qint64>(COMMERCIAL_BUILD_SALT % 2147483647LL) % base\;
  return r == static_cast<qint64>(${_mod_expected})\;"
    )
  elseif(_style EQUAL 2)
    # Style 2: Bit rotation check (use 64-bit in both CMake and C++ to match)
    math(EXPR _rot "${_i} % 15 + 1")
    math(EXPR _rot_inv "63 - ${_rot}")
    math(EXPR _rot_right "${_salt_trunc} >> ${_rot}")
    math(EXPR _rot_left "${_salt_trunc} << ${_rot_inv}")
    math(EXPR _rot_val "${_rot_right} ^ ${_rot_left}")
    if(_rot_val LESS 0)
      math(EXPR _rot_val "${_rot_val} * -1")
    endif()
    set(_body
"  const qint64 s = static_cast<qint64>(COMMERCIAL_BUILD_SALT % 2147483647LL)\;
  const qint64 r = (s >> ${_rot}) ^ (s << ${_rot_inv})\;
  const qint64 v = r < 0 ? -r : r\;
  return v == static_cast<qint64>(${_rot_val}LL)\;"
    )
  elseif(_style EQUAL 3)
    # Style 3: Additive hash
    math(EXPR _add_a "${_ca_trunc} % 65521")
    math(EXPR _add_expected "(${_salt_trunc} + ${_add_a}) % 65521")
    set(_body
"  constexpr quint32 prime = 65521u\;
  const auto s = static_cast<quint32>(COMMERCIAL_BUILD_SALT % 2147483647ULL)\;
  return (s + ${_add_a}u) % prime == ${_add_expected}u\;"
    )
  elseif(_style EQUAL 4)
    # Style 4: Multi-byte extraction (use truncated salt in both CMake and C++)
    math(EXPR _byte_idx "${_i} % 4")
    math(EXPR _byte_shift "${_byte_idx} * 8")
    math(EXPR _byte_val "(${_salt_trunc} >> ${_byte_shift}) % 256")
    set(_body
"  const auto s = static_cast<quint32>(COMMERCIAL_BUILD_SALT % 2147483647ULL)\;
  const auto byte_val = static_cast<quint8>((s >> ${_byte_shift}u) & 0xFFu)\;
  return byte_val == static_cast<quint8>(${_byte_val}u)\;"
    )
  else()
    # Style 5: Polynomial check
    math(EXPR _poly_a "${_ca_trunc} % 251 + 2")
    math(EXPR _poly_b "${_cb_trunc} % 251 + 1")
    math(EXPR _poly_val "((${_salt_trunc} % 65521) * ${_poly_a} + ${_poly_b}) % 65521")
    set(_body
"  constexpr quint32 a = ${_poly_a}u, b = ${_poly_b}u, m = 65521u\;
  const auto s = static_cast<quint32>(COMMERCIAL_BUILD_SALT % 2147483647ULL) % m\;
  return (s * a + b) % m == ${_poly_val}u\;"
    )
  endif()

  list(APPEND _LG_BODIES "${_body}")
endforeach()

#---------------------------------------------------------------------------------------------------
# Generate the header file
#---------------------------------------------------------------------------------------------------

# Build the function declarations
set(_FUNCS "")
set(_DISPATCH "")
foreach(_i RANGE 0 23)
  list(GET _LG_NAMES ${_i} _fname)
  list(GET _LG_BODIES ${_i} _fbody)
  string(APPEND _FUNCS "
[[nodiscard]] inline bool ${_fname}()
{
${_fbody}
}
")

  # Build dispatch entries
  string(APPEND _DISPATCH "    &${_fname},\n")
endforeach()

# Write the header
file(WRITE "${_LG_OUTPUT}"
"//
// AUTO-GENERATED — DO NOT EDIT
// Generated by cmake/GenLicenseGuards.cmake during commercial build configuration.
// This file changes on every build. Do not commit.
//
// SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
//

#pragma once

#include <array>

#if __has_include(<QtTypes>)
#  include <QtTypes>
#else
using quint8  = unsigned char;
using quint32 = unsigned int;
using quint64 = unsigned long long;
using qint64  = long long;
#endif

#ifndef COMMERCIAL_BUILD_SALT
#  define COMMERCIAL_BUILD_SALT 0
#endif

namespace Licensing { namespace Guards {

//--------------------------------------------------------------------------------------------------
// Generated guard functions — each validates a unique transform of the build salt
//--------------------------------------------------------------------------------------------------
${_FUNCS}

//--------------------------------------------------------------------------------------------------
// Dispatch table
//--------------------------------------------------------------------------------------------------

using GuardFn = bool(*)();

inline const std::array<GuardFn, ${_LG_COUNT}>& guardTable()
{
  static const std::array<GuardFn, ${_LG_COUNT}> table = {
${_DISPATCH}  };
  return table;
}

//--------------------------------------------------------------------------------------------------
// Public API — call from feature code
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runs the guard function selected by the call-site line number.
 *
 * Each call site dispatches to a different guard, so patching one doesn't
 * disable the others. The modular dispatch means an attacker must find and
 * neutralize all ${_LG_COUNT} guards to fully bypass protection.
 */
[[nodiscard]] inline bool runGuard(unsigned int site)
{
  return guardTable()[site % ${_LG_COUNT}]();
}

}}  // namespace Licensing::Guards
"
)

message(STATUS "Generated ${_LG_COUNT} license guard functions in ${_LG_OUTPUT}")

# Make the generated directory available as an include path
set(LICENSE_GUARDS_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated" CACHE INTERNAL "")

#---------------------------------------------------------------------------------------------------
# Self-test: compile and run a small program that asserts all guards pass
#---------------------------------------------------------------------------------------------------

set(_LG_GEN_DIR "${CMAKE_BINARY_DIR}/generated")
set(_LG_TEST_SRC "${_LG_GEN_DIR}/LicenseGuardsTest.cpp")

# Write test source — bracket argument [=[ ]=] avoids all CMake escaping issues
file(WRITE "${_LG_TEST_SRC}" [=[
#include <cstdio>
#include <cstdlib>
]=]
"#define COMMERCIAL_BUILD_SALT ${COMMERCIAL_BUILD_SALT}ULL\n"
"#define GUARD_COUNT ${_LG_COUNT}\n"
[=[

using quint8  = unsigned char;
using quint32 = unsigned int;
using quint64 = unsigned long long;
using qint64  = long long;

#include "LicenseGuards.generated.h"

int main()
{
  int failures = 0;
  for (unsigned i = 0; i < GUARD_COUNT; ++i)
  {
    if (!Licensing::Guards::guardTable()[i]())
    {
      std::fprintf(stderr, "FAIL: guard %u\n", i);
      ++failures;
    }
  }

  if (failures)
  {
    std::fprintf(stderr, "%d / %d guards failed\n", failures, GUARD_COUNT);
    return 1;
  }

  std::printf("OK: all %d license guards passed\n", GUARD_COUNT);
  return 0;
}
]=]
)

# try_compile needs CMAKE_FLAGS "-DCMAKE_CXX_FLAGS=..." for include dirs
try_compile(_LG_TEST_COMPILED
  "${_LG_GEN_DIR}/test_build"
  "${_LG_TEST_SRC}"
  CMAKE_FLAGS "-DCMAKE_CXX_FLAGS=-I${_LG_GEN_DIR}"
  CXX_STANDARD 17
)

if(NOT _LG_TEST_COMPILED)
  message(FATAL_ERROR
    "License guard self-test FAILED to compile.\n"
    "The generated guards contain invalid C++. This is a bug in GenLicenseGuards.cmake.")
endif()

try_run(_LG_TEST_RUN_RESULT _LG_TEST_COMPILE_RESULT
  "${_LG_GEN_DIR}/test_run"
  "${_LG_TEST_SRC}"
  CMAKE_FLAGS "-DCMAKE_CXX_FLAGS=-I${_LG_GEN_DIR}"
  CXX_STANDARD 17
  RUN_OUTPUT_VARIABLE _LG_TEST_OUTPUT
)

if(NOT _LG_TEST_RUN_RESULT EQUAL 0)
  message(FATAL_ERROR
    "License guard self-test FAILED at runtime.\n"
    "One or more guards did not pass with COMMERCIAL_BUILD_SALT=${COMMERCIAL_BUILD_SALT}.\n"
    "Output: ${_LG_TEST_OUTPUT}\n"
    "This is a bug in GenLicenseGuards.cmake — the expected values are incorrect.")
endif()

message(STATUS "License guard self-test passed: ${_LG_TEST_OUTPUT}")
