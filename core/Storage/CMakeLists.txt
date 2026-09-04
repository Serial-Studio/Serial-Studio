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
# This is a partition of the old monolith, not a clean layer: it still reaches sideways into
# Pipeline, Devices, Api and Ui and upward into app/src. The include roots and the sibling links
# below spell that debt out; scripts/layer-verify.py ratchets it down per directed edge.
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
# Pro sinks: Historian sessions and the InfluxDB writer
#---------------------------------------------------------------------------------------------------

if(BUILD_COMMERCIAL)
  target_sources(
    SerialStudioStorage PRIVATE

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
# Include roots -- the transitional debt
#---------------------------------------------------------------------------------------------------
#
# Every partition directory is an include root and so is app/src, which is what keeps every
# #include line in the moved tree byte-identical. It is deliberate debt, not a layering statement:
# scripts/layer-verify.py counts the upward and sideways includes per directed edge against a
# checked-in baseline and fails on growth, and each follow-up spec drives one edge to zero.
#
#---------------------------------------------------------------------------------------------------

target_include_directories(
  SerialStudioStorage PUBLIC

  ${CMAKE_SOURCE_DIR}/core/Pipeline
  ${CMAKE_SOURCE_DIR}/core/Devices
  ${CMAKE_SOURCE_DIR}/core/Storage
  ${CMAKE_SOURCE_DIR}/core/Api
  ${CMAKE_SOURCE_DIR}/core/Ui
  ${CMAKE_SOURCE_DIR}/app/src
)

# MDF4/Player.cpp, MDF4/Export.cpp, InfluxDB/Export.cpp and Sessions/Export.cpp gate their Pro
# entry points on Licensing/CommercialToken.h, which the generated guards header backs.
if(BUILD_COMMERCIAL AND LICENSE_GUARDS_INCLUDE_DIR)
  target_include_directories(SerialStudioStorage PRIVATE ${LICENSE_GUARDS_INCLUDE_DIR})
endif()

target_link_libraries(
  SerialStudioStorage PUBLIC

  ${QT_LIBS}

  SerialStudio::Core
  SerialStudio::Protocols

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
