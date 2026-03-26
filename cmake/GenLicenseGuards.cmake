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
  math(EXPR _style "${_i} % 12")

  if(_style EQUAL 0)
    # Style 0: XOR check
    set(_body
"  const auto s = static_cast<qint64>(runtimeSalt() % 2147483647LL)\;
  const auto v = (s ^ static_cast<qint64>(${_ca_trunc})) + static_cast<qint64>(${_cb_trunc})\;
  return v == static_cast<qint64>(${_expected})\;"
    )
  elseif(_style EQUAL 1)
    # Style 1: Modular arithmetic
    math(EXPR _mod_base "${_ca_trunc} % 9973 + 7")
    math(EXPR _mod_expected "${_salt_trunc} % ${_mod_base}")
    set(_body
"  constexpr qint64 base = ${_mod_base}LL\;
  const auto r = static_cast<qint64>(runtimeSalt() % 2147483647LL) % base\;
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
"  const qint64 s = static_cast<qint64>(runtimeSalt() % 2147483647LL)\;
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
  const auto s = static_cast<quint32>(runtimeSalt() % 2147483647ULL)\;
  return (s + ${_add_a}u) % prime == ${_add_expected}u\;"
    )
  elseif(_style EQUAL 4)
    # Style 4: Multi-byte extraction (use truncated salt in both CMake and C++)
    math(EXPR _byte_idx "${_i} % 4")
    math(EXPR _byte_shift "${_byte_idx} * 8")
    math(EXPR _byte_val "(${_salt_trunc} >> ${_byte_shift}) % 256")
    set(_body
"  const auto s = static_cast<quint32>(runtimeSalt() % 2147483647ULL)\;
  const auto byte_val = static_cast<quint8>((s >> ${_byte_shift}u) & 0xFFu)\;
  return byte_val == static_cast<quint8>(${_byte_val}u)\;"
    )
  elseif(_style EQUAL 5)
    # Style 5: Polynomial check
    math(EXPR _poly_a "${_ca_trunc} % 251 + 2")
    math(EXPR _poly_b "${_cb_trunc} % 251 + 1")
    math(EXPR _poly_val "((${_salt_trunc} % 65521) * ${_poly_a} + ${_poly_b}) % 65521")
    set(_body
"  constexpr quint32 a = ${_poly_a}u, b = ${_poly_b}u, m = 65521u\;
  const auto s = static_cast<quint32>(runtimeSalt() % 2147483647ULL) % m\;
  return (s * a + b) % m == ${_poly_val}u\;"
    )
  elseif(_style EQUAL 6)
    # Style 6: Fibonacci-sequence modular check
    math(EXPR _fib_mod "${_ca_trunc} % 9949 + 53")
    math(EXPR _fib_steps "${_i} % 7 + 5")
    # Compute Fibonacci-like sequence: f(0)=salt%mod, f(1)=cb%mod, f(n)=(f(n-1)+f(n-2))%mod
    math(EXPR _fib_a "${_salt_trunc} % ${_fib_mod}")
    math(EXPR _fib_b "${_cb_trunc} % ${_fib_mod}")
    foreach(_fi RANGE 2 ${_fib_steps})
      math(EXPR _fib_c "(${_fib_a} + ${_fib_b}) % ${_fib_mod}")
      set(_fib_a "${_fib_b}")
      set(_fib_b "${_fib_c}")
    endforeach()
    set(_body
"  constexpr quint32 mod = ${_fib_mod}u\;
  const auto s = static_cast<quint32>(runtimeSalt() % 2147483647ULL) % mod\;
  quint32 a = s, b = ${_cb_trunc}u % mod\;
  for (int i = 2\; i <= ${_fib_steps}\; ++i)
  {
    const quint32 c = (a + b) % mod\;
    a = b\;
    b = c\;
  }
  return b == ${_fib_b}u\;"
    )
  elseif(_style EQUAL 7)
    # Style 7: Multiplicative hash (Knuth-style golden ratio)
    math(EXPR _mul_k "${_ca_trunc} | 1")
    math(EXPR _mul_val "((${_salt_trunc} * ${_mul_k}) >> 16) % 65521")
    if(_mul_val LESS 0)
      math(EXPR _mul_val "${_mul_val} * -1")
    endif()
    set(_body
"  const auto s = static_cast<qint64>(runtimeSalt() % 2147483647LL)\;
  constexpr qint64 k = ${_mul_k}LL\;
  const auto v = ((s * k) >> 16) % 65521LL\;
  const auto r = v < 0 ? -v : v\;
  return r == static_cast<qint64>(${_mul_val}LL)\;"
    )
  elseif(_style EQUAL 8)
    # Style 8: Two-round Feistel-like mix
    math(EXPR _fe_k1 "${_ca_trunc} % 65521 + 1")
    math(EXPR _fe_k2 "${_cb_trunc} % 65521 + 1")
    # Split salt into two 16-bit halves, mix
    math(EXPR _fe_lo "${_salt_trunc} % 65536")
    math(EXPR _fe_hi "(${_salt_trunc} >> 16) % 65536")
    # Round 1: hi' = lo ^ ((hi * k1) % 65536)
    math(EXPR _fe_r1 "${_fe_lo} ^ ((${_fe_hi} * ${_fe_k1}) % 65536)")
    math(EXPR _fe_r1 "${_fe_r1} % 65536")
    # Round 2: lo' = hi ^ ((hi' * k2) % 65536)
    math(EXPR _fe_r2 "${_fe_hi} ^ ((${_fe_r1} * ${_fe_k2}) % 65536)")
    math(EXPR _fe_r2 "${_fe_r2} % 65536")
    # Combine
    math(EXPR _fe_expected "(${_fe_r1} << 16) | ${_fe_r2}")
    set(_body
"  const auto s = static_cast<quint32>(runtimeSalt() % 2147483647ULL)\;
  const quint32 lo = s & 0xFFFFu, hi = (s >> 16) & 0xFFFFu\;
  constexpr quint32 k1 = ${_fe_k1}u, k2 = ${_fe_k2}u\;
  const quint32 r1 = (lo ^ ((hi * k1) % 65536u)) & 0xFFFFu\;
  const quint32 r2 = (hi ^ ((r1 * k2) % 65536u)) & 0xFFFFu\;
  return ((r1 << 16) | r2) == ${_fe_expected}u\;"
    )
  elseif(_style EQUAL 9)
    # Style 9: CRC-inspired polynomial division
    math(EXPR _crc_poly "${_ca_trunc} % 255 + 256")
    # Compute byte-by-byte CRC over 4 bytes of salt
    set(_crc_val 0)
    foreach(_bi RANGE 0 3)
      math(EXPR _byte_shift "${_bi} * 8")
      math(EXPR _sbyte "(${_salt_trunc} >> ${_byte_shift}) % 256")
      math(EXPR _crc_val "${_crc_val} ^ ${_sbyte}")
      foreach(_bit RANGE 0 7)
        math(EXPR _crc_hi "${_crc_val} % 2")
        if(_crc_hi)
          math(EXPR _crc_val "(${_crc_val} >> 1) ^ ${_crc_poly}")
        else()
          math(EXPR _crc_val "${_crc_val} >> 1")
        endif()
        math(EXPR _crc_val "${_crc_val} % 65536")
      endforeach()
    endforeach()
    set(_body
"  const auto s = static_cast<quint32>(runtimeSalt() % 2147483647ULL)\;
  constexpr quint32 poly = ${_crc_poly}u\;
  quint32 crc = 0\;
  for (int i = 0\; i < 4\; ++i)
  {
    crc ^= (s >> (i * 8)) & 0xFFu\;
    for (int b = 0\; b < 8\; ++b)
      crc = (crc & 1u) ? ((crc >> 1) ^ poly) : (crc >> 1)\;
    crc &= 0xFFFFu\;
  }
  return crc == ${_crc_val}u\;"
    )
  elseif(_style EQUAL 10)
    # Style 10: Bit-count parity check (popcount of masked value)
    math(EXPR _pc_mask "${_ca_trunc} | 1")
    math(EXPR _pc_masked "${_salt_trunc} & ${_pc_mask}")
    # Count bits set in CMake (manual popcount)
    set(_pc_count 0)
    set(_pc_tmp ${_pc_masked})
    foreach(_unused RANGE 0 30)
      math(EXPR _pc_bit "${_pc_tmp} % 2")
      math(EXPR _pc_count "${_pc_count} + ${_pc_bit}")
      math(EXPR _pc_tmp "${_pc_tmp} >> 1")
      if(_pc_tmp EQUAL 0)
        break()
      endif()
    endforeach()
    math(EXPR _pc_parity "${_pc_count} % 2")
    math(EXPR _pc_high_bits "${_pc_count} >> 1")
    set(_body
"  const auto s = static_cast<quint32>(runtimeSalt() % 2147483647ULL)\;
  constexpr quint32 mask = ${_pc_mask}u\;
  const quint32 masked = s & mask\;
  int bits = 0\;
  for (quint32 v = masked\; v\; v >>= 1)
    bits += static_cast<int>(v & 1u)\;
  return (bits % 2 == ${_pc_parity}u) && (bits >> 1 == ${_pc_high_bits}u)\;"
    )
  else()
    # Style 11: Cascaded XOR-shift
    _lg_hash_seed("xorshift_k_${_i}" _xor_k)
    math(EXPR _xor_k "${_xor_k} % 2147483647")
    math(EXPR _xs_v "${_salt_trunc} ^ ${_xor_k}")
    # Three rounds of xorshift
    math(EXPR _xs_v "${_xs_v} ^ (${_xs_v} >> 7)")
    if(_xs_v LESS 0)
      math(EXPR _xs_v "${_xs_v} * -1")
    endif()
    math(EXPR _xs_v "${_xs_v} ^ (${_xs_v} >> 3)")
    math(EXPR _xs_v "${_xs_v} % 2147483647")
    math(EXPR _xs_v "${_xs_v} ^ (${_xs_v} >> 11)")
    if(_xs_v LESS 0)
      math(EXPR _xs_v "${_xs_v} * -1")
    endif()
    math(EXPR _xs_v "${_xs_v} % 2147483647")
    set(_body
"  const auto s = static_cast<qint64>(runtimeSalt() % 2147483647LL)\;
  constexpr qint64 k = ${_xor_k}LL\;
  qint64 v = s ^ k\;
  v ^= (v >> 7)\;
  v = v < 0 ? -v : v\;
  v ^= (v >> 3)\;
  v %= 2147483647LL\;
  v ^= (v >> 11)\;
  v = v < 0 ? -v : v\;
  v %= 2147483647LL\;
  return v == static_cast<qint64>(${_xs_v}LL)\;"
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

# Split the salt into 4 fragments with XOR masking so it never appears as a
# single immediate value in the binary. Each fragment is salt_part ^ mask,
# reassembled at runtime.
_lg_hash_seed("frag_mask_0" _fm0)
_lg_hash_seed("frag_mask_1" _fm1)
_lg_hash_seed("frag_mask_2" _fm2)
_lg_hash_seed("frag_mask_3" _fm3)

# Extract 16-bit fragments from salt
math(EXPR _sf0 "${COMMERCIAL_BUILD_SALT} & 0xFFFF")
math(EXPR _sf1 "(${COMMERCIAL_BUILD_SALT} >> 16) & 0xFFFF")
math(EXPR _sf2 "(${COMMERCIAL_BUILD_SALT} >> 32) & 0xFFFF")
math(EXPR _sf3 "(${COMMERCIAL_BUILD_SALT} >> 48) & 0xFFFF")

# Mask each fragment
math(EXPR _mk0 "${_fm0} & 0xFFFF")
math(EXPR _mk1 "${_fm1} & 0xFFFF")
math(EXPR _mk2 "${_fm2} & 0xFFFF")
math(EXPR _mk3 "${_fm3} & 0xFFFF")
math(EXPR _ef0 "${_sf0} ^ ${_mk0}")
math(EXPR _ef1 "${_sf1} ^ ${_mk1}")
math(EXPR _ef2 "${_sf2} ^ ${_mk2}")
math(EXPR _ef3 "${_sf3} ^ ${_mk3}")

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
// Runtime salt — loaded through volatile to defeat constant-folding
//--------------------------------------------------------------------------------------------------

inline quint64 runtimeSalt()
{
  // Salt is split into 4 XOR-masked 16-bit fragments and reassembled here.
  // The volatile reads prevent the compiler from folding these into a single
  // constant, so guard functions contain real arithmetic in the binary.
  static volatile quint32 f0 = ${_ef0}u;
  static volatile quint32 f1 = ${_ef1}u;
  static volatile quint32 f2 = ${_ef2}u;
  static volatile quint32 f3 = ${_ef3}u;

  const quint64 p0 = static_cast<quint64>(f0 ^ ${_mk0}u);
  const quint64 p1 = static_cast<quint64>(f1 ^ ${_mk1}u);
  const quint64 p2 = static_cast<quint64>(f2 ^ ${_mk2}u);
  const quint64 p3 = static_cast<quint64>(f3 ^ ${_mk3}u);
  return p0 | (p1 << 16) | (p2 << 32) | (p3 << 48);
}

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
