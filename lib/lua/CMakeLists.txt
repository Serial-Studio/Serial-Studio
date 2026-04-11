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
