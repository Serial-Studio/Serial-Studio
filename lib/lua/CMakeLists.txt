#
# Lua 5.4.7 — embedded as a static library for the frame parser engine.
#
# Excludes lua.c (standalone interpreter) and luac.c (standalone compiler).
#
# Compiled as C++ since the parent project only enables CXX.
# Lua 5.4 is designed to compile cleanly as both C and C++.
#

cmake_minimum_required(VERSION 3.20)

add_library(lua54 STATIC
  src/lapi.c
  src/lauxlib.c
  src/lbaselib.c
  src/lcode.c
  src/lcorolib.c
  src/lctype.c
  src/ldblib.c
  src/ldebug.c
  src/ldo.c
  src/ldump.c
  src/lfunc.c
  src/lgc.c
  src/linit.c
  src/liolib.c
  src/llex.c
  src/lmathlib.c
  src/lmem.c
  src/loadlib.c
  src/lobject.c
  src/lopcodes.c
  src/loslib.c
  src/lparser.c
  src/lstate.c
  src/lstring.c
  src/lstrlib.c
  src/ltable.c
  src/ltablib.c
  src/ltm.c
  src/lundump.c
  src/lutf8lib.c
  src/lvm.c
  src/lzio.c
)

target_include_directories(lua54 PUBLIC src/)

# Compile .c files as C++ (Lua 5.4 supports this)
set_target_properties(lua54 PROPERTIES
  LINKER_LANGUAGE CXX
)

set_source_files_properties(
  src/lapi.c src/lauxlib.c src/lbaselib.c src/lcode.c src/lcorolib.c
  src/lctype.c src/ldblib.c src/ldebug.c src/ldo.c src/ldump.c
  src/lfunc.c src/lgc.c src/linit.c src/liolib.c src/llex.c
  src/lmathlib.c src/lmem.c src/loadlib.c src/lobject.c src/lopcodes.c
  src/loslib.c src/lparser.c src/lstate.c src/lstring.c src/lstrlib.c
  src/ltable.c src/ltablib.c src/ltm.c src/lundump.c src/lutf8lib.c
  src/lvm.c src/lzio.c
  PROPERTIES LANGUAGE CXX
)

# Lua-as-C++ relies on the C++ exception unwinder: LUAI_THROW expands to
# `throw` and LUAI_TRY to `try { ... } catch(...) { ... }` in ldo.c.
# Production flags inherited from the parent — specifically the combo of
# -ffunction-sections / -fdata-sections, -Wl,-dead_strip, and LTO on
# macOS — can strip personality routines or unwind tables for functions
# that are only reached via a thrown exception. The result is the
# __cxa_throw → demangling_terminate_handler → abort crash you see when
# a Lua runtime error is raised inside lua_pcall (e.g. user calls a nil
# math.log10 inside their transform()).
#
# Force frame pointers, async-unwind tables, and disable section-level
# splitting on this target so the unwinder always has enough metadata to
# walk out of Lua's VM, regardless of what the parent project enabled.
# Performance cost on a ~200KB static lib is negligible.
if(NOT MSVC)
  target_compile_options(lua54 PRIVATE
    -fno-omit-frame-pointer
    -fasynchronous-unwind-tables
    -fexceptions
    -fno-function-sections
    -fno-data-sections
  )
endif()

# Disable LTO for Lua specifically — interprocedural inlining across the
# C++-exception throw path has a track record of dropping unwind info
# when paired with macOS ld's -dead_strip.
set_target_properties(lua54 PROPERTIES
  INTERPROCEDURAL_OPTIMIZATION FALSE
)
