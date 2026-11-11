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
# SerialStudioStorage -- the recording and playback layer (spec 0076)
#---------------------------------------------------------------------------------------------------
#
# Everything that turns a live block into a file and a file back into blocks: the CSV exporter and
# player, the MDF4 pair, the Historian session database, and the InfluxDB line-protocol sink.
#
# Headers ride along with their sources so AUTOMOC sees the Q_OBJECT classes; the same entries were
# removed from the executable's HEADERS list, or moc would run twice and the link would fail on
# duplicate staticMetaObject symbols.
#
# A strict layer since spec 0077: it includes only itself and Core, Protocols and Pipeline, and the include
# roots and links below say exactly that. scripts/layer-verify.py fails on an include, an
# include root or a link outside that graph.
#
#---------------------------------------------------------------------------------------------------

qt_add_library(
  SerialStudioStorage STATIC

  CSV/Player.cpp
  CSV/Player/FileIndexer.cpp
  CSV/Player/MultiSourceMap.cpp
  CSV/Player/RowCodec.cpp
  CSV/Player/RowSyntax.cpp
  CSV/PlayerLoaderWorker.cpp
  CSV/Export.cpp
  MDF4/Player.cpp
  MDF4/PlayerLoaderWorker.cpp
  MDF4/Export.cpp
  CSV/SparseRowMerger.h
  CSV/Export.h
  CSV/Player.h
  Replay/PlayerState.h
  Replay/PlayerState.cpp
  Replay/LinkGate.h
  Replay/LinkGate.cpp
  CSV/Player/FileIndexer.h
  CSV/Player/MultiSourceMap.h
  CSV/Player/RowCodec.h
  CSV/Player/RowSyntax.h
  CSV/PlayerLoaderWorker.h
  MDF4/Player.h
  MDF4/PlayerLoaderWorker.h
  MDF4/Export.h
)

add_library(SerialStudio::Storage ALIAS SerialStudioStorage)

#---------------------------------------------------------------------------------------------------
# Pro sinks: Historian sessions, the InfluxDB writer and the MQTT publisher stack
#---------------------------------------------------------------------------------------------------

if(BUILD_COMMERCIAL)
  target_sources(
    SerialStudioStorage PRIVATE

    MQTT/Publisher.h
    MQTT/PublisherWorker.h
    MQTT/BrokerOptions.h
    MQTT/CsvExpansion.h
    MQTT/NotificationPayload.h
    MQTT/PublisherScript.h
    MQTT/SparkplugPublisher.h
    MQTT/Publisher.cpp
    MQTT/PublisherWorker.cpp
    MQTT/BrokerOptions.cpp
    MQTT/CsvExpansion.cpp
    MQTT/NotificationPayload.cpp
    MQTT/PublisherScript.cpp
    MQTT/SparkplugPublisher.cpp
    InfluxDB/LineProtocol.h
    InfluxDB/Export.h
    Sessions/BlockFingerprint.h
    Sessions/DatabaseManager.h
    Sessions/DatabaseManager/DatabaseSchema.h
    Sessions/DatabaseManager/SessionExporter.h
    Sessions/DatabaseManager/ReproducibilityVerifier.h
    Sessions/DatabaseWorker.h
    Sessions/Export.h
    Sessions/HtmlReport.h
    Sessions/Player.h
    Sessions/ReportOptionsModel.h
    Sessions/Player/ReplayAlignment.h
    Sessions/Player/ReplayClock.h
    Sessions/Player/ReplayFrameValues.h
    Sessions/Player/ReplaySynthesis.h
    Sessions/Player/SessionDbReader.h
    Sessions/Player/PreSessionState.h
    Sessions/PlayerLoaderWorker.h
    Sessions/ReportData.h
    Sessions/BlockReader.h
    Sessions/StreamBlockCodec.h
    Sessions/Verifier.h
    InfluxDB/Export.cpp
    Sessions/BlockFingerprint.cpp
    Sessions/DatabaseManager.cpp
    Sessions/DatabaseManager/DatabaseSchema.cpp
    Sessions/DatabaseManager/SessionExporter.cpp
    Sessions/DatabaseManager/ReproducibilityVerifier.cpp
    Sessions/DatabaseWorker.cpp
    Sessions/Export.cpp
    Sessions/HtmlReport.cpp
    Sessions/Player.cpp
    Sessions/ReportOptionsModel.cpp
    Sessions/Player/ReplayAlignment.cpp
    Sessions/Player/ReplayClock.cpp
    Sessions/Player/ReplayFrameValues.cpp
    Sessions/Player/ReplaySynthesis.cpp
    Sessions/Player/SessionDbReader.cpp
    Sessions/Player/PreSessionState.cpp
    Sessions/PlayerLoaderWorker.cpp
    Sessions/ReportData.cpp
    Sessions/Verifier.cpp
    Sessions/VerifierRegression.cpp
  )
endif()

#---------------------------------------------------------------------------------------------------
# Include roots -- the layer graph
#---------------------------------------------------------------------------------------------------
#
# This partition keeps its own relative include root plus the roots of the layers below it that
# it may include (spec 0077). Nothing from app/src and nothing sideways: scripts/layer-verify.py
# fails on an include outside the graph and on a root or a link this list does not justify. The
# commercial token's generated guards header arrives through SerialStudio::Core.
#
#---------------------------------------------------------------------------------------------------

target_include_directories(
  SerialStudioStorage PUBLIC

  ${CMAKE_SOURCE_DIR}/core/Storage
  ${CMAKE_SOURCE_DIR}/core/Pipeline
)

target_link_libraries(
  SerialStudioStorage PUBLIC

  ${QT_LIBS}

  SerialStudio::Core
  SerialStudio::Protocols
  SerialStudio::Pipeline

  mdf
  luajit
  QCodeEditor
)

if(MSVC)
  target_compile_options(SerialStudioStorage PRIVATE /W4)
endif()

if(BUILD_COMMERCIAL AND PRODUCTION_OPTIMIZATION)
  set_target_properties(SerialStudioStorage PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
  )
endif()

if(SS_UNITY_BUILD)
  set_target_properties(SerialStudioStorage PROPERTIES
    UNITY_BUILD ON
    UNITY_BUILD_BATCH_SIZE ${SS_UNITY_BATCH_SIZE}
  )

  if(MSVC)
    target_compile_options(SerialStudioStorage PRIVATE /bigobj)
  endif()
endif()

if(COMMAND serial_studio_harden)
  serial_studio_harden(SerialStudioStorage)
endif()
