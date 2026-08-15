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
# For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial

#---------------------------------------------------------------------------------------------------
# C++ unit-test tier (spec 0032)
#---------------------------------------------------------------------------------------------------
#
# Only reachable through -DSS_BUILD_TESTS=ON, so a default configure never looks for Qt6::Test and
# never creates a target here.
#
# Each suite is a plain add_executable that links Qt6::Test plus the handful of production
# translation units it actually exercises. The application target is deliberately NOT a dependency:
# qt_add_executable(), the QML module, and the two-stage PGO/LTO flow stay untouched, and a suite
# links in seconds.
#
# This directory is a subdirectory of app/, so it inherits CMAKE_AUTOMOC, include_directories(src),
# and the directory-level sanitizer/optimization options. That inheritance is why the tests live
# here instead of under the repo-root tests/ tree (which sanitize-commit.py would never format).
#
# All five suites are registered up front, ahead of the source files landing, so parallel follow-up
# tasks (T3-T9) add a .cpp without contending on this file. ss_add_unit_test() skips any suite whose
# sources are not on disk yet and says so, so the tier configures and runs at every intermediate
# state.
#
#---------------------------------------------------------------------------------------------------

find_package(Qt6 REQUIRED COMPONENTS Test)

set(SS_APP_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(SS_UNIT_TEST_TARGETS "")
set(SS_PLATFORM_LIBS "")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(Qt6 REQUIRED COMPONENTS DBus)
  list(APPEND SS_PLATFORM_LIBS Qt6::DBus)
endif()

#---------------------------------------------------------------------------------------------------
# Suite registration helper
#---------------------------------------------------------------------------------------------------
#
# ss_add_unit_test(<name> SOURCES <files...> LIBS <targets...>)
#
# Creates the executable, links Qt6::Test plus LIBS, registers it with ctest under <name>, and adds
# it to the ss_unit_tests aggregate target.
#
# Every suite is one QObject in one .cpp whose last line is #include "<name>.moc" -- AUTOMOC writes
# the meta-object into that file and the translation unit has to pull it in itself. Omitting it
# surfaces as an unresolved vtable at link time, not as a missing-file error.
#
#---------------------------------------------------------------------------------------------------

function(ss_add_unit_test NAME)
  cmake_parse_arguments(SS_TEST "" "" "SOURCES;LIBS" ${ARGN})

  set(resolved_sources "")
  foreach(source IN LISTS SS_TEST_SOURCES)
    if(IS_ABSOLUTE "${source}")
      set(resolved "${source}")
    else()
      set(resolved "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
    endif()

    if(NOT EXISTS "${resolved}")
      message(STATUS "Unit tests: skipping ${NAME}, ${source} lands in a follow-up task")
      return()
    endif()

    list(APPEND resolved_sources "${resolved}")
  endforeach()

  add_executable(${NAME} ${resolved_sources})
  target_link_libraries(${NAME} PRIVATE Qt6::Test ${SS_TEST_LIBS})
  add_test(NAME ${NAME} COMMAND ${NAME})

  list(APPEND SS_UNIT_TEST_TARGETS ${NAME})
  set(SS_UNIT_TEST_TARGETS "${SS_UNIT_TEST_TARGETS}" PARENT_SCOPE)
endfunction()

#---------------------------------------------------------------------------------------------------
# Suites
#---------------------------------------------------------------------------------------------------
#
# Ordered by link cost. The link set of each suite is a decision, not an accident: growing one past
# what is listed here is the signal to move that coverage to the in-app test mode instead.
#
#---------------------------------------------------------------------------------------------------

ss_add_unit_test(tst_circular_buffer
  SOURCES tst_circular_buffer.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

ss_add_unit_test(tst_checksums
  SOURCES tst_checksums.cpp
          ${SS_APP_SRC}/IO/Checksum.cpp
  LIBS    Qt6::Core
)

ss_add_unit_test(tst_bar_bands
  SOURCES tst_bar_bands.cpp
  LIBS    Qt6::Core
)

ss_add_unit_test(tst_layout_patterns
  SOURCES tst_layout_patterns.cpp
          ${SS_APP_SRC}/UI/LayoutPatterns.cpp
  LIBS    Qt6::Core Qt6::Gui
)

ss_add_unit_test(tst_lua_migration
  SOURCES tst_lua_migration.cpp
          ${SS_APP_SRC}/DataModel/Scripting/LuaMigration.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

ss_add_unit_test(tst_frame_serialization
  SOURCES tst_frame_serialization.cpp
          ${SS_APP_SRC}/DataModel/Frame.cpp
          ${SS_APP_SRC}/DataModel/Generated/DatasetSerialization.cpp
          ${SS_APP_SRC}/DataModel/Project/PropertyValidators.cpp
          ${SS_APP_SRC}/SerialStudioFrameSupport.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Core5Compat
          Qt6::Gui
)

set_source_files_properties(
  ${CMAKE_CURRENT_SOURCE_DIR}/dsp_scalar_ref.cpp
  PROPERTIES COMPILE_DEFINITIONS "SS_SIMD_DISABLE;SS_DSP_NAMESPACE=DspSimdScalar"
)

ss_add_unit_test(tst_dsp_kernels
  SOURCES tst_dsp_kernels.cpp
          dsp_scalar_ref.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

ss_add_unit_test(tst_async_engine
  SOURCES tst_async_engine.cpp
          ${SS_APP_SRC}/Async/TaskTree.cpp
          ${SS_APP_SRC}/Async/RetryPolicy.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

ss_add_unit_test(tst_frame_delimiters
  SOURCES tst_frame_delimiters.cpp
          session_context_stub.cpp
          ${SS_APP_SRC}/IO/FrameReader.cpp
          ${SS_APP_SRC}/IO/Checksum.cpp
          ${SS_APP_SRC}/DataModel/NotificationCenter.cpp
          ${SS_APP_SRC}/Platform/AppPlatform.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
          Qt6::Qml
          Qt6::Widgets
          luajit
          ${SS_PLATFORM_LIBS}
)

#---------------------------------------------------------------------------------------------------
# Coverage-expansion suites (2026-07-27)
#---------------------------------------------------------------------------------------------------
#
# Registered up front like the originals: ss_add_unit_test() skips any suite whose .cpp has not
# landed yet, so the tier configures and runs at every intermediate state. Ordered by link cost.
#
#---------------------------------------------------------------------------------------------------

ss_add_unit_test(tst_json_validator
  SOURCES tst_json_validator.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

ss_add_unit_test(tst_ring_wrap
  SOURCES tst_ring_wrap.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

ss_add_unit_test(tst_parse_budget
  SOURCES tst_parse_budget.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

ss_add_unit_test(tst_enum_labels
  SOURCES tst_enum_labels.cpp
          ${SS_APP_SRC}/API/EnumLabels.cpp
          ${SS_APP_SRC}/UI/LayoutPatterns.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
)

ss_add_unit_test(tst_password_hash
  SOURCES tst_password_hash.cpp
          ${SS_APP_SRC}/Misc/PasswordHash.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Network
)

ss_add_unit_test(tst_legacy_icons
  SOURCES tst_legacy_icons.cpp
          ${SS_APP_SRC}/Misc/IconRegistryLegacy.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

ss_add_unit_test(tst_property_validators
  SOURCES tst_property_validators.cpp
          ${SS_APP_SRC}/DataModel/Project/PropertyValidators.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
)


ss_add_unit_test(tst_mirror_protocol
  SOURCES tst_mirror_protocol.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
)

ss_add_unit_test(tst_async_combinators
  SOURCES tst_async_combinators.cpp
          ${SS_APP_SRC}/Async/TaskTree.cpp
          ${SS_APP_SRC}/Async/RetryPolicy.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

ss_add_unit_test(tst_xymodem
  SOURCES tst_xymodem.cpp
          ${SS_APP_SRC}/IO/FileTransmission/Protocol.h
          ${SS_APP_SRC}/IO/FileTransmission/XMODEM.cpp
          ${SS_APP_SRC}/IO/FileTransmission/YMODEM.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

ss_add_unit_test(tst_zmodem
  SOURCES tst_zmodem.cpp
          ${SS_APP_SRC}/IO/FileTransmission/ZMODEM.cpp
          ${SS_APP_SRC}/IO/FileTransmission/Protocol.h
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

ss_add_unit_test(tst_tls_identity
  SOURCES tst_tls_identity.cpp
          ${SS_APP_SRC}/MQTT/TlsIdentity.cpp
  LIBS    Qt6::Core
          Qt6::Network
)
if(TARGET tst_tls_identity)
  target_compile_definitions(tst_tls_identity PRIVATE BUILD_COMMERCIAL)
endif()

ss_add_unit_test(tst_gsusb_protocol
  SOURCES tst_gsusb_protocol.cpp
  LIBS    Qt6::Core
)

ss_add_unit_test(tst_lua_compat
  SOURCES tst_lua_compat.cpp
          ${SS_APP_SRC}/DataModel/Scripting/LuaCompat.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          luajit
)

ss_add_unit_test(tst_js_watchdog
  SOURCES tst_js_watchdog.cpp
          ${SS_APP_SRC}/DataModel/Scripting/JsWatchdog.cpp
          ${SS_APP_SRC}/DataModel/Scripting/JsWatchdogThread.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Qml
)

ss_add_unit_test(tst_stream_worker
  SOURCES tst_stream_worker.cpp
          ${SS_APP_SRC}/IO/HAL_Driver.h
          ${SS_APP_SRC}/IO/StreamWorker.cpp
          ${SS_APP_SRC}/DataModel/Scripting/LuaCompat.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
          Qt6::Qml
          luajit
)

ss_add_unit_test(tst_frame_consumer
  SOURCES tst_frame_consumer.cpp
          ${SS_APP_SRC}/DataModel/FrameConsumer.cpp
          ${SS_APP_SRC}/DataModel/Frame.cpp
          ${SS_APP_SRC}/DataModel/Generated/DatasetSerialization.cpp
          ${SS_APP_SRC}/DataModel/Project/PropertyValidators.cpp
          ${SS_APP_SRC}/SerialStudioFrameSupport.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Core5Compat
          Qt6::Gui
)

ss_add_unit_test(tst_frame_support
  SOURCES tst_frame_support.cpp
          ${SS_APP_SRC}/DataModel/Frame.cpp
          ${SS_APP_SRC}/DataModel/Generated/DatasetSerialization.cpp
          ${SS_APP_SRC}/DataModel/Project/PropertyValidators.cpp
          ${SS_APP_SRC}/SerialStudioFrameSupport.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Core5Compat
          Qt6::Gui
)

ss_add_unit_test(tst_frame_json_legacy
  SOURCES tst_frame_json_legacy.cpp
          ${SS_APP_SRC}/DataModel/Frame.cpp
          ${SS_APP_SRC}/DataModel/Generated/DatasetSerialization.cpp
          ${SS_APP_SRC}/DataModel/Project/PropertyValidators.cpp
          ${SS_APP_SRC}/SerialStudioFrameSupport.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Core5Compat
          Qt6::Gui
)

ss_add_unit_test(tst_cframe_parser
  SOURCES tst_cframe_parser.cpp
          ${SS_APP_SRC}/DataModel/Scripting/CFrameParser.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/NativeTemplate.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/NativeTemplateRegistry.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextTemplates.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp
          ${SS_APP_SRC}/SerialStudioFrameSupport.cpp
          ${SS_APP_SRC}/Misc/Utilities.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Core5Compat
          Qt6::Gui
          Qt6::Svg
          Qt6::Widgets
)

ss_add_unit_test(tst_frame_reader_modes
  SOURCES tst_frame_reader_modes.cpp
          session_context_stub.cpp
          ${SS_APP_SRC}/IO/FrameReader.cpp
          ${SS_APP_SRC}/IO/Checksum.cpp
          ${SS_APP_SRC}/DataModel/NotificationCenter.cpp
          ${SS_APP_SRC}/Platform/AppPlatform.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
          Qt6::Qml
          Qt6::Widgets
          luajit
          ${SS_PLATFORM_LIBS}
)

#---------------------------------------------------------------------------------------------------
# Aggregate target
#---------------------------------------------------------------------------------------------------
#
# `cmake --build <dir> --target ss_unit_tests` builds every suite and nothing else, so CI can run
# the tier without ever building the application.
#
#---------------------------------------------------------------------------------------------------

add_custom_target(ss_unit_tests)

if(SS_UNIT_TEST_TARGETS)
  add_dependencies(ss_unit_tests ${SS_UNIT_TEST_TARGETS})
endif()

message(STATUS "Unit tests enabled: ${SS_UNIT_TEST_TARGETS}")
