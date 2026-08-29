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

#
# Ring semantics of IO::CircularBuffer, the SPSC store the frame reader parses out of. The
# buffer is header-only, so SSAssert.cpp is the entire production link set.
#
ss_add_unit_test(tst_circular_buffer
  SOURCES tst_circular_buffer.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# IO::checksum() and the algorithm registry it dispatches through; both live in Checksum.cpp,
# which is why the whole known-answer corpus checks in one TU.
#
ss_add_unit_test(tst_checksums
  SOURCES tst_checksums.cpp
          ${SS_APP_SRC}/IO/Checksum.cpp
  LIBS    Qt6::Core
)

#
# Header-only band lookup shared by Bar/Gauge/Meter/BarPanel (spec 0052); no production TU to
# link.
#
ss_add_unit_test(tst_bar_bands
  SOURCES tst_bar_bands.cpp
  LIBS    Qt6::Core
)

#
# Pure layout tiler (spec 0053). LayoutPatterns.cpp deliberately depends on nothing but QtCore
# and QtGui (QRect), which is what keeps this suite a seconds-long link and lets the whole
# pattern catalog be verified without a GUI.
#
ss_add_unit_test(tst_layout_patterns
  SOURCES tst_layout_patterns.cpp
          ${SS_APP_SRC}/UI/LayoutPatterns.cpp
  LIBS    Qt6::Core Qt6::Gui
)

#
# Source-to-source Lua rewriter behind the parser autofix. It is a lexer plus a precedence walker
# over QString and nothing else -- no Lua state, no engine, no project model -- so the suite links
# against QtCore alone and the whole operator matrix is checkable without a running app.
#
ss_add_unit_test(tst_lua_migration
  SOURCES tst_lua_migration.cpp
          ${SS_APP_SRC}/DataModel/Scripting/LuaMigration.cpp
          ${SS_APP_SRC}/SSAssert.cpp
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

#
# SIMD-versus-scalar equivalence for every kernel in DSPSimd.h. The scalar half comes from the
# dsp_scalar_ref.cpp build configured above, never from a second compilation of the test TU.
#
ss_add_unit_test(tst_dsp_kernels
  SOURCES tst_dsp_kernels.cpp
          dsp_scalar_ref.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Real-versus-complex KissFFT equivalence over the bins FFTPlot and Waterfall read. The widgets
# transform real input with kiss_fftr; this pins that its bins 0..N/2 match a complex transform
# of the same signal, which is the assumption that swap rests on. Vendored library only, so
# kissfft is the whole link set.
#
ss_add_unit_test(tst_real_fft
  SOURCES tst_real_fft.cpp
  LIBS    Qt6::Core kissfft
)

#
# Spec 0034 AC1-AC3 of the task-tree engine: sequencing, parallel completion, timeout, retry
# backoff, cancellation and error provenance. Fake steps and a virtual clock only, so TaskTree.cpp
# plus RetryPolicy.cpp is the whole link set and the suite never waits on real time.
#
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
# session_context_stub.cpp carries the link-only stand-ins that keep the carve-out honest: the
# SessionContext::current() reference inside NotificationCenter::instance() (linking the real
# SessionContext.cpp would drag the dtor closure described below) and SerialStudio::staticMetaObject
# for the Q_ENUM inlines in FrameReader's moc (compiling moc_SerialStudio.cpp instead would pull
# the whole invokable surface and with it SerialStudio.cpp).
#
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

#
# Misc::JsonValidator's security bounds -- file size, nesting depth, array size -- and the parse
# error plumbing around them. The validator is header-only, so nothing but SSAssert.cpp links.
#
ss_add_unit_test(tst_json_validator
  SOURCES tst_json_validator.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Wrap-adjacent CircularBuffer coverage that tst_circular_buffer does not reach: operator[],
# mid-scan resume on wrapped content, setCapacity() while wrapped, and peekRangeInto() at an
# offset that itself straddles the wrap point. Same header-only link set as that suite.
#
ss_add_unit_test(tst_ring_wrap
  SOURCES tst_ring_wrap.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# The spec-0051 fair-share parse-load governor, driven on a fabricated clock so the EWMA recovery
# window is deterministic. ParseBudget.h is header-only: SSAssert.cpp is the only TU needed.
#
ss_add_unit_test(tst_parse_budget
  SOURCES tst_parse_budget.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# The frame-pool policy is the regression guard for the slot-selection bug that let a large
# project's pool materialise a full frame copy per publish (2026-08-15). Pure bookkeeping: no
# session context, no Frame storage, so it links against QtCore alone.
#
ss_add_unit_test(tst_frame_pool_policy
  SOURCES tst_frame_pool_policy.cpp
          ${SS_APP_SRC}/DataModel/FramePoolPolicy.cpp
  LIBS    Qt6::Core
)

#
# The unified publication payload (spec 0055). DataBlock.h is header-only and pulls Frame.h only
# for the Frame that StructureSnapshot carries, so SSAssert.cpp is the entire production link set
# -- keeping it that cheap is deliberate: the pool's no-reallocation property is the thing worth
# checking on every build, and a suite that needed FrameBuilder could not run that often.
#
ss_add_unit_test(tst_data_block
  SOURCES tst_data_block.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# The cascading envelope pyramid behind the time-axis plots (spec 0057): every coarse cell must
# equal the brute-force min/max of the level-0 range it covers, before and after wrap, and level
# selection must follow seconds-per-pixel. DSP.h is header-only but its FixedQueue pins plot-scale
# buffers through Platform::AppPlatform, so AppPlatform.cpp (QtWidgets for QApplication) is the
# whole production link set -- the same carve-out tst_frame_delimiters documents.
#
#
# The 1-2-5 auto-scale ladder behind the plot Y range (spec 0058): header-only, SSAssert.cpp is
# the whole production link set.
#
ss_add_unit_test(tst_plot_autoscale
  SOURCES tst_plot_autoscale.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Compiled expression transforms (spec 0060): grammar, evaluator and slot table. The TU pulls
# SerialStudio.h only for the inline fast_float toDouble, so no SerialStudio*.cpp is needed;
# QtGui comes with the QColor include of that header.
#
ss_add_unit_test(tst_expression_transform
  SOURCES tst_expression_transform.cpp
          ${SS_APP_SRC}/DataModel/Scripting/ExpressionTransform.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
)

#
# Frame annotation layer (spec 0059): the model, filter proxy, payload extraction, CSV export and
# the JS decoder with carry-over. Console/Annotations.cpp reaches JsWatchdog, SSAssert and the
# ScriptTemplates manifest helpers (which localise names through Misc::Translator, hence Widgets);
# QtQml for the QJSEngine the decoder embeds.
#
ss_add_unit_test(tst_console_annotations
  SOURCES tst_console_annotations.cpp
          ${SS_APP_SRC}/Console/Annotations.cpp
          ${SS_APP_SRC}/DataModel/Scripting/JsWatchdog.cpp
          ${SS_APP_SRC}/DataModel/Scripting/JsWatchdogThread.cpp
          ${SS_APP_SRC}/DataModel/Scripting/ScriptTemplates.cpp
          ${SS_APP_SRC}/Misc/Translator.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
          Qt6::Qml
          Qt6::Widgets
)

ss_add_unit_test(tst_envelope_ring
  SOURCES tst_envelope_ring.cpp
          ${SS_APP_SRC}/Platform/AppPlatform.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
          Qt6::Widgets
          ${SS_PLATFORM_LIBS}
)

#
# Retained sweep segments of DSP::SweepEngine (spec 0061): same DSP.h link set as
# tst_envelope_ring.
#
ss_add_unit_test(tst_sweep_segments
  SOURCES tst_sweep_segments.cpp
          ${SS_APP_SRC}/Platform/AppPlatform.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
          Qt6::Widgets
          ${SS_PLATFORM_LIBS}
)

#
# Frozen pre-0055 session archives (spec 0055 R8). The build under test no longer writes the old
# layout, so the legacy read path cannot be checked against a fixture it produces itself; these are
# real 4.0.3 captures under tests/fixtures/sessions/. BlockReader.h is header-only, so Qt6::Sql plus
# SSAssert.cpp is the whole link set -- adding the session worker here is the move that would turn a
# seconds-long suite into an application link.
#
ss_add_unit_test(tst_sessions_legacy_archive
  SOURCES tst_sessions_legacy_archive.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core Qt6::Gui Qt6::Sql
)

if(TARGET tst_sessions_legacy_archive)
  target_compile_definitions(tst_sessions_legacy_archive
    PRIVATE SS_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../tests/fixtures/sessions")
endif()

#
# The two synthetic-refresh lanes (spec 0064). RepublishGate.h is header-only precisely so the
# lane-asymmetry rule can be pinned without FrameBuilder's link set: a masked dashboard refresh
# must never discharge the export lane's obligation, or a stream source silently empties every
# recording of the frame lane's table-fed datasets.
#
ss_add_unit_test(tst_republish_lanes
  SOURCES tst_republish_lanes.cpp
  LIBS    Qt6::Core
)

#
# The single-file sparse CSV body (spec 0055). SparseRowMerger.h was extracted from CSV::ExportWorker
# precisely so this can be checked without QFile, the workspace manager or a session: the suite
# supplies the two formatter stubs itself and links nothing but QtCore.
#
ss_add_unit_test(tst_csv_sparse_writer
  SOURCES tst_csv_sparse_writer.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# The session stream-block codec (spec 0054) is a header-only pair of free functions, so the suite
# needs no production TU at all -- which is the point of having extracted it: the encoder in
# Sessions::ExportWorker and the decoder in Sessions::Player cannot drift apart without this
# failing.
#
ss_add_unit_test(tst_stream_block_codec
  SOURCES tst_stream_block_codec.cpp
  LIBS    Qt6::Core
)

#
# The layout-pattern labels forward to UI::Layouts::patternId(), so the enum-label suite needs that
# TU in its link set; LayoutPatterns.cpp is header-only in its dependencies (QtCore/QtGui types).
#
ss_add_unit_test(tst_enum_labels
  SOURCES tst_enum_labels.cpp
          ${SS_APP_SRC}/API/EnumLabels.cpp
          ${SS_APP_SRC}/UI/LayoutPatterns.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
)

#
# The same TU compiled with BUILD_COMMERCIAL defined: EnumLabels.cpp's commercial BusType and
# DashboardWidget cases are unreachable in the GPL binary above, so their slug/label mappings would
# otherwise never be tested. The #ifdef BUILD_COMMERCIAL rows in tst_enum_labels.cpp swap the
# "falls back to unknown" gap assertions for the real commercial round-trip and uniqueness sweeps.
# The define is set on the target (like tst_opcua_wire) so the tier stays independent of the
# application's own configuration.
#
ss_add_unit_test(tst_enum_labels_commercial
  SOURCES tst_enum_labels.cpp
          ${SS_APP_SRC}/API/EnumLabels.cpp
          ${SS_APP_SRC}/UI/LayoutPatterns.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
)
if(TARGET tst_enum_labels_commercial)
  target_compile_definitions(tst_enum_labels_commercial PRIVATE BUILD_COMMERCIAL)
endif()

#
# PasswordHash.cpp includes QPasswordDigestor, which lives in QtNetwork (not QtCore) despite the
# rest of the file being Core-only; without Qt6::Network here the TU fails to find the header.
#
ss_add_unit_test(tst_password_hash
  SOURCES tst_password_hash.cpp
          ${SS_APP_SRC}/Misc/PasswordHash.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Network
)

#
# Misc::legacyIconPath() and the generated remap table it dispatches through -- what keeps a saved
# project's legacy qrc:/icons/... URL resolving. IconRegistryLegacy.cpp carries both, so the
# spec-0028 registry and the theme manager stay out of the link set.
#
ss_add_unit_test(tst_legacy_icons
  SOURCES tst_legacy_icons.cpp
          ${SS_APP_SRC}/Misc/IconRegistryLegacy.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Boundary sweep of the four ProjectModel-free validators in PropertyValidators.cpp. The hooks that
# do reach ProjectModel live in PropertyHooks.cpp and are deliberately not linked: that TU is what
# would pull the application in (same reasoning as tst_frame_serialization above).
#
ss_add_unit_test(tst_property_validators
  SOURCES tst_property_validators.cpp
          ${SS_APP_SRC}/DataModel/Project/PropertyValidators.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
)


#
# Byte-level contract of the spec-0040 mirror wire codec: the layout hash, the significant-digit
# rounding and the structure/snapshot/heartbeat/chunk encoders. MirrorProtocol.h is header-only, so
# a wire break is caught here without linking the API server or the dashboard.
#
ss_add_unit_test(tst_mirror_protocol
  SOURCES tst_mirror_protocol.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
)

#
# Second Async:: suite: the free-function combinator vocabulary, RetryPolicy's geometric schedule
# and step cap, and a nested sequential(parallel, timeout, retry) tree. Same two TUs as
# tst_async_engine, kept separate so a failure names which half of the engine broke.
#
ss_add_unit_test(tst_async_combinators
  SOURCES tst_async_combinators.cpp
          ${SS_APP_SRC}/Async/TaskTree.cpp
          ${SS_APP_SRC}/Async/RetryPolicy.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Protocol.h is a header-only Q_OBJECT (the abstract sender base): with no Protocol.cpp to pair it
# with, AUTOMOC only emits its meta-object if the header itself is a target source. Dropping it
# surfaces as an undefined vtable for IO::Protocols::Protocol, not as a missing-file error.
#
ss_add_unit_test(tst_xymodem
  SOURCES tst_xymodem.cpp
          ${SS_APP_SRC}/IO/FileTransmission/Protocol.h
          ${SS_APP_SRC}/IO/FileTransmission/XMODEM.cpp
          ${SS_APP_SRC}/IO/FileTransmission/YMODEM.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Protocol.h is listed as a source on purpose: IO::Protocols::Protocol is an abstract QObject with
# no .cpp of its own, so nothing in this link set would make AUTOMOC emit its meta-object and the
# base-class signals ZMODEM emits would stay undefined at link time.
#
ss_add_unit_test(tst_zmodem
  SOURCES tst_zmodem.cpp
          ${SS_APP_SRC}/IO/FileTransmission/ZMODEM.cpp
          ${SS_APP_SRC}/IO/FileTransmission/Protocol.h
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# TlsIdentity.{h,cpp} are commercial-only TUs guarded by BUILD_COMMERCIAL; the suite defines the
# macro itself so the tier stays independent of the application target's configuration.
#
ss_add_unit_test(tst_tls_identity
  SOURCES tst_tls_identity.cpp
          ${SS_APP_SRC}/MQTT/TlsIdentity.cpp
  LIBS    Qt6::Core
          Qt6::Network
)
if(TARGET tst_tls_identity)
  target_compile_definitions(tst_tls_identity PRIVATE BUILD_COMMERCIAL)
endif()

#
# SparkplugPayload.{h,cpp} (spec 0073) is the hand-rolled Sparkplug B v1.0 protobuf codec the MQTT
# session layer decodes broker traffic with. The bytes arrive from the network, so the suite is
# where the bounds checks and the fixed caps are proven; no protobuf runtime is linked and the TU
# compiles against Qt Core alone, so unlike the other commercial suites here it needs no
# BUILD_COMMERCIAL define.
#
ss_add_unit_test(tst_sparkplug_payload
  SOURCES tst_sparkplug_payload.cpp
          ${SS_APP_SRC}/IO/Drivers/MQTT/SparkplugPayload.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# SparkplugSession.{h,cpp} (spec 0073) is the birth-certificate state machine the codec above feeds:
# alias tables, the bounded pre-birth buffer, sequence gaps and the synthetic Online metric. It is
# QObject-free and header-only in its dependencies (OpcUaWire.h supplies the slot ceiling), so the
# link set is the codec plus SSAssert.cpp and no BUILD_COMMERCIAL define is needed.
#
ss_add_unit_test(tst_sparkplug_session
  SOURCES tst_sparkplug_session.cpp
          ${SS_APP_SRC}/IO/Drivers/MQTT/SparkplugSession.cpp
          ${SS_APP_SRC}/IO/Drivers/MQTT/SparkplugPayload.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# SparkplugPublisher.{h,cpp} (spec 0073 A1b) is the outbound Edge Node state machine: the alias
# registry, the seq/bdSeq counters the specification pins, and the birth/data/death payloads. It
# produces topic+payload pairs rather than performing I/O, so the suite drives the whole publishing
# lifecycle with no broker and round-trips every payload back through the decoder.
#
ss_add_unit_test(tst_sparkplug_publisher
  SOURCES tst_sparkplug_publisher.cpp
          ${SS_APP_SRC}/MQTT/SparkplugPublisher.cpp
          ${SS_APP_SRC}/IO/Drivers/MQTT/SparkplugPayload.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# S7Address.{h,cpp} (spec 0073) turns the absolute S7 spellings an engineer types ("DB5.DBD20",
# "MW10", "Q0.1") into an area, an offset and a read width. A misparsed offset reads the wrong
# memory on a live controller, so the grammar and every rejection reason are pinned here; the TU is
# Qt-Core-only and QObject-free, so no BUILD_COMMERCIAL define and no driver are linked.
#
ss_add_unit_test(tst_s7_address
  SOURCES tst_s7_address.cpp
          ${SS_APP_SRC}/IO/Drivers/S7Address.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# S7/IsoTsap.{h,cpp} (spec 0073) is the ISO-on-TCP transport the in-house S7comm client rides on:
# RFC 1006 TPKT framing, the ISO 8073 class-0 handshake that carries the rack and slot, and the
# data TPDUs a message is reassembled out of. It owns no socket -- the receive buffer is handed in
# on every decode -- so partial arrivals and hostile framing are driven here byte by byte.
#
ss_add_unit_test(tst_s7comm_isotsap
  SOURCES tst_s7comm_isotsap.cpp
          ${SS_APP_SRC}/IO/Drivers/S7/IsoTsap.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# S7/S7Pdu.{h,cpp} (spec 0073) is the S7comm application layer: the setup negotiation that fixes
# the message budget, the chunking that budget imposes on a variable list, the read-variable
# request bytes and the response walk. A misplaced octet reads the wrong memory on a live machine
# and a mis-sized chunk is silently truncated by the controller, so the whole service is pinned
# against golden bytes here; S7Address.cpp comes with it because the items are built from real
# parsed addresses rather than from hand-filled structs.
#
ss_add_unit_test(tst_s7comm_pdu
  SOURCES tst_s7comm_pdu.cpp
          ${SS_APP_SRC}/IO/Drivers/S7/S7Pdu.cpp
          ${SS_APP_SRC}/IO/Drivers/S7Address.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Apci.{h,cpp} (spec 0073) is the IEC 60870-5-104 transport layer: the I/S/U frame formats, the
# 15-bit sequence counters, the k/w windows and the t1/t2/t3 deadlines. It owns no QTimer and no
# socket -- every deadline is a query against a caller-supplied millisecond clock -- so the whole
# timing contract runs here against a fake clock. Qt-Core-only and QObject-free.
#
ss_add_unit_test(tst_iec104_apci
  SOURCES tst_iec104_apci.cpp
          ${SS_APP_SRC}/IO/Drivers/Iec104/Apci.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Asdu.{h,cpp} (spec 0073) is the IEC 60870-5-104 application layer. A misread element width
# shifts every point after it in the same object list, so each of the twelve monitor types, both
# SQ addressing modes and every quality descriptor is golden-decoded here; the TU is Qt-Core-only
# and links neither the driver nor a network stack.
#
ss_add_unit_test(tst_iec104_asdu
  SOURCES tst_iec104_asdu.cpp
          ${SS_APP_SRC}/IO/Drivers/Iec104/Asdu.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# LineProtocol.h (spec 0073) renders the InfluxDB sink's points. The escaping rules differ per
# line-protocol position and a wrong one is a silently rejected write, so the suite is where they
# are pinned; the header is deliberately Qt-Core-only and header-only, so nothing else is linked
# and no BUILD_COMMERCIAL define is needed.
#
ss_add_unit_test(tst_influx_lineprotocol
  SOURCES tst_influx_lineprotocol.cpp
  LIBS    Qt6::Core
)

#
# CanReassembly.{h,cpp} (spec 0073) joins J1939 transport-protocol and ISO 15765-2 fragments back
# into single messages. The fragments arrive from the bus, so the suite is where the fixed session
# caps, the timeout eviction and the drop-the-whole-session discipline are proven; the TU is
# QObject-free and compiles against Qt Core alone, so it needs no BUILD_COMMERCIAL define and links
# neither QtSerialBus nor the driver.
#
ss_add_unit_test(tst_can_reassembly
  SOURCES tst_can_reassembly.cpp
          ${SS_APP_SRC}/IO/Drivers/CANBus/CanReassembly.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Header-only suite (MirrorProtocol.h precedent): GsUsbProtocol.h carries the gs_usb wire
# vocabulary the CANable backend compiles against, so the solver and DLC tables test without
# linking libusb or QtSerialBus.
#
ss_add_unit_test(tst_gsusb_protocol
  SOURCES tst_gsusb_protocol.cpp
  LIBS    Qt6::Core
)

#
# OpcUaWire.h (spec 0066) is the delta-frame vocabulary the OPC UA driver encodes and the opcua
# native template decodes; the suite links the template registry so the latch semantics are tested
# through the real makeParser() path. Commercial-only TU, so the suite defines BUILD_COMMERCIAL.
#
ss_add_unit_test(tst_opcua_wire
  SOURCES tst_opcua_wire.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/NativeTemplate.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/NativeTemplateRegistry.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextTemplates.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/WireLatchTemplates.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/NativeTemplateSupport.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryRaw.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryHex.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryBase64.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryTlv.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryCobs.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinarySlip.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryUbx.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinarySirf.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryMavlink.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryNmea2000.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryRtcm.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryModbus.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryMessagePack.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextDelimited.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextFixedWidth.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextKeyValue.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextIniConfig.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextAtCommands.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextNmea0183.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextUrlEncoded.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextJsonData.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextXmlData.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextYamlData.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/MultiFrameBatchedSensor.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/MultiFrameTimeSeries2d.cpp
          ${SS_APP_SRC}/SerialStudioFrameSupport.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Core5Compat
          Qt6::Gui
)
if(TARGET tst_opcua_wire)
  target_compile_definitions(tst_opcua_wire PRIVATE BUILD_COMMERCIAL)
endif()

#
# OpcUaMarshal is the one seam between open62541's C vocabulary and the Qt types the driver
# speaks (spec 0067), so it is the place a wrong conversion would otherwise reach the dashboard
# unnoticed: a dropped source timestamp degrades silently to the local monotonic fallback, and a
# collapsed array latches only its first element. Needs no server.
#
# Guarded on the target: SS_BUILD_TESTS builds the stack on a GPL configure too, so this suite
# runs in CI's unit tier; an SS_ENABLE_OPCUA=OFF configure has no open62541 to link and skips it
# the same way a missing source does.
#
if(TARGET open62541)
  ss_add_unit_test(tst_opcua_marshal
    SOURCES tst_opcua_marshal.cpp
            ${SS_APP_SRC}/IO/Drivers/OpcUaMarshal.cpp
            ${SS_APP_SRC}/SSAssert.cpp
    LIBS    Qt6::Core
            open62541
  )
  if(TARGET tst_opcua_marshal)
    target_compile_definitions(tst_opcua_marshal PRIVATE BUILD_COMMERCIAL)
  endif()
endif()

#
# The Lua 5.1/5.2 compatibility shim plus the console and restricted-os sandbox. luajit is a real
# dependency here and not an accident: the contract under test is what the shim installs into a
# live lua_State, which no stub can stand in for.
#
ss_add_unit_test(tst_lua_compat
  SOURCES tst_lua_compat.cpp
          ${SS_APP_SRC}/DataModel/Scripting/LuaCompat.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          luajit
)

#
# JsWatchdog and the singleton worker thread that expires it. Qt6::Qml is the QJSEngine the
# watchdog interrupts, so it cannot be stubbed; the shutdown latch makes slot declaration order
# load-bearing inside the suite, which is a property of the .cpp and not of this link set.
#
ss_add_unit_test(tst_js_watchdog
  SOURCES tst_js_watchdog.cpp
          ${SS_APP_SRC}/DataModel/Scripting/JsWatchdog.cpp
          ${SS_APP_SRC}/DataModel/Scripting/JsWatchdogThread.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Qml
)

#
# HAL_Driver.h is header-only, so AUTOMOC's same-basename rule never reaches it and the driver's
# meta-object (openFinished, linkDropped, the vtable) goes undefined at link. Listing the header as
# a source is what gets moc_HAL_Driver.cpp generated for this suite. Qt6::Gui covers the QColor
# include that StreamWorker.cpp pulls in through FrameBuilder.h -> ... -> SerialStudio.h; the two
# FrameBuilder table-API injectors it references are stubbed inside tst_stream_worker.cpp so the
# suite links without the FrameBuilder dependency web. ExpressionTransform.cpp backs the
# expression-mode channels (spec 0060) that StreamProcessor compiles and evaluates.
#
ss_add_unit_test(tst_stream_worker
  SOURCES tst_stream_worker.cpp
          ${SS_APP_SRC}/IO/HAL_Driver.h
          ${SS_APP_SRC}/IO/StreamWorker.cpp
          ${SS_APP_SRC}/DataModel/Scripting/ExpressionTransform.cpp
          ${SS_APP_SRC}/DataModel/Scripting/LuaCompat.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
          Qt6::Qml
          luajit
)

#
# FrameConsumerWorkerBase's monotonic clock and DataModel::TimestampedFrame -- the source-owns-time
# rule stated as a test. Frame.cpp drags the generated serialization TU and the validators with it,
# which is why this link set matches tst_frame_serialization's.
#
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

#
# The pure SerialStudio helpers in SerialStudioFrameSupport.cpp, tested directly rather than through
# Frame.cpp. Core5Compat is not optional: the East-Asian encodings go through QTextCodec.
#
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

#
# Companion to tst_frame_serialization: that suite pins the happy-path round trip of every entity,
# this one the legacy, clamping and repair branches a saved project can still reach. Identical link
# set, because it is the same serializer under test from the other side.
#
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

#
# CFrameParser reports a load failure through Misc::Utilities::showMessageBox(), so Utilities.cpp is
# a link-time dependency even though every test in the suite loads with message boxes disabled.
#
ss_add_unit_test(tst_cframe_parser
  SOURCES tst_cframe_parser.cpp
          ${SS_APP_SRC}/DataModel/Scripting/CFrameParser.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/NativeTemplate.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/NativeTemplateRegistry.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextTemplates.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/WireLatchTemplates.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/NativeTemplateSupport.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryRaw.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryHex.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryBase64.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryTlv.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryCobs.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinarySlip.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryUbx.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinarySirf.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryMavlink.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryNmea2000.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryRtcm.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryModbus.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/BinaryMessagePack.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextDelimited.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextFixedWidth.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextKeyValue.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextIniConfig.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextAtCommands.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextNmea0183.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextUrlEncoded.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextJsonData.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextXmlData.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/TextYamlData.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/MultiFrameBatchedSensor.cpp
          ${SS_APP_SRC}/DataModel/Scripting/NativeTemplates/MultiFrameTimeSeries2d.cpp
          ${SS_APP_SRC}/SerialStudioFrameSupport.cpp
          ${SS_APP_SRC}/Misc/Utilities.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Core5Compat
          Qt6::Gui
          Qt6::Svg
          Qt6::Widgets
)

#
# Companion to tst_frame_delimiters: the operation modes and the reader paths that suite leaves
# alone. Same four-TU carve-out and the same session_context_stub.cpp, for the reasons spelled out
# above -- linking the real SessionContext.cpp would pull its dtor closure in.
#
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

#
# tst_dbc_importer (spec 0073 T19) covers the extended-multiplexing import: SG_MUL_VAL_ ranges in
# the generated Lua spec, the topological selector-before-dependent order a nested chain needs, and
# the unchanged numeric gate a simple-mux DBC still gets. It hits the identical wall, symbol group
# for symbol group, so it stays unregistered next to its sibling:
#
#   DBCImporter.cpp:158,168,222        Misc::Utilities::showMessageBox    -> Misc/Utilities.cpp
#   DBCImporter.cpp:210,232            ProjectModel::importCompleted,
#                                      importProjectFromJson, metaobject  -> ProjectModel.cpp + the
#                                                                            17 Project/*.cpp TUs
#   DBCImporter.cpp:373..1132          SerialStudio::groupWidgetId,
#                                      datasetWidgetId                    -> SerialStudio.cpp
#   SessionContext dtor closure        same nine-module closure as above
#
# AxisTicks.h and ImporterCommon.h are header-only, so they add nothing; the blocker is entirely
# the GUI/session surface (importDBC, showPreview, confirmImport) plus the stack SessionContext.
# Splitting those three slots into a second TU removes the first three groups but not the fourth,
# which is why the unblock is the same SessionContext test-double seam (M2-T16). Flip the comment
# once that lands. Until then the coverage that runs is the codegen assertion set in
# tests/scripts/test_cpp_regressions.py (R15/R17).
#
# ss_add_unit_test(tst_dbc_importer
#   SOURCES tst_dbc_importer.cpp
#           ${SS_APP_SRC}/DataModel/Importers/DBCImporter.cpp
#           ${SS_APP_SRC}/SessionContext.cpp
#           ${SS_APP_SRC}/SSAssert.cpp
#   LIBS    Qt6::Core
#           Qt6::Gui
#           Qt6::SerialBus
#           Qt6::Widgets
# )
#
# if(TARGET tst_dbc_importer)
#   target_compile_definitions(tst_dbc_importer
#     PRIVATE SS_DBC_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../tests/fixtures/dbc")
# endif()
#

#
# Stereo color policy behind the 3D plot's anaglyph mode (spec 0072). Plot3DStereo.cpp is
# deliberately free of Qt Quick and of every project type -- QColor and nothing else -- which is
# what lets the whole shipped palette be checked for both-eye visibility without a GUI, a scene
# graph or a running app. Commercial-only TU, so the suite defines BUILD_COMMERCIAL.
#
ss_add_unit_test(tst_plot3d_stereo
  SOURCES tst_plot3d_stereo.cpp
          ${SS_APP_SRC}/UI/Widgets/Plot3D/Plot3DStereo.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core Qt6::Gui
)

if(TARGET tst_plot3d_stereo)
  target_compile_definitions(tst_plot3d_stereo PRIVATE BUILD_COMMERCIAL)
endif()

#
# The terminal's ANSI color unit (spec 0070): the xterm-256 table, the theme-selected base
# palettes, and the SGR codes that select from them. Pure QColor, so QQuickItem never enters
# the link set.
#
ss_add_unit_test(tst_ansi_palette
  SOURCES tst_ansi_palette.cpp
          ${SS_APP_SRC}/UI/Widgets/Terminal/AnsiPalette.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Gui
)

#
# The terminal's escape-sequence parser (spec 0070), driven against a recording AnsiSink. The
# sink boundary is what keeps Terminal.cpp (and QQuickItem) out of this suite.
#
ss_add_unit_test(tst_ansi_state_machine
  SOURCES tst_ansi_state_machine.cpp
          ${SS_APP_SRC}/UI/Widgets/Terminal/AnsiStateMachine.cpp
  LIBS    Qt6::Core
)

#
# Search state of the terminal widget (spec 0070): query/match bookkeeping over caller-supplied
# lines, no widget in the link set.
#
ss_add_unit_test(tst_terminal_search
  SOURCES tst_terminal_search.cpp
          ${SS_APP_SRC}/UI/Widgets/Terminal/TerminalSearch.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# AI tool schema catalog (spec 0070): both tool rosters and every input-schema builder,
# checked for structural validity and pinned enums. Pure JSON, no app link.
#
ss_add_unit_test(tst_tool_schemas
  SOURCES tst_tool_schemas.cpp
          ${SS_APP_SRC}/AI/Tools/ToolSchemas.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# AI snapshot compaction (spec 0070): the pure result-reduction transforms split out of the
# registry-bound resolvers precisely so this suite can link QtCore alone.
#
ss_add_unit_test(tst_tool_compact
  SOURCES tst_tool_compact.cpp
          ${SS_APP_SRC}/AI/Tools/ToolCompact.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Provider-neutral wire JSON for the AI assistant (spec 0070): history/tool translation, the
# Chat-Completions body build, and HTTP error mapping shared by eight providers.
#
ss_add_unit_test(tst_provider_json
  SOURCES tst_provider_json.cpp
          ${SS_APP_SRC}/AI/Providers/ProviderJson.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# The <think>-tag stream splitter (spec 0070): carry/scan state across chunk boundaries,
# extracted from OpenAIReply so it tests without a network stack.
#
ss_add_unit_test(tst_think_tag_splitter
  SOURCES tst_think_tag_splitter.cpp
          ${SS_APP_SRC}/AI/Providers/ThinkTagSplitter.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Console text formatting (spec 0070): the normalize/timestamp loop, control-char filter and
# hex dump shared by the two console append paths. QtCore only; the SIMD dots inline from
# DSPSimd.h.
#
ss_add_unit_test(tst_console_text_format
  SOURCES tst_console_text_format.cpp
          ${SS_APP_SRC}/Console/TextFormat.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Waterfall view/window state (spec 0070): pan/zoom clamps, dB window, axis-window math shared
# by the axis painter, cursor readout and source-rect mapping. Qt-value-type only.
#
ss_add_unit_test(tst_waterfall_viewstate
  SOURCES tst_waterfall_viewstate.cpp
          ${SS_APP_SRC}/UI/Widgets/Waterfall/WaterfallViewState.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Waterfall tick ladders and label formatting (spec 0070), now free functions with a narrow
# header.
#
ss_add_unit_test(tst_waterfall_ticks
  SOURCES tst_waterfall_ticks.cpp
          ${SS_APP_SRC}/UI/Widgets/Waterfall/WaterfallTicks.cpp
  LIBS    Qt6::Core
)

#
# The history-repair passes AI::Conversation runs before every provider request: orphan
# tool_result removal, missing-result synthesis, tool-pair reconciliation across malformed
# orderings, transcript aging with its exemption list, and fresh-turn pruning. An unpaired
# tool_use makes the provider reject the whole request, so these are integrity gates, not
# formatting. HistorySurgery.cpp logs through the AI category, hence Logging.cpp.
#
ss_add_unit_test(tst_conversation_history
  SOURCES tst_conversation_history.cpp
          ${SS_APP_SRC}/AI/Conversation/HistorySurgery.cpp
          ${SS_APP_SRC}/AI/Logging.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# The context-window budgeter: token estimation, the reservation arithmetic, and the rule that
# a trim may only cut at a fresh user turn. Pulls HistorySurgery.cpp for the boundary scan.
#
ss_add_unit_test(tst_conversation_budget
  SOURCES tst_conversation_budget.cpp
          ${SS_APP_SRC}/AI/Conversation/TokenBudget.cpp
          ${SS_APP_SRC}/AI/Conversation/HistorySurgery.cpp
          ${SS_APP_SRC}/AI/Logging.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# The meta-tool surface advertised to the model each turn: schema shapes, the 11-tool roster
# and its order, the injected recipe/skill enums, and the two flags that reshape the curated
# command list. Pure JSON building with no ContextBuilder dependency, so QtCore alone.
#
ss_add_unit_test(tst_conversation_metatools
  SOURCES tst_conversation_metatools.cpp
          ${SS_APP_SRC}/AI/Conversation/MetaToolCatalog.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# The API socket's credential and reception guards (spec 0070). AuthPrimitives.cpp is the pure
# half of ServerAuth (QtCore only) so these KATs need neither the consent prompt's QtWidgets
# dependency nor a live server; ClientReception drives its ReceptionHost stub, so QTcpSocket is
# only ever an opaque handle and Qt6::Network is here for that type alone.
#
ss_add_unit_test(tst_server_auth
  SOURCES tst_server_auth.cpp
          ${SS_APP_SRC}/API/Server/AuthPrimitives.cpp
          ${SS_APP_SRC}/API/Server/ClientReception.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Network
)

#
# Modbus project generation (spec 0070): the register-group -> project/parser builder that used
# to live inside the driver. Link set mirrors tst_frame_serialization.
#
ss_add_unit_test(tst_modbus_generation
  SOURCES tst_modbus_generation.cpp
          ${SS_APP_SRC}/IO/Drivers/Modbus/ModbusProjectGenerator.cpp
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
# Serial-port identity scoring shared by UART and Modbus (spec 0070): the 100/50/10/5 ladder
# that reattaches a saved device to a re-enumerated port.
#
ss_add_unit_test(tst_serial_port_identity
  SOURCES tst_serial_port_identity.cpp
          ${SS_APP_SRC}/IO/Drivers/SerialPortIdentity.cpp
  LIBS    Qt6::Core
          Qt6::SerialPort
)

#
# OPC UA endpoint selection (spec 0070): pure policy ranking/deprecation/token acceptance over
# synthetic endpoint descriptors. QtCore only -- no open62541.
#
ss_add_unit_test(tst_opcua_endpoint_selection
  SOURCES tst_opcua_endpoint_selection.cpp
          ${SS_APP_SRC}/IO/Drivers/OpcUa/OpcUaEndpointSelection.cpp
  LIBS    Qt6::Core
)

#
# The MQTT publisher's dashboard payload transform (spec 0070): RFC 4180 escaping, the column
# label the CSV header and the Sparkplug metric registry share, header/row assembly, and the
# block->frame materialisation both dashboard modes feed on. CsvExpansion is QObject-free, so
# SSAssert.cpp is the whole production link set; the TU is commercial-only, so the suite
# defines the macro itself.
#
ss_add_unit_test(tst_mqtt_csv_expansion
  SOURCES tst_mqtt_csv_expansion.cpp
          ${SS_APP_SRC}/MQTT/CsvExpansion.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)
if(TARGET tst_mqtt_csv_expansion)
  target_compile_definitions(tst_mqtt_csv_expansion PRIVATE BUILD_COMMERCIAL)
endif()

#
# Window geometry math (spec 0070): resize-edge detection with the shipping 8 px margin,
# resize/constrain/cascade KATs. Stateless free functions; Qt6::Quick supplies QQuickItem.
#
ss_add_unit_test(tst_window_geometry
  SOURCES tst_window_geometry.cpp
          ${SS_APP_SRC}/UI/WindowManager/WindowGeometry.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core Qt6::Gui Qt6::Quick
)

#
# Snap overlay publication (spec 0070): gesture publish/dedupe/clear and guide serialization,
# driven through QSignalSpy. SnapOverlay takes no Dashboard reference precisely so this suite
# links without the app.
#
ss_add_unit_test(tst_snap_overlay
  SOURCES tst_snap_overlay.cpp
          ${SS_APP_SRC}/UI/WindowManager/SnapOverlay.cpp
          ${SS_APP_SRC}/UI/WindowManager/WindowGeometry.cpp
          ${SS_APP_SRC}/UI/SnapGuides.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core Qt6::Gui Qt6::Quick
)

#
# USB hex-request parsing (spec 0070): the control-request byte/uint parsers and the transfer
# status labels, mirrored libusb-free so the suite links in a GPL configure too.
#
ss_add_unit_test(tst_usb_hex_parsing
  SOURCES tst_usb_hex_parsing.cpp
          ${SS_APP_SRC}/IO/Drivers/USB/UsbHex.cpp
  LIBS    Qt6::Core
)

#
# Widget-extension manifest validation (spec 0070): the version-range grammar and every
# rejection code the spec-0038 trust model rests on, injected host version + reserved ids so
# the parser links QtCore-only.
#
ss_add_unit_test(tst_widget_manifest
  SOURCES tst_widget_manifest.cpp
          ${SS_APP_SRC}/UI/WidgetManifestParser.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# The language table (spec 0070): one row per shipped language replacing four parallel
# switches across Translator and ThemeManager.
#
ss_add_unit_test(tst_language_table
  SOURCES tst_language_table.cpp
          ${SS_APP_SRC}/Misc/LanguageTable.cpp
  LIBS    Qt6::Core
)

#
# The theme catalog (spec 0070): built-in/user theme scanning and localized title rows,
# separated from the palette build so it loads without a GUI.
#
ss_add_unit_test(tst_theme_catalog
  SOURCES tst_theme_catalog.cpp
          ${SS_APP_SRC}/Misc/ThemeCatalog.cpp
          ${SS_APP_SRC}/Misc/LanguageTable.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Extension catalog helpers (spec 0070): filter/platform/path-safety free functions split out
# of ExtensionManager; the path-traversal KATs pin the install trust boundary.
#
ss_add_unit_test(tst_extension_catalog
  SOURCES tst_extension_catalog.cpp
          ${SS_APP_SRC}/Misc/Extensions/ExtensionCatalog.cpp
  LIBS    Qt6::Core
)

#
# Reply capture for scripted writes (spec 0070): arm/record/poll/disarm over per-device
# buffers, the mutex moved as one unit with what it guards. QtCore only.
#
ss_add_unit_test(tst_connection_reply_capture
  SOURCES tst_connection_reply_capture.cpp
          ${SS_APP_SRC}/IO/ConnectionManager/ReplyCapture.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Replay CSV row codec (spec 0070): the QString reader and its byte-view twin held to the
# same rows -- an export/import contract that had no coverage before.
#
ss_add_unit_test(tst_replay_row_codec
  SOURCES tst_replay_row_codec.cpp
          ${SS_APP_SRC}/DataModel/Scripting/ReplayRowCodec.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# The mixed scalar/vector frame unzip both script engines now share (spec 0070);
# header-inline, so no production TU to link.
#
ss_add_unit_test(tst_script_frame_shaping
  SOURCES tst_script_frame_shaping.cpp
  LIBS    Qt6::Core
)

#
# The .proto lexer/parser (spec 0070): the class extraction that finally unblocked importer
# unit coverage -- the parser no longer reaches Utilities/ProjectModel, so it links QtCore
# alone (the full-importer suites above stay commented for the reasons they document).
#
ss_add_unit_test(tst_proto_parser
  SOURCES tst_proto_parser.cpp
          ${SS_APP_SRC}/DataModel/Importers/ProtoLexer.cpp
          ${SS_APP_SRC}/DataModel/Importers/ProtoParser.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# The code formatter behind every embedded editor (spec 0070): brace/Lua-block depth,
# hanging indents, string/comment immunity, idempotence. Pure statics, QtCore only.
#
ss_add_unit_test(tst_code_formatter
  SOURCES tst_code_formatter.cpp
          ${SS_APP_SRC}/DataModel/Editors/CodeFormatter.cpp
  LIBS    Qt6::Core
)

#
# CSV row syntax (spec 0070): separator sniffing, top-level splitting under quotes, and the
# timestamp unit-scale table. Pure functions + SSAssert only.
#
ss_add_unit_test(tst_csv_rows
  SOURCES tst_csv_rows.cpp
          ${SS_APP_SRC}/CSV/Player/RowSyntax.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# CSV multi-source replay mapping (spec 0070): column->source bits, the 8-source ceiling,
# sparse backfill semantics.
#
ss_add_unit_test(tst_csv_multisource
  SOURCES tst_csv_multisource.cpp
          ${SS_APP_SRC}/CSV/Player/MultiSourceMap.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# CSV row codec (spec 0070): per-mode cell exclusion, the column-mapping inverse and the
# QuickPlot payload branches, over the shared replay row splitter.
#
ss_add_unit_test(tst_csv_row_codec
  SOURCES tst_csv_row_codec.cpp
          ${SS_APP_SRC}/CSV/Player/RowCodec.cpp
          ${SS_APP_SRC}/CSV/Player/RowSyntax.cpp
          ${SS_APP_SRC}/DataModel/Scripting/ReplayRowCodec.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
)

#
# Historian replay alignment (spec 0070): column->project mapping and stream timeline merge,
# ProjectModel flattened out so the suite links QtCore alone. Commercial-only TU.
#
ss_add_unit_test(tst_session_alignment
  SOURCES tst_session_alignment.cpp
          ${SS_APP_SRC}/Sessions/Player/ReplayAlignment.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)
if(TARGET tst_session_alignment)
  target_compile_definitions(tst_session_alignment PRIVATE BUILD_COMMERCIAL)
endif()

#
# Historian frame synthesis (spec 0070): the DB-free half of replay -- row selection, window
# scatter, gap fill, and the steady replay clock.
#
ss_add_unit_test(tst_session_synthesis
  SOURCES tst_session_synthesis.cpp
          ${SS_APP_SRC}/Sessions/Player/ReplayFrameValues.cpp
          ${SS_APP_SRC}/Sessions/Player/ReplayClock.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core Qt6::Gui Qt6::Sql
)
if(TARGET tst_session_synthesis)
  target_compile_definitions(tst_session_synthesis PRIVATE BUILD_COMMERCIAL)
endif()

#
# Dashboard view/session state (spec 0070): both view-state scopes, layout clamps, the
# persistence gate and its deliberate autoHideToolbar exemption. Qml supplies QJSValue
# normalization.
#
ss_add_unit_test(tst_dashboard_viewstate
  SOURCES tst_dashboard_viewstate.cpp
          ${SS_APP_SRC}/UI/Dashboard/DashboardViewState.cpp
  LIBS    Qt6::Core Qt6::Qml
)

#
# Project Editor navigation history (spec 0070): ring arithmetic, dedup-at-cursor, the
# 128-entry cap, resolvable walks. Plain value type, QtCore only.
#
ss_add_unit_test(tst_project_nav_history
  SOURCES tst_project_nav_history.cpp
          ${SS_APP_SRC}/DataModel/Project/ProjectNavHistory.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# Project folder-tree templates (spec 0070): existence, parentage, cycle termination and
# display paths, driven through a local POD -- no ProjectModel in the link set.
#
ss_add_unit_test(tst_project_folders
  SOURCES tst_project_folders.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Gui
)

#
# Canvas widget enum mappers (spec 0070): the five Canvas2D string<->Qt maps, header-inline
# so the per-frame call sites keep full inlinability. Commercial-only headers.
#
ss_add_unit_test(tst_painter_enums
  SOURCES tst_painter_enums.cpp
  LIBS    Qt6::Core
          Qt6::Gui
)
if(TARGET tst_painter_enums)
  target_compile_definitions(tst_painter_enums PRIVATE BUILD_COMMERCIAL)
endif()

#
# Canvas widget path geometry (spec 0070): sweep normalization, arc/ellipse/roundRect/arcTo
# construction, including the degenerate fallbacks. Header-inline, commercial-only.
#
ss_add_unit_test(tst_painter_geometry
  SOURCES tst_painter_geometry.cpp
  LIBS    Qt6::Core
          Qt6::Gui
)
if(TARGET tst_painter_geometry)
  target_compile_definitions(tst_painter_geometry PRIVATE BUILD_COMMERCIAL)
endif()

#
# CLI spec parsers (spec 0070): the pure text->value half of bus configuration (int ranges,
# Modbus TCP addresses and register specs, serial index tables). QtCore only.
#
ss_add_unit_test(tst_cli_spec_parsers
  SOURCES tst_cli_spec_parsers.cpp
          ${SS_APP_SRC}/Misc/CLI/CliSpecParsers.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# The io.getLatestFrame capture lane (spec 0070): SPSC mirror ring, arming latch, sequence
# dedup and the 1 Hz parse-load mirror, driven GUI-side with a stub builder thread.
# HAL_Driver.h is listed for AUTOMOC (header-only Q_OBJECT).
#
ss_add_unit_test(tst_latest_frame_tap
  SOURCES tst_latest_frame_tap.cpp
          ${SS_APP_SRC}/DataModel/FrameBuilder/LatestFrameTap.cpp
          ${SS_APP_SRC}/IO/HAL_Driver.h
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
)

#
# The GUI table-snapshot channel (spec 0070): pool + ring moved whole; the suite stubs the
# two PipelineHost statics DataTable.cpp reaches (same precedent as tst_stream_worker).
#
ss_add_unit_test(tst_table_snapshot_channel
  SOURCES tst_table_snapshot_channel.cpp
          ${SS_APP_SRC}/DataModel/FrameBuilder/TableSnapshotChannel.cpp
          ${SS_APP_SRC}/DataModel/DataTable.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core Qt6::Gui Qt6::Qml
)

#
# Chat digest builders (spec 0070): handoff digest window, title selection, stale-card
# downgrade -- pure over QVariantList, with the Redactor scrub in the link set.
#
ss_add_unit_test(tst_chat_digest
  SOURCES tst_chat_digest.cpp
          ${SS_APP_SRC}/AI/Conversation/ChatDigest.cpp
          ${SS_APP_SRC}/AI/Redactor.cpp
  LIBS    Qt6::Core
)

#
# The terminal's line store (spec 0070): wrapping at the column count, the scrollback trim,
# duplicate collapsing with and without a timestamp prefix, and cursor clamping -- no
# QQuickItem in the link set. AnsiPalette.cpp comes along because the buffer writes the
# SGR-selected colors into each cell.
#
ss_add_unit_test(tst_terminal_buffer
  SOURCES tst_terminal_buffer.cpp
          ${SS_APP_SRC}/UI/Widgets/Terminal/TerminalBuffer.cpp
          ${SS_APP_SRC}/UI/Widgets/Terminal/AnsiPalette.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Gui
)

#
# OPC UA project generation (spec 0070): tag-list -> project-JSON translation. Pure, same
# link tier as tst_modbus_generation; commercial bus enum, so the target defines the macro.
#
ss_add_unit_test(tst_opcua_project_builder
  SOURCES tst_opcua_project_builder.cpp
          ${SS_APP_SRC}/IO/Drivers/OpcUa/OpcUaProjectBuilder.cpp
          ${SS_APP_SRC}/DataModel/Frame.cpp
          ${SS_APP_SRC}/DataModel/Generated/DatasetSerialization.cpp
          ${SS_APP_SRC}/DataModel/Project/PropertyValidators.cpp
          ${SS_APP_SRC}/SerialStudioFrameSupport.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
          Qt6::Core5Compat
          Qt6::Gui
)
if(TARGET tst_opcua_project_builder)
  target_compile_definitions(tst_opcua_project_builder PRIVATE BUILD_COMMERCIAL)
endif()

#
# The replay seek engine (spec 0070): seek keys, series enumeration, ring snapshot/restore
# including the same-uniqueId-two-sources case that once crashed replay. QtCore + SSAssert.
#
ss_add_unit_test(tst_replay_seek_engine
  SOURCES tst_replay_seek_engine.cpp
          ${SS_APP_SRC}/UI/Dashboard/ReplaySeekEngine.cpp
          ${SS_APP_SRC}/SSAssert.cpp
  LIBS    Qt6::Core
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
