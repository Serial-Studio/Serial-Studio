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
# SerialStudioPipeline -- the acquisition and project layer (spec 0076)
#---------------------------------------------------------------------------------------------------
#
# The hotpath and everything that configures it: the frame reader and the pipeline host thread,
# the stream worker, the parsers (JS, Lua, Native), the project model and its editors, the frame
# builder with its block stager and publisher, the data tables and the DSP helpers.
#
# The rest of IO/ -- the drivers, DeviceManager, ConnectionManager -- is NOT here: it lives in
# SerialStudio::Devices. What stays is the part that runs on IO::PipelineHost's processing thread.
#
# Headers ride along with their sources so AUTOMOC sees the Q_OBJECT classes; the same entries were
# removed from the executable's HEADERS list, or moc would run twice and the link would fail on
# duplicate staticMetaObject symbols.
#
# This is a partition of the old monolith, not a clean layer: it still reaches sideways into
# Devices, Storage, Api and Ui and upward into app/src. The include roots and the sibling links
# below spell that debt out; scripts/layer-verify.py ratchets it down per directed edge.
#
#---------------------------------------------------------------------------------------------------

qt_add_library(
  SerialStudioPipeline STATIC

  Platform/AppPlatform.cpp
  IO/PipelineHost.cpp
  IO/StreamWorker.cpp
  IO/FrameReader.cpp
  DataModel/Scripting/ControlScript.cpp
  DataModel/Scripting/ControlScriptWorker.cpp
  DataModel/Scripting/MacroWorker.cpp
  DataModel/Scripting/MacroRunner.cpp
  DataModel/Scripting/FrameParser.cpp
  DataModel/Scripting/ParserTemplateCatalog.cpp
  DataModel/Scripting/ReplayRowCodec.cpp
  DataModel/Scripting/ScriptDeviceWait.cpp
  DataModel/Scripting/ScriptResult.cpp
  DataModel/Scripting/FrameParserPipeline.cpp
  DataModel/Scripting/JsScriptEngine.cpp
  DataModel/Scripting/JsWatchdog.cpp
  DataModel/Scripting/JsWatchdogThread.cpp
  DataModel/Scripting/LuaScriptEngine.cpp
  DataModel/Scripting/LuaCompat.cpp
  DataModel/Scripting/LuaMigration.cpp
  DataModel/Scripting/LuaDeadlineHook.cpp
  DataModel/Scripting/ScriptDryRun.cpp
  DataModel/Scripting/ExpressionTransform.cpp
  DataModel/Scripting/DashboardApi.cpp
  DataModel/Scripting/DeviceWriteApi.cpp
  DataModel/Scripting/ScriptApiCall.cpp
  DataModel/Scripting/ScriptTemplates.cpp
  DataModel/Scripting/CFrameParser.cpp
  DataModel/Scripting/NativeTemplates/NativeTemplate.cpp
  DataModel/Scripting/NativeTemplates/NativeTemplateRegistry.cpp
  DataModel/Scripting/NativeTemplates/TextTemplates.cpp
  DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp
  DataModel/Scripting/NativeTemplates/WireLatchTemplates.cpp
  DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp
  DataModel/Scripting/NativeTemplates/NativeTemplateSupport.cpp
  DataModel/Scripting/NativeTemplates/BinaryRaw.cpp
  DataModel/Scripting/NativeTemplates/BinaryHex.cpp
  DataModel/Scripting/NativeTemplates/BinaryBase64.cpp
  DataModel/Scripting/NativeTemplates/BinaryTlv.cpp
  DataModel/Scripting/NativeTemplates/BinaryCobs.cpp
  DataModel/Scripting/NativeTemplates/BinarySlip.cpp
  DataModel/Scripting/NativeTemplates/BinaryUbx.cpp
  DataModel/Scripting/NativeTemplates/BinarySirf.cpp
  DataModel/Scripting/NativeTemplates/BinaryMavlink.cpp
  DataModel/Scripting/NativeTemplates/BinaryNmea2000.cpp
  DataModel/Scripting/NativeTemplates/BinaryRtcm.cpp
  DataModel/Scripting/NativeTemplates/BinaryModbus.cpp
  DataModel/Scripting/NativeTemplates/BinaryMessagePack.cpp
  DataModel/Scripting/NativeTemplates/TextDelimited.cpp
  DataModel/Scripting/NativeTemplates/TextFixedWidth.cpp
  DataModel/Scripting/NativeTemplates/TextKeyValue.cpp
  DataModel/Scripting/NativeTemplates/TextIniConfig.cpp
  DataModel/Scripting/NativeTemplates/TextAtCommands.cpp
  DataModel/Scripting/NativeTemplates/TextNmea0183.cpp
  DataModel/Scripting/NativeTemplates/TextUrlEncoded.cpp
  DataModel/Scripting/NativeTemplates/TextJsonData.cpp
  DataModel/Scripting/NativeTemplates/TextXmlData.cpp
  DataModel/Scripting/NativeTemplates/TextYamlData.cpp
  DataModel/Scripting/NativeTemplates/MultiFrameBatchedSensor.cpp
  DataModel/Scripting/NativeTemplates/MultiFrameTimeSeries2d.cpp
  DataModel/Editors/JsCodeEditor.cpp
  DataModel/Editors/EmbeddedCodeEditor.cpp
  DataModel/Editors/EmbeddedCodeEditorItem.cpp
  DataModel/Editors/EditorFormatting.cpp
  DataModel/Editors/ScriptTemplateCatalog.cpp
  DataModel/Editors/ControlScriptEditor.cpp
  DataModel/Editors/MacroEditor.cpp
  DataModel/Editors/SerialStudioCompleter.cpp
  DataModel/Editors/FrameParserModel.cpp
  DataModel/Editors/CodeFormatter.cpp
  DataModel/Editors/ExpressionHighlighter.cpp
  DataModel/Editors/OutputCodeEditor.cpp
  DataModel/ProjectModel.cpp
  DataModel/Project/ProjectHistory.cpp
  DataModel/Project/PropertyHooks.cpp
  DataModel/Project/PropertyValidators.cpp
  DataModel/Generated/DatasetSerialization.cpp
  DataModel/Generated/DatasetForm.cpp
  DataModel/Project/ProjectPersistence.cpp
  DataModel/Project/ProjectPresentation.cpp
  DataModel/Project/ProjectLoader.cpp
  DataModel/Project/ProjectFolders.cpp
  DataModel/Project/ProjectWorkspaces.cpp
  DataModel/Project/ProjectWorkspaceRefs.cpp
  DataModel/Project/ProjectFixedLayouts.cpp
  DataModel/Project/ProjectTables.cpp
  DataModel/Project/ProjectSources.cpp
  DataModel/Project/ProjectEntities.cpp
  DataModel/Project/ProjectOutputWidgets.cpp
  DataModel/Project/ProjectBulkOps.cpp
  DataModel/Project/ProjectNavHistory.cpp
  DataModel/ProjectEditor.cpp
  DataModel/Project/ProjectEditorWiring.cpp
  DataModel/Project/ProjectEditorTree.cpp
  DataModel/Project/ProjectEditorMqtt.cpp
  DataModel/Project/ProjectEditorForms.cpp
  DataModel/Project/ProjectEditorCommit.cpp
  DataModel/Project/ProjectEditorMultiSelect.cpp
  DataModel/Project/ProjectEditorSelection.cpp
  DataModel/Project/ProjectEditorSummaries.cpp
  DataModel/Importers/ProtoImporter.cpp
  DataModel/Importers/ProtoLexer.cpp
  DataModel/Importers/ProtoParser.cpp
  DataModel/ExportStructure.cpp
  DataModel/FrameBuilder.cpp
  DataModel/ReplayPlaybackEngine.cpp
  DataModel/FrameBuilder/BlockPublisher.cpp
  DataModel/FrameBuilder/BlockStager.cpp
  DataModel/FrameBuilder/ReplayIngest.cpp
  DataModel/FrameBuilder/LatestFrameTap.cpp
  DataModel/FrameBuilder/QuickPlotBuilder.cpp
  DataModel/FrameBuilder/TableSnapshotChannel.cpp
  DataModel/FrameBuilder/TableScriptBridge.cpp
  DataModel/FrameBuilder/TransformCompiler.cpp
  DataModel/FramePoolPolicy.cpp
  DataModel/Frame.cpp
  DataModel/FrameConsumer.cpp
  DataModel/DataTable.cpp
  DataModel/RowFilterProxy.cpp
  DataModel/Editors/DatasetTransformEditor.cpp
  DataModel/NotificationCenter.cpp
  DataModel/Dialogs/TransmitTestDialog.cpp
  Platform/AppPlatform.h
  IO/FrameConfig.h
  IO/PipelineHost.h
  IO/StreamWorker.h
  IO/FrameReader.h
  DataModel/Scripting/FrameParser.h
  DataModel/Scripting/ParserTemplateCatalog.h
  DataModel/Scripting/ReplayRowCodec.h
  DataModel/Scripting/ScriptDeviceWait.h
  DataModel/Scripting/ScriptFrameShaping.h
  DataModel/Scripting/ScriptResult.h
  DataModel/Scripting/ControlScript.h
  DataModel/Scripting/ControlScriptWorker.h
  DataModel/Scripting/MacroWorker.h
  DataModel/Scripting/MacroRunner.h
  DataModel/Scripting/FrameParserPipeline.h
  DataModel/Scripting/IScriptEngine.h
  DataModel/Scripting/JsScriptEngine.h
  DataModel/Scripting/JsWatchdog.h
  DataModel/Scripting/JsWatchdogThread.h
  DataModel/Scripting/LuaScriptEngine.h
  DataModel/Scripting/LuaCompat.h
  DataModel/Scripting/LuaCompatJIT.h
  DataModel/Scripting/LuaMigration.h
  DataModel/Scripting/LuaDeadlineHook.h
  DataModel/Scripting/ScriptDryRun.h
  DataModel/Scripting/ExpressionTransform.h
  DataModel/Scripting/DashboardApi.h
  DataModel/Scripting/DeviceWriteApi.h
  DataModel/Scripting/ScriptApiCall.h
  DataModel/Scripting/ScriptTemplates.h
  DataModel/Scripting/CFrameParser.h
  DataModel/Scripting/NativeTemplates/NativeTemplate.h
  DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h
  DataModel/Scripting/NativeTemplates/BinaryRaw.h
  DataModel/Scripting/NativeTemplates/BinaryHex.h
  DataModel/Scripting/NativeTemplates/BinaryBase64.h
  DataModel/Scripting/NativeTemplates/BinaryTlv.h
  DataModel/Scripting/NativeTemplates/BinaryCobs.h
  DataModel/Scripting/NativeTemplates/BinarySlip.h
  DataModel/Scripting/NativeTemplates/BinaryUbx.h
  DataModel/Scripting/NativeTemplates/BinarySirf.h
  DataModel/Scripting/NativeTemplates/BinaryMavlink.h
  DataModel/Scripting/NativeTemplates/BinaryNmea2000.h
  DataModel/Scripting/NativeTemplates/BinaryRtcm.h
  DataModel/Scripting/NativeTemplates/BinaryModbus.h
  DataModel/Scripting/NativeTemplates/BinaryMessagePack.h
  DataModel/Scripting/NativeTemplates/TextDelimited.h
  DataModel/Scripting/NativeTemplates/TextFixedWidth.h
  DataModel/Scripting/NativeTemplates/TextKeyValue.h
  DataModel/Scripting/NativeTemplates/TextIniConfig.h
  DataModel/Scripting/NativeTemplates/TextAtCommands.h
  DataModel/Scripting/NativeTemplates/TextNmea0183.h
  DataModel/Scripting/NativeTemplates/TextUrlEncoded.h
  DataModel/Scripting/NativeTemplates/TextJsonData.h
  DataModel/Scripting/NativeTemplates/TextXmlData.h
  DataModel/Scripting/NativeTemplates/TextYamlData.h
  DataModel/Scripting/NativeTemplates/MultiFrameBatchedSensor.h
  DataModel/Scripting/NativeTemplates/MultiFrameTimeSeries2d.h
  DataModel/Editors/JsCodeEditor.h
  DataModel/Editors/EmbeddedCodeEditor.h
  DataModel/Editors/EmbeddedCodeEditorItem.h
  DataModel/Editors/EditorFormatting.h
  DataModel/Editors/ScriptTemplateCatalog.h
  DataModel/Editors/ControlScriptEditor.h
  DataModel/Editors/MacroEditor.h
  DataModel/Editors/SerialStudioCompleter.h
  DataModel/Editors/ExpressionHighlighter.h
  DataModel/Editors/FrameParserModel.h
  DataModel/Editors/CodeFormatter.h
  DataModel/Editors/OutputCodeEditor.h
  DataModel/ProjectModel.h
  DataModel/Project/ProjectHistory.h
  DataModel/Project/PropertyHooks.h
  DataModel/Generated/DatasetRegistry.h
  DataModel/Project/ProjectPersistence.h
  DataModel/Project/ProjectPresentation.h
  DataModel/Project/ProjectLoader.h
  DataModel/Project/ProjectFolders.h
  DataModel/Project/ProjectWorkspaces.h
  DataModel/Project/ProjectWorkspaceRefs.h
  DataModel/Project/ProjectFixedLayouts.h
  DataModel/Project/ProjectTables.h
  DataModel/Project/ProjectSources.h
  DataModel/Project/ProjectEntities.h
  DataModel/Project/ProjectOutputWidgets.h
  DataModel/Project/ProjectBulkOps.h
  DataModel/Project/ProjectNavHistory.h
  DataModel/Project/ProjectNaming.h
  DataModel/Project/ProjectEditorIcons.h
  DataModel/ProjectEditor.h
  DataModel/Project/ProjectEditorItemIds.h
  DataModel/Importers/ProtoImporter.h
  DataModel/Importers/ProtoLexer.h
  DataModel/Importers/ProtoParser.h
  DataModel/DataBlock.h
  DataModel/Frame.h
  DataModel/FrameKeys.h
  DataModel/FrameBuilder.h
  DataModel/RepublishGate.h
  DataModel/FrameBuilder/BlockPublisher.h
  DataModel/FrameBuilder/BlockStager.h
  DataModel/FrameBuilder/ReplayIngest.h
  DataModel/FrameBuilder/LatestFrameTap.h
  DataModel/FrameBuilder/QuickPlotBuilder.h
  DataModel/FrameBuilder/TableSnapshotChannel.h
  DataModel/FrameBuilder/TableScriptBridge.h
  DataModel/FrameBuilder/TransformCompiler.h
  DataModel/FramePoolPolicy.h
  DataModel/FrameConsumer.h
  DataModel/DataTable.h
  DataModel/RowFilterProxy.h
  DataModel/ExportSchema.h
  DataModel/ExportStructure.h
  DataModel/ReplayPlaybackEngine.h
  DataModel/Editors/DatasetTransformEditor.h
  DataModel/NotificationCenter.h
  DataModel/Dialogs/TransmitTestDialog.h
  DSP.h
  DSPDownsample.h
)

add_library(SerialStudio::Pipeline ALIAS SerialStudioPipeline)

#---------------------------------------------------------------------------------------------------
# Pro importers and the Canvas Widget editor
#---------------------------------------------------------------------------------------------------

if(BUILD_COMMERCIAL)
  target_sources(
    SerialStudioPipeline PRIVATE

    DataModel/Editors/PainterCodeEditor.h
    DataModel/Importers/AxisTicks.h
    DataModel/Importers/ImporterCommon.h
    DataModel/Importers/DBCImporter.h
    DataModel/Importers/DBCMultiplexing.h
    DataModel/Importers/ModbusRegisterMap.h
    DataModel/Importers/ModbusMapImporter.h
    DataModel/Editors/PainterCodeEditor.cpp
    DataModel/Importers/DBCImporter.cpp
    DataModel/Importers/DBCMultiplexing.cpp
    DataModel/Importers/ModbusRegisterMap.cpp
    DataModel/Importers/ModbusMapImporter.cpp
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
  SerialStudioPipeline PUBLIC

  ${CMAKE_SOURCE_DIR}/core/Pipeline
  ${CMAKE_SOURCE_DIR}/core/Devices
  ${CMAKE_SOURCE_DIR}/core/Storage
  ${CMAKE_SOURCE_DIR}/core/Api
  ${CMAKE_SOURCE_DIR}/core/Ui
  ${CMAKE_SOURCE_DIR}/app/src
)

# FrameBuilder.cpp and NotificationCenter.cpp gate their Pro entry points on
# Licensing/CommercialToken.h, which the generated guards header backs.
if(BUILD_COMMERCIAL AND LICENSE_GUARDS_INCLUDE_DIR)
  target_include_directories(SerialStudioPipeline PRIVATE ${LICENSE_GUARDS_INCLUDE_DIR})
endif()

# luajit is the Lua parser and transform engine; QCodeEditor backs every embedded script editor
# under DataModel/Editors/.
target_link_libraries(
  SerialStudioPipeline PUBLIC

  ${QT_LIBS}

  SerialStudio::Core
  SerialStudio::Protocols

  luajit
  QCodeEditor
)

#
# QuickPlotBuilder.cpp includes IO/Drivers/Audio.h under BUILD_COMMERCIAL, which pulls
# ThirdParty/miniaudio.h: the MA_* layout switches must match the executable's miniaudio.cpp.
#
if(BUILD_COMMERCIAL)
  ss_apply_miniaudio_definitions(SerialStudioPipeline)
endif()

if(MSVC)
  target_compile_options(SerialStudioPipeline PRIVATE /W4)
endif()

if(BUILD_COMMERCIAL AND PRODUCTION_OPTIMIZATION)
  set_target_properties(SerialStudioPipeline PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
  )
endif()

if(SS_UNITY_BUILD)
  set_target_properties(SerialStudioPipeline PROPERTIES
    UNITY_BUILD ON
    UNITY_BUILD_BATCH_SIZE ${SS_UNITY_BATCH_SIZE}
  )

  if(MSVC)
    target_compile_options(SerialStudioPipeline PRIVATE /bigobj)
  endif()
endif()

if(COMMAND serial_studio_harden)
  serial_studio_harden(SerialStudioPipeline)
endif()
