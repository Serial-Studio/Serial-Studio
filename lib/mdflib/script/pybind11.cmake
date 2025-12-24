# Copyright 2024 Ingemar Hedvall
# SPDX-License-Identifier: MIT

include (FetchContent)

include(FetchContent)
FetchContent_Declare(pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG HEAD
)

FetchContent_MakeAvailable(pybind11)
cmake_print_properties(TARGETS pybind11::headers PROPERTIES
        NAME
        INTERFACE_INCLUDE_DIRECTORIES )

cmake_print_variables(
        pybind11_POPULATED
        pybind11_INCLUDE_DIRS
        pybind11_SOURCE_DIR
        pybind11_BINARY_DIR
)
