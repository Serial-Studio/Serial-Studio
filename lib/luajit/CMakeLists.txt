#
# LuaJIT 2.1 (rolling; .relver holds the commit's epoch, and lib/VERSIONS.json resolves it
# to the commit SHA — see also doc/claude/specs/0051) —
# embedded as a static library replacing Lua 5.4 for every scripting surface (spec 0051 M2).
#
# LuaJIT cannot reuse the lua54 "compile as C++" recipe: it is C-only, ships a two-stage
# build (host tools generate the VM), and provides its own error unwinding. This file ports
# src/Makefile + src/msvcbuild.bat to CMake for NATIVE builds on the architectures Serial
# Studio ships (x86-64 and arm64; macOS / Linux / Windows MSVC+clang-cl / MinGW). Cross
# compilation is intentionally unsupported — host tools are built with the target toolchain.
# The one sanctioned exception is macOS x86_64 on an arm64 host: the target-arch host tools
# execute under Rosetta, so the native-build invariant (buildvm runs what it targets) holds.
#
# Stage 1 (host): minilua -> DynASM generates buildvm_arch.h -> buildvm.
# Stage 2 (target): buildvm emits lj_vm.S (or lj_vm.obj via peobj on MSVC) plus the
# lj_*def.h tables; the static library compiles lj_*.c + lib_*.c + the VM object.
#
# Invariants honored (cpp-compiler-flags skill):
#  - IEEE-stable math: no fast-math anywhere; LuaJIT requires precise semantics itself.
#  - LTO is disabled on this target (asm VM + generated tables interact badly with IPO;
#    same policy the lua54 target had on macOS, applied everywhere for LuaJIT).
#  - -fno-stack-protector on the VM library mirrors upstream's Makefile: interpreter frames
#    hand-crafted in assembly do not carry canaries; the rest of the app stays hardened.
#

cmake_minimum_required(VERSION 3.20)

enable_language(C)
if(NOT MSVC)
  enable_language(ASM)
endif()

set(LJ_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LJ_GEN "${CMAKE_CURRENT_BINARY_DIR}/gen")
file(MAKE_DIRECTORY "${LJ_GEN}")

#---------------------------------------------------------------------------------------------------
# Architecture + DynASM flag derivation
#---------------------------------------------------------------------------------------------------
#
# Non-MSVC replicates src/Makefile's TARGET_TESTARCH probe: preprocess lj_arch.h with -dM and
# derive the DynASM flags from the LJ_* macros, exactly like upstream. MSVC (cl and clang-cl)
# uses the hardcoded msvcbuild.bat recipe instead (x64 and arm64 only).
#
# The probe must see the TARGET architecture flags, exactly like upstream's TARGET_TESTARCH
# (which probes via TARGET_CC with its -arch flags). A bare compiler invocation probes the
# build HOST: on the macOS x86_64 CI leg (arm64 runner + CMAKE_OSX_ARCHITECTURES=x86_64,
# executed under Rosetta) that generated an arm64 buildvm_arch.h compiled into an x86-64
# buildvm — the dual-number #error and the missing CCallState retp member. Single-arch
# builds only: the universal dmg is a post-build lipo merge of two single-arch builds, and
# a fat buildvm_arch.h cannot exist.
#

set(LJ_DASM_FLAGS "")
set(LJ_DASC "")
set(LJ_BUILDVM_DEFS "")

if(MSVC)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|arm64|aarch64")
    set(LJ_DASC "vm_arm64.dasc")
    list(APPEND LJ_BUILDVM_DEFS "LUAJIT_TARGET=LUAJIT_ARCH_ARM64")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64")
    set(LJ_DASC "vm_x64.dasc")
  else()
    message(FATAL_ERROR "LuaJIT: unsupported MSVC architecture '${CMAKE_SYSTEM_PROCESSOR}'")
  endif()
  set(LJ_DASM_FLAGS -D WIN -D JIT -D FFI -D ENDIAN_LE -D FPU -D P64)
else()
  set(LJ_PROBE_ARCH_FLAGS "")
  if(APPLE AND CMAKE_OSX_ARCHITECTURES)
    list(LENGTH CMAKE_OSX_ARCHITECTURES LJ_OSX_ARCH_COUNT)
    if(LJ_OSX_ARCH_COUNT GREATER 1)
      message(FATAL_ERROR
        "LuaJIT: single-architecture builds only (CMAKE_OSX_ARCHITECTURES='"
        "${CMAKE_OSX_ARCHITECTURES}'); build each slice separately and lipo the results")
    endif()
    list(APPEND LJ_PROBE_ARCH_FLAGS -arch ${CMAKE_OSX_ARCHITECTURES})
  endif()

  execute_process(
    COMMAND ${CMAKE_C_COMPILER} ${LJ_PROBE_ARCH_FLAGS} -E -dM -x c "${LJ_SRC}/lj_arch.h"
    OUTPUT_VARIABLE LJ_TESTARCH
    RESULT_VARIABLE LJ_TESTARCH_RC
    ERROR_QUIET)
  if(NOT LJ_TESTARCH_RC EQUAL 0)
    message(FATAL_ERROR "LuaJIT: lj_arch.h architecture probe failed")
  endif()

  if(LJ_TESTARCH MATCHES "LJ_TARGET_X64 ")
    set(LJ_ARCH "x64")
  elseif(LJ_TESTARCH MATCHES "LJ_TARGET_ARM64 ")
    set(LJ_ARCH "arm64")
  else()
    message(FATAL_ERROR "LuaJIT: unsupported architecture (probe found no X64/ARM64 target)")
  endif()

  if(LJ_TESTARCH MATCHES "LJ_LE 1")
    list(APPEND LJ_DASM_FLAGS -D ENDIAN_LE)
  else()
    list(APPEND LJ_DASM_FLAGS -D ENDIAN_BE)
  endif()
  if(LJ_TESTARCH MATCHES "LJ_ARCH_BITS 64")
    list(APPEND LJ_DASM_FLAGS -D P64)
  endif()
  if(LJ_TESTARCH MATCHES "LJ_HASJIT 1")
    list(APPEND LJ_DASM_FLAGS -D JIT)
  endif()
  if(LJ_TESTARCH MATCHES "LJ_HASFFI 1")
    list(APPEND LJ_DASM_FLAGS -D FFI)
  endif()
  if(LJ_TESTARCH MATCHES "LJ_DUALNUM 1")
    list(APPEND LJ_DASM_FLAGS -D DUALNUM)
  endif()
  if(LJ_TESTARCH MATCHES "LJ_ARCH_HASFPU 1")
    list(APPEND LJ_DASM_FLAGS -D FPU)
  endif()
  if(NOT LJ_TESTARCH MATCHES "LJ_ABI_SOFTFP 1")
    list(APPEND LJ_DASM_FLAGS -D HFABI)
  endif()
  if(LJ_TESTARCH MATCHES "LJ_NO_UNWIND 1")
    list(APPEND LJ_DASM_FLAGS -D NO_UNWIND)
  endif()
  if(LJ_TESTARCH MATCHES "LJ_ABI_PAUTH 1")
    list(APPEND LJ_DASM_FLAGS -D PAUTH)
  endif()
  if(LJ_TESTARCH MATCHES "LJ_ARCH_VERSION ([0-9]+)")
    list(APPEND LJ_DASM_FLAGS -D "VER=${CMAKE_MATCH_1}")
  endif()
  if(WIN32)
    list(APPEND LJ_DASM_FLAGS -D WIN)
  endif()

  if(LJ_ARCH STREQUAL "x64")
    if(LJ_TESTARCH MATCHES "LJ_FR2 1")
      set(LJ_DASC "vm_x64.dasc")
    else()
      set(LJ_DASC "vm_x86.dasc")
    endif()
  else()
    set(LJ_DASC "vm_arm64.dasc")
  endif()

  message(STATUS "LuaJIT: arch=${LJ_ARCH} dasc=${LJ_DASC}")
endif()

#---------------------------------------------------------------------------------------------------
# Stage 1 — host tools (built with the target toolchain; native builds only)
#---------------------------------------------------------------------------------------------------

add_executable(luajit-minilua "${LJ_SRC}/host/minilua.c")
if(NOT MSVC)
  target_link_libraries(luajit-minilua PRIVATE m)
endif()
set_target_properties(luajit-minilua PROPERTIES INTERPROCEDURAL_OPTIMIZATION FALSE)
ss_exclude_from_pgo(luajit-minilua)

add_custom_command(
  OUTPUT "${LJ_GEN}/buildvm_arch.h"
  COMMAND luajit-minilua "${CMAKE_CURRENT_SOURCE_DIR}/dynasm/dynasm.lua" -LN ${LJ_DASM_FLAGS}
          -o "${LJ_GEN}/buildvm_arch.h" "${LJ_SRC}/${LJ_DASC}"
  WORKING_DIRECTORY "${LJ_SRC}"
  DEPENDS luajit-minilua "${LJ_SRC}/${LJ_DASC}"
  COMMENT "LuaJIT: DynASM ${LJ_DASC}")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.relver" LJ_RELVER)
string(STRIP "${LJ_RELVER}" LJ_RELVER)
file(WRITE "${LJ_GEN}/luajit_relver.txt" "${LJ_RELVER}\n")

add_custom_command(
  OUTPUT "${LJ_GEN}/luajit.h"
  COMMAND luajit-minilua "${LJ_SRC}/host/genversion.lua" "${LJ_SRC}/luajit_rolling.h"
          "${LJ_GEN}/luajit_relver.txt" "${LJ_GEN}/luajit.h"
  DEPENDS luajit-minilua "${LJ_SRC}/luajit_rolling.h" "${LJ_GEN}/luajit_relver.txt"
  COMMENT "LuaJIT: version header")

add_executable(luajit-buildvm
  "${LJ_SRC}/host/buildvm.c"
  "${LJ_SRC}/host/buildvm_asm.c"
  "${LJ_SRC}/host/buildvm_peobj.c"
  "${LJ_SRC}/host/buildvm_lib.c"
  "${LJ_SRC}/host/buildvm_fold.c"
  "${LJ_GEN}/buildvm_arch.h"
  "${LJ_GEN}/luajit.h")
target_include_directories(luajit-buildvm PRIVATE
  "${LJ_GEN}" "${LJ_SRC}" "${CMAKE_CURRENT_SOURCE_DIR}/dynasm")
target_compile_definitions(luajit-buildvm PRIVATE ${LJ_BUILDVM_DEFS})
set_target_properties(luajit-buildvm PROPERTIES INTERPROCEDURAL_OPTIMIZATION FALSE)
ss_exclude_from_pgo(luajit-buildvm)

#---------------------------------------------------------------------------------------------------
# Stage 2 — generated VM + tables
#---------------------------------------------------------------------------------------------------

set(LJ_LIB_C
  "${LJ_SRC}/lib_base.c" "${LJ_SRC}/lib_math.c" "${LJ_SRC}/lib_bit.c" "${LJ_SRC}/lib_string.c"
  "${LJ_SRC}/lib_table.c" "${LJ_SRC}/lib_io.c" "${LJ_SRC}/lib_os.c" "${LJ_SRC}/lib_package.c"
  "${LJ_SRC}/lib_debug.c" "${LJ_SRC}/lib_jit.c" "${LJ_SRC}/lib_ffi.c" "${LJ_SRC}/lib_buffer.c")

if(MSVC)
  set(LJ_VM_OUT "${LJ_GEN}/lj_vm.obj")
  set(LJ_VM_MODE peobj)
elseif(APPLE)
  set(LJ_VM_OUT "${LJ_GEN}/lj_vm.S")
  set(LJ_VM_MODE machasm)
else()
  set(LJ_VM_OUT "${LJ_GEN}/lj_vm.S")
  set(LJ_VM_MODE elfasm)
endif()

add_custom_command(
  OUTPUT "${LJ_VM_OUT}"
  COMMAND luajit-buildvm -m ${LJ_VM_MODE} -o "${LJ_VM_OUT}"
  DEPENDS luajit-buildvm
  COMMENT "LuaJIT: buildvm ${LJ_VM_MODE}")

foreach(def bcdef ffdef libdef recdef)
  add_custom_command(
    OUTPUT "${LJ_GEN}/lj_${def}.h"
    COMMAND luajit-buildvm -m ${def} -o "${LJ_GEN}/lj_${def}.h" ${LJ_LIB_C}
    DEPENDS luajit-buildvm ${LJ_LIB_C}
    COMMENT "LuaJIT: buildvm ${def}")
endforeach()

add_custom_command(
  OUTPUT "${LJ_GEN}/lj_folddef.h"
  COMMAND luajit-buildvm -m folddef -o "${LJ_GEN}/lj_folddef.h" "${LJ_SRC}/lj_opt_fold.c"
  DEPENDS luajit-buildvm "${LJ_SRC}/lj_opt_fold.c"
  COMMENT "LuaJIT: buildvm folddef")

#---------------------------------------------------------------------------------------------------
# The library
#---------------------------------------------------------------------------------------------------

set(LJ_CORE_C
  lj_assert.c lj_gc.c lj_err.c lj_char.c lj_bc.c lj_obj.c lj_buf.c
  lj_str.c lj_tab.c lj_func.c lj_udata.c lj_meta.c lj_debug.c
  lj_prng.c lj_state.c lj_dispatch.c lj_vmevent.c lj_vmmath.c
  lj_strscan.c lj_strfmt.c lj_strfmt_num.c lj_serialize.c
  lj_api.c lj_profile.c
  lj_lex.c lj_parse.c lj_bcread.c lj_bcwrite.c lj_load.c
  lj_ir.c lj_opt_mem.c lj_opt_fold.c lj_opt_narrow.c
  lj_opt_dce.c lj_opt_loop.c lj_opt_split.c lj_opt_sink.c
  lj_mcode.c lj_snap.c lj_record.c lj_crecord.c lj_ffrecord.c
  lj_asm.c lj_trace.c lj_gdbjit.c
  lj_ctype.c lj_cdata.c lj_cconv.c lj_ccall.c lj_ccallback.c
  lj_carith.c lj_clib.c lj_cparse.c
  lj_lib.c lj_alloc.c lib_aux.c lib_init.c)

set(LJ_SOURCES "")
foreach(src ${LJ_CORE_C})
  list(APPEND LJ_SOURCES "${LJ_SRC}/${src}")
endforeach()
list(APPEND LJ_SOURCES ${LJ_LIB_C})
list(APPEND LJ_SOURCES
  "${LJ_GEN}/lj_bcdef.h" "${LJ_GEN}/lj_ffdef.h" "${LJ_GEN}/lj_libdef.h"
  "${LJ_GEN}/lj_recdef.h" "${LJ_GEN}/lj_folddef.h" "${LJ_GEN}/luajit.h")

if(MSVC)
  list(APPEND LJ_SOURCES "${LJ_VM_OUT}")
  set_source_files_properties("${LJ_VM_OUT}" PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)
else()
  list(APPEND LJ_SOURCES "${LJ_VM_OUT}")
  set_source_files_properties("${LJ_VM_OUT}" PROPERTIES LANGUAGE ASM GENERATED TRUE)
endif()

add_library(luajit STATIC ${LJ_SOURCES})

target_include_directories(luajit PRIVATE "${LJ_GEN}")
target_include_directories(luajit PUBLIC "${LJ_SRC}" "${LJ_GEN}")

# LUA52COMPAT restores the 5.2-era APIs the app's compat shim builds on (spec 0051 R19).
# External unwinding is the Makefile's own choice on macOS and probed-ELF targets; Windows
# uses LuaJIT's built-in SEH path with no define (msvcbuild parity).
target_compile_definitions(luajit PRIVATE LUAJIT_ENABLE_LUA52COMPAT)
if(APPLE OR (UNIX AND NOT WIN32))
  target_compile_definitions(luajit PRIVATE LUAJIT_UNWIND_EXTERNAL)
endif()

if(NOT MSVC)
  target_compile_options(luajit PRIVATE -fno-stack-protector)
endif()

if(APPLE AND CMAKE_C_COMPILER_ID MATCHES "^(Clang|AppleClang)$")
  target_compile_options(luajit PRIVATE -fno-lto -fno-whole-program-vtables)
endif()

set_target_properties(luajit PROPERTIES INTERPROCEDURAL_OPTIMIZATION FALSE)
