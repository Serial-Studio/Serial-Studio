#
# Serial Studio
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
# You must comply with the terms of one of these licenses, depending
# on your use case.
#
# For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
# For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
#
# SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
#

cmake_minimum_required(VERSION 3.20)
project(SerialStudioTests LANGUAGES CXX VERSION ${PROJECT_VERSION})

#-------------------------------------------------------------------------------
# C++ options
#-------------------------------------------------------------------------------

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#-------------------------------------------------------------------------------
# Find Qt Test module
#-------------------------------------------------------------------------------

find_package(Qt6 REQUIRED COMPONENTS Test Core Qml Widgets)

#-------------------------------------------------------------------------------
# Enable CTest
#-------------------------------------------------------------------------------

enable_testing()

#-------------------------------------------------------------------------------
# Common include directories
#-------------------------------------------------------------------------------

include_directories(
  ${CMAKE_SOURCE_DIR}/app/src
  ${CMAKE_SOURCE_DIR}/lib
)

#-------------------------------------------------------------------------------
# Helper function to create and register tests
#-------------------------------------------------------------------------------

function(add_serial_studio_test test_name)
  add_executable(${test_name} ${ARGN})

  target_link_libraries(${test_name}
    Qt6::Test
    Qt6::Core
    Qt6::Qml
    Qt6::Widgets
  )

  add_test(NAME ${test_name} COMMAND ${test_name})

  set_target_properties(${test_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
  )
endfunction()

#-------------------------------------------------------------------------------
# Copy test fixtures to build directory
#-------------------------------------------------------------------------------

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fixtures
     DESTINATION ${CMAKE_BINARY_DIR}/tests)

#-------------------------------------------------------------------------------
# IO Module Tests
#-------------------------------------------------------------------------------

# Test: Checksum algorithms
add_serial_studio_test(test_checksum
  IO/test_checksum.cpp
  ${CMAKE_SOURCE_DIR}/app/src/IO/Checksum.cpp
)

# Test: Checksum fuzzy testing
add_serial_studio_test(test_checksum_fuzz
  IO/test_checksum_fuzz.cpp
  ${CMAKE_SOURCE_DIR}/app/src/IO/Checksum.cpp
)

# Test: Circular buffer
add_serial_studio_test(test_circular_buffer
  IO/test_circular_buffer.cpp
)

# Test: Fixed queue
add_serial_studio_test(test_fixed_queue
  IO/test_fixed_queue.cpp
)

# Test: Frame reader
add_serial_studio_test(test_frame_reader
  IO/test_frame_reader.cpp
  ${CMAKE_SOURCE_DIR}/app/src/IO/FrameReader.cpp
  ${CMAKE_SOURCE_DIR}/app/src/IO/Checksum.cpp
  ${CMAKE_SOURCE_DIR}/app/src/SerialStudio.cpp
)

# Test: Console
add_serial_studio_test(test_console
  IO/test_console.cpp
  ${CMAKE_SOURCE_DIR}/app/src/IO/Console.cpp
  ${CMAKE_SOURCE_DIR}/app/src/IO/CircularBuffer.h
)

#-------------------------------------------------------------------------------
# JSON Module Tests
#-------------------------------------------------------------------------------

# Test: Frame structures
add_serial_studio_test(test_frame
  JSON/test_frame.cpp
  ${CMAKE_SOURCE_DIR}/app/src/JSON/Frame.cpp
  ${CMAKE_SOURCE_DIR}/app/src/SerialStudio.cpp
)

# Test: Frame serialization
add_serial_studio_test(test_frame_serialization
  JSON/test_frame_serialization.cpp
  ${CMAKE_SOURCE_DIR}/app/src/JSON/Frame.cpp
  ${CMAKE_SOURCE_DIR}/app/src/SerialStudio.cpp
)

# Test: Frame deserialization
add_serial_studio_test(test_frame_deserialization
  JSON/test_frame_deserialization.cpp
  ${CMAKE_SOURCE_DIR}/app/src/JSON/Frame.cpp
  ${CMAKE_SOURCE_DIR}/app/src/SerialStudio.cpp
)

# Test: Frame utilities
add_serial_studio_test(test_frame_utilities
  JSON/test_frame_utilities.cpp
  ${CMAKE_SOURCE_DIR}/app/src/JSON/Frame.cpp
  ${CMAKE_SOURCE_DIR}/app/src/SerialStudio.cpp
)

# Test: Frame builder
add_serial_studio_test(test_frame_builder
  JSON/test_frame_builder.cpp
  ${CMAKE_SOURCE_DIR}/app/src/JSON/FrameBuilder.cpp
  ${CMAKE_SOURCE_DIR}/app/src/JSON/Frame.cpp
  ${CMAKE_SOURCE_DIR}/app/src/JSON/FrameParser.cpp
  ${CMAKE_SOURCE_DIR}/app/src/SerialStudio.cpp
)

# Test: Project model
add_serial_studio_test(test_project_model
  JSON/test_project_model.cpp
  ${CMAKE_SOURCE_DIR}/app/src/JSON/ProjectModel.cpp
  ${CMAKE_SOURCE_DIR}/app/src/JSON/Frame.cpp
  ${CMAKE_SOURCE_DIR}/app/src/SerialStudio.cpp
)

#-------------------------------------------------------------------------------
# Misc Module Tests
#-------------------------------------------------------------------------------

# Test: Utilities
add_serial_studio_test(test_utilities
  Misc/test_utilities.cpp
  ${CMAKE_SOURCE_DIR}/app/src/Misc/Utilities.cpp
)

# Test: Translator
add_serial_studio_test(test_translator
  Misc/test_translator.cpp
  ${CMAKE_SOURCE_DIR}/app/src/Misc/Translator.cpp
)

# Test: Theme manager
add_serial_studio_test(test_theme_manager
  Misc/test_theme_manager.cpp
  ${CMAKE_SOURCE_DIR}/app/src/Misc/ThemeManager.cpp
)

#-------------------------------------------------------------------------------
# DSP Module Tests
#-------------------------------------------------------------------------------

# Test: DSP processing (disabled - DSP.cpp not found)
# add_serial_studio_test(test_dsp
#   DSP/test_dsp.cpp
# )

#-------------------------------------------------------------------------------
# CSV Module Tests
#-------------------------------------------------------------------------------

# Test: CSV export
add_serial_studio_test(test_export
  CSV/test_export.cpp
  ${CMAKE_SOURCE_DIR}/app/src/CSV/Export.cpp
  ${CMAKE_SOURCE_DIR}/app/src/JSON/Frame.cpp
  ${CMAKE_SOURCE_DIR}/app/src/SerialStudio.cpp
)

#-------------------------------------------------------------------------------
# Licensing Module Tests
#-------------------------------------------------------------------------------

# Test: SimpleCrypt
add_serial_studio_test(test_simple_crypt
  Licensing/test_simple_crypt.cpp
  ${CMAKE_SOURCE_DIR}/app/src/Licensing/SimpleCrypt.cpp
)

# Test: MachineID
add_serial_studio_test(test_machine_id
  Licensing/test_machine_id.cpp
  ${CMAKE_SOURCE_DIR}/app/src/Licensing/MachineID.cpp
)

#-------------------------------------------------------------------------------
# Test summary
#-------------------------------------------------------------------------------

message(STATUS "Configured ${CMAKE_CURRENT_SOURCE_DIR} with ${CMAKE_BUILD_TYPE} build type")
