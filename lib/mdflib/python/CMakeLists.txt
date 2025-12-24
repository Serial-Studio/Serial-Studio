# Copyright 2024 Ingemar Hedvall
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.20)
include(CMakePrintHelpers)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # optional, ensure standard is supported
set(CMAKE_CXX_EXTENSIONS OFF)  # optional, keep compiler extensions off
set(CMAKE_DEBUG_POSTFIX "") # The import lib function doesn't accept the 'd' pos fix



cmake_print_properties(TARGETS Pytest::Pytest PROPERTIES
        NAME
        LOCATION)

cmake_print_properties(TARGETS pybind11::headers PROPERTIES
        NAME
        LOCATION
        INTERFACE_INCLUDE_DIRECTORIES
)
cmake_print_variables(pybind11_INCLUDE_DIRECTORIES)

project(pymdf_lib
        VERSION 1.0
        DESCRIPTION "ASAM MDF Python Module."
        LANGUAGES CXX)


pybind11_add_module( pymdf MODULE
        src/pysamplerecord.cpp
        src/pymdf.h src/pymdf.cpp )

cmake_print_variables(CMAKE_SOURCE_DIR)
cmake_print_variables(CMAKE_CURRENT_SOURCE_DIR)
cmake_print_variables(CMAKE_BINARY_DIR)
cmake_print_variables(CMAKE_CURRENT_BINARY_DIR)
cmake_print_variables(Python3_EXECUTABLE)


target_include_directories(pymdf PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(pymdf PRIVATE ${pybind11_INCLUDE_DIRS})

target_link_libraries(pymdf PRIVATE mdf)

cmake_print_properties(TARGETS pymdf PROPERTIES
        NAME
        INCLUDE_DIRECTORIES
)

if (MDF_BUILD_TEST)

    # Need to copy the pyd modules to the virtual environment
    # so the CLION can handle the pytest properly
    file(GLOB module_files CONFIGURE_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/pymdf.* )

    foreach (pyfile ${module_files})
        add_custom_command(
                TARGET pymdf
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ARGS ${pyfile} "${CMAKE_SOURCE_DIR}/venv/Lib"
        )
    endforeach ()


    # Add the test to the CTest. The pyd module is in the current binary dir,
    # so the pytest should run.
    enable_testing()
    add_test(NAME pymdf_test
            COMMAND ${Python3_EXECUTABLE} -m pytest --log-cli-level=0 ${CMAKE_CURRENT_SOURCE_DIR}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}

    )

endif()