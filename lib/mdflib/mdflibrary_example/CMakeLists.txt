# Copyright 2023 Simplxs
# SPDX-License-Identifier: MIT
project(
  mdflibraryexample
  VERSION 1.0
  DESCRIPTION "MdfLibraryTest"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(mdflibraryexample src/test.cpp)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_target_properties(mdflibraryexample PROPERTIES LINK_FLAGS
                                                  "-Wl,-rpath,'$ORIGIN'")
endif()

target_link_libraries(mdflibraryexample PRIVATE mdflibrary)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.3 AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0 AND NOT MINGW)
  # fix for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=90050
  target_link_libraries(mdflibraryexample PRIVATE pthread stdc++fs)
endif()

target_include_directories(
  mdflibraryexample PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
