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

#
# AppPlatform.cpp reaches QDBusConnection under Q_OS_LINUX (same gate as the application target in
# app/CMakeLists.txt), so any suite that compiles it needs the QtDBus include path on Linux.
#
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

#
# SerialStudioFrameSupport.cpp holds the four SerialStudio statics Frame.cpp reaches (commercialCfg,
# resolveEscapeSequences, hexToBytes, encodeText). They live apart from SerialStudio.cpp so this
# suite links without the icon registry, theme manager and player singletons. Core5Compat comes with
# them: the East-Asian encodings go through QTextCodec.
#
# serialize(Dataset) / read(Dataset&, ...) are generated (spec 0036), so DatasetSerialization.cpp is
# mandatory here. It calls two PropertyHooks validators, and PropertyHooks.cpp reaches
# ProjectModel::groups() / datasetCount(), which would drag the whole application in; the
# ProjectModel-free validators therefore live in PropertyValidators.cpp and that is the TU this
# suite links. Adding PropertyHooks.cpp instead is the move that breaks the seconds-long link.
#
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

#
# dsp_scalar_ref.cpp is the *scalar* build of the same header the test TU sees vectorized. The two
# builds must not share symbols: DSP:: kernels are inline with external linkage, so without a
# distinct namespace the linker folds them and the "scalar" wrapper silently calls the vector code,
# producing a green test that checks nothing. SS_DSP_NAMESPACE is what keeps them distinct, and it
# applies to this one source file only.
#
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

#
# The carve-out is these four TUs and no more. AppPlatform.cpp is mandatory (FrameReader's ctor/dtor
# call lockMemoryResident/unlockMemoryResident) and now carries only OS-level helpers:
# FileOpenEventFilter, the one class there that reached AppState and the project model, lives in its
# own TU that this suite deliberately does not link.
#
ss_add_unit_test(tst_frame_delimiters
  SOURCES tst_frame_delimiters.cpp
          ${SS_APP_SRC}/IO/FrameReader.cpp
          ${SS_APP_SRC}/IO/Checksum.cpp
          ${SS_APP_SRC}/DataModel/NotificationCenter.cpp
          ${SS_APP_SRC}/Platform/AppPlatform.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
          Qt6::Qml
          Qt6::Widgets
          lua54
          ${SS_PLATFORM_LIBS}
)

#
# tst_proto_importer (spec 0039 AC3) is written and lands with this file, but it stays unregistered:
# the minimal link set that resolves is the application, which is the one thing a suite here may not
# carry. Four undefined-symbol groups force it, and none of them is reachable from the pure
# generation path the test actually calls:
#
#   ProtoImporter.cpp:938,948,974,1014  Misc::Utilities::showMessageBox   -> Misc/Utilities.cpp
#   ProtoImporter.cpp:1008,1024         ProjectModel::importCompleted,
#                                       importProjectFromJson, metaobject  -> ProjectModel.cpp + the
#                                                                             17 Project/*.cpp TUs
#   ProtoImporter.cpp:1086..1245        SerialStudio::groupWidgetId        -> SerialStudio.cpp
#   SessionContext dtor closure         ~SessionContext destroys eight unique_ptr members ->
#                                       vtables of the six implicit-dtor modules (each in its
#                                       moc_*.cpp) + ~ConnectionManager + ~NotificationCenter
#                                       and everything their dtors reach
#
# Registering it anyway would configure and then fail at link, taking ss_unit_tests and CI with it.
# M2 ownership did NOT unblock this (2026-07-25 analysis): a ProtoImporter TU split removes the
# first three groups, but the stack SessionContext the suite builds still drags the dtor closure
# above, and the shared detail:: proto parser (also used by showPreview) resists a clean cut.
# The honest unblock is a SessionContext test-double seam -- see the M2-T16 status note in
# doc/claude/specs/0039-session-context/m2-plan.md. Until then the source is compiled by nothing
# and AC3 stays gated -- recorded as such in doc/claude/specs/0039-session-context/spec.md.
#
# ss_add_unit_test(tst_proto_importer
#   SOURCES tst_proto_importer.cpp
#           ${SS_APP_SRC}/DataModel/Importers/ProtoImporter.cpp
#           ${SS_APP_SRC}/SessionContext.cpp
#   LIBS    Qt6::Core
#           Qt6::Gui
#           Qt6::Widgets
# )
#

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
