# Copyright 2025 Ingemar Hedvall
# SPDX-License-Identifier: MIT

include (FetchContent)
include(CMakePrintHelpers)

FetchContent_Declare(utillib
        GIT_REPOSITORY https://github.com/ihedvall/utillib.git
        GIT_TAG HEAD)
set(UTIL_DOC OFF)
set(UTIL_TEST OFF)
set(UTIL_TOOLS OFF)

FetchContent_MakeAvailable(utillib)
cmake_print_variables(utillib_POPULATED utillib_SOURCE_DIR utillib_BINARY_DIR)
cmake_print_properties(TARGETS util PROPERTIES INCLUDE_DIRECTORIES LINK_DIRECTORIES LINK_LIBRARIES)
