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
# SerialStudioUi -- the presentation layer (spec 0076)
#---------------------------------------------------------------------------------------------------
#
# The C++ half of the interface: the Dashboard facade and every visualization and output widget,
# the taskbar and window manager, the console, the platform window decorations, the AI assistant,
# and the Misc/ services (themes, translations, icons, extensions, problems, diagnostics).
#
# Misc/ModuleManager.* and Misc/CLI* stay with the executable: they are the composition root and
# its command line, and nothing below them may reach either.
#
# Headers ride along with their sources so AUTOMOC sees the Q_OBJECT classes; the same entries were
# removed from the executable's HEADERS list, or moc would run twice and the link would fail on
# duplicate staticMetaObject symbols.
#
# This is a partition of the old monolith, not a clean layer: it still reaches sideways into
# Pipeline, Devices, Storage and Api and upward into app/src. The include roots and the sibling
# links below spell that debt out; scripts/layer-verify.py ratchets it down per directed edge.
#
#---------------------------------------------------------------------------------------------------

qt_add_library(
  SerialStudioUi STATIC

  Misc/ThemeManager.cpp
  Misc/ThemeCatalog.cpp
  Misc/LanguageTable.cpp
  Misc/CommonFonts.cpp
  Misc/ContextRegistry.cpp
  Misc/Utilities.cpp
  Misc/PasswordHash.cpp
  Misc/Translator.cpp
  Misc/TimerEvents.cpp
  Misc/ProblemCenter.cpp
  Misc/ConnectionDiagnostics.cpp
  Misc/Diagnostics/DeviceAccess.cpp
  Misc/Diagnostics/SerialChecks.cpp
  Misc/Diagnostics/BluetoothChecks.cpp
  Misc/Diagnostics/NetworkChecks.cpp
  Misc/Problems/ProjectCheckers.cpp
  Misc/Problems/LinkCheckers.cpp
  Misc/Problems/ScriptCheckers.cpp
  Misc/WorkspaceManager.cpp
  Misc/ExtensionManager.cpp
  Misc/Extensions/PluginRunner.cpp
  Misc/Extensions/ExtensionCatalog.cpp
  Misc/Extensions/ExtensionInstaller.cpp
  Misc/Extensions/ExtensionAutoUpdater.cpp
  Misc/Examples.cpp
  Misc/HelpCenter.cpp
  Misc/IconEngine.cpp
  Misc/IconRegistry.cpp
  Misc/IconRegistryLegacy.cpp
  UI/CommandRegistry.cpp
  UI/CommandStrings.cpp
  Misc/GraphicsBackend.cpp
  Misc/HighDpiScaling.cpp
  Misc/CrashTracker.cpp
  Misc/BackupManager.cpp
  Platform/FileOpenEventFilter.cpp
  UI/DashboardWidget.cpp
  UI/Dashboard.cpp
  UI/Dashboard/DashboardTools.cpp
  UI/Dashboard/DashboardIngest.cpp
  UI/Dashboard/PlotControlBank.cpp
  UI/Dashboard/ReplaySeekEngine.cpp
  UI/Dashboard/DashboardViewState.cpp
  UI/Dashboard/WidgetMapBuilder.cpp
  UI/AlarmMonitor.cpp
  UI/Taskbar.cpp
  UI/Taskbar/TaskbarModel.cpp
  UI/Taskbar/TaskbarWindowMap.cpp
  UI/Taskbar/FocusCycler.cpp
  UI/Taskbar/TaskbarSearch.cpp
  UI/Taskbar/TaskbarWorkspaces.cpp
  UI/WindowManager/WindowGeometry.cpp
  UI/WindowManager/SnapOverlay.cpp
  UI/WindowManager/WindowLayoutStore.cpp
  UI/TaskbarSettings.cpp
  UI/WindowManager.cpp
  UI/SnapGuides.cpp
  UI/UISessionRegistry.cpp
  UI/WidgetRegistry.cpp
  UI/WidgetExtensions.cpp
  UI/WidgetManifestParser.cpp
  Misc/Problems/ExtensionCheckers.cpp
  UI/Widgets/ExtensionData.cpp
  UI/Widgets/LEDPanel.cpp
  UI/Widgets/Gauge.cpp
  UI/Widgets/Plot.cpp
  UI/Widgets/PlotBase.cpp
  UI/Widgets/PlotAreaFill.cpp
  UI/Widgets/GpuStroke.cpp
  UI/Widgets/PlotCurve.cpp
  UI/LayoutPatterns.cpp
  UI/Widgets/Compass.cpp
  UI/Widgets/Bar.cpp
  UI/Widgets/BarPanel.cpp
  UI/Widgets/Meter.cpp
  UI/Widgets/FFTPlot.cpp
  UI/Widgets/Accelerometer.cpp
  UI/Widgets/DataGrid.cpp
  UI/Widgets/Terminal.cpp
  UI/Widgets/Terminal/AnsiPalette.cpp
  UI/Widgets/Terminal/TerminalBuffer.cpp
  UI/Widgets/Terminal/Vt100Keymap.cpp
  UI/Widgets/Terminal/AnsiStateMachine.cpp
  UI/Widgets/Terminal/TerminalSearch.cpp
  UI/Widgets/Gyroscope.cpp
  UI/Widgets/GPS.cpp
  UI/Widgets/MultiPlot.cpp
  UI/Widgets/WebView.cpp
  Console/Annotations.cpp
  Console/Handler.cpp
  Console/TextFormat.cpp
  Console/Export.cpp
  Misc/Utilities.h
  Misc/PasswordHash.h
  Misc/CommonFonts.h
  Misc/ContextRegistry.h
  Misc/JsonValidator.h
  Misc/ThemeManager.h
  Misc/ThemeCatalog.h
  Misc/LanguageTable.h
  Misc/TimerEvents.h
  Misc/ProblemCenter.h
  Misc/ConnectionDiagnostics.h
  Misc/Diagnostics/DiagnosticsShared.h
  Misc/Diagnostics/DeviceAccess.h
  Misc/Diagnostics/SerialChecks.h
  Misc/Diagnostics/BluetoothChecks.h
  Misc/Diagnostics/NetworkChecks.h
  Misc/Problems/ProjectCheckers.h
  Misc/Problems/LinkCheckers.h
  Misc/Problems/ScriptCheckers.h
  Misc/Problems/ExtensionCheckers.h
  Misc/Translator.h
  Misc/WorkspaceManager.h
  Misc/ExtensionManager.h
  Misc/Extensions/PluginRunner.h
  Misc/Extensions/ExtensionCatalog.h
  Misc/Extensions/ExtensionInstaller.h
  Misc/Extensions/ExtensionAutoUpdater.h
  Misc/Examples.h
  Misc/HelpCenter.h
  Misc/IconEngine.h
  Misc/IconRegistry.h
  Misc/IconRegistryLegacy.h
  UI/CommandRegistry.h
  Misc/GraphicsBackend.h
  Misc/HighDpiScaling.h
  Misc/CrashTracker.h
  Misc/BackupManager.h
  UI/Dashboard.h
  UI/Dashboard/DashboardTools.h
  UI/Dashboard/DashboardIngest.h
  UI/Dashboard/PlotControlBank.h
  UI/Dashboard/ReplaySeekEngine.h
  UI/Dashboard/DashboardViewState.h
  UI/Dashboard/WidgetMapBuilder.h
  UI/DashboardWidget.h
  UI/AlarmMonitor.h
  UI/Taskbar.h
  UI/Taskbar/TaskbarModel.h
  UI/Taskbar/TaskbarWindowMap.h
  UI/Taskbar/FocusCycler.h
  UI/Taskbar/TaskbarSearch.h
  UI/Taskbar/TaskbarWorkspaces.h
  UI/WindowManager/WindowGeometry.h
  UI/WindowManager/SnapOverlay.h
  UI/WindowManager/WindowLayoutStore.h
  UI/TaskbarSettings.h
  UI/WindowManager.h
  UI/SnapGuides.h
  UI/WidgetRegistry.h
  UI/WidgetBands.h
  UI/LayoutPatterns.h
  UI/WidgetExtensions.h
  UI/WidgetManifestParser.h
  UI/Widgets/GPS.h
  UI/Widgets/MultiPlot.h
  UI/Widgets/WebView.h
  UI/Widgets/Gauge.h
  UI/Widgets/Plot.h
  UI/Widgets/PlotBase.h
  UI/Widgets/PlotAreaFill.h
  UI/Widgets/GpuStroke.h
  UI/Widgets/PlotCurve.h
  UI/Widgets/PlotLogScale.h
  UI/Widgets/PlotAutoScale.h
  UI/Widgets/DataGrid.h
  UI/Widgets/FFTPlot.h
  UI/Widgets/FFTWindow.h
  UI/Widgets/Gyroscope.h
  UI/Widgets/Bar.h
  UI/Widgets/BarPanel.h
  UI/Widgets/Meter.h
  UI/Widgets/Accelerometer.h
  UI/Widgets/ExtensionData.h
  UI/Widgets/LEDPanel.h
  UI/Widgets/Compass.h
  UI/Widgets/Terminal.h
  UI/Widgets/Terminal/AnsiPalette.h
  UI/Widgets/Terminal/TerminalBuffer.h
  UI/Widgets/Terminal/Vt100Keymap.h
  UI/Widgets/Terminal/AnsiStateMachine.h
  UI/Widgets/Terminal/TerminalSearch.h
  UI/UISessionRegistry.h
  Platform/NativeWindow.h
  Console/Annotations.h
  Console/Handler.h
  Console/TextFormat.h
  Console/Export.h
)

add_library(SerialStudio::Ui ALIAS SerialStudioUi)

#---------------------------------------------------------------------------------------------------
# Pro widgets, the shortcut generator and the AI assistant
#---------------------------------------------------------------------------------------------------

if(BUILD_COMMERCIAL)
  target_sources(
    SerialStudioUi PRIVATE

    Misc/Diagnostics/AudioChecks.h
    UI/Widgets/Plot3D.h
    UI/Widgets/Plot3D/Plot3DNodes.h
    UI/Widgets/Plot3D/Plot3DOverlay.h
    UI/Widgets/Plot3D/Plot3DStereo.h
    UI/Widgets/Plot3D/Plot3DEyeMaterial.h
    UI/Widgets/ImageView.h
    UI/Widgets/ImageExport.h
    UI/Widgets/AudioExport.h
    UI/Widgets/Waterfall.h
    UI/Widgets/Waterfall/WaterfallMath.h
    UI/Widgets/Waterfall/WaterfallTicks.h
    UI/Widgets/Waterfall/WaterfallColorMap.h
    UI/Widgets/Waterfall/WaterfallTiles.h
    UI/Widgets/Waterfall/WaterfallViewState.h
    UI/Widgets/Waterfall/WaterfallOverlay.h
    UI/Widgets/Waterfall/WaterfallRingTexture.h
    UI/Widgets/Waterfall/WaterfallSpectrogramNodes.h
    UI/Widgets/Painter.h
    UI/Widgets/PainterContext.h
    UI/Widgets/Painter/PainterGradient.h
    UI/Widgets/Painter/PainterPattern.h
    UI/Widgets/Painter/PainterEnums.h
    UI/Widgets/Painter/PainterGeometry.h
    UI/Widgets/Painter/PainterBlur.h
    UI/Widgets/Painter/PainterFont.h
    UI/Widgets/PainterDataBridge.h
    UI/ImageProvider.h
    UI/Widgets/Output/Base.h
    UI/Widgets/Output/Button.h
    UI/Widgets/Output/Slider.h
    UI/Widgets/Output/Toggle.h
    UI/Widgets/Output/TextField.h
    UI/Widgets/Output/Panel.h
    UI/Widgets/Output/PanelLayout.h
    Misc/ShortcutGenerator.h
    AI/Logging.h
    AI/KeyVault.h
    AI/CommandRegistry.h
    AI/FileSandbox.h
    AI/ToolDispatcher.h
    AI/Tools/ToolBulkTools.h
    AI/Tools/ToolCatalog.h
    AI/Tools/ToolCompact.h
    AI/Tools/ToolDispatch.h
    AI/Tools/ToolFilesystemTools.h
    AI/Tools/ToolResolve.h
    AI/Tools/ToolSchemas.h
    AI/Tools/ToolScriptTools.h
    AI/Tools/ToolSupport.h
    AI/Tools/ToolTileTools.h
    AI/Assistant.h
    AI/DocSearch.h
    AI/Redactor.h
    AI/SseEventReader.h
    AI/ContextBuilder.h
    AI/Conversation.h
    AI/Conversation/AsyncToolRunner.h
    AI/Conversation/AutoVerifier.h
    AI/Conversation/ChatDigest.h
    AI/Conversation/HelpFetcher.h
    AI/Conversation/MetaToolRunner.h
    AI/Conversation/ToolCallStatus.h
    AI/Conversation/ToolTurnRunner.h
    AI/Conversation/HistorySurgery.h
    AI/Conversation/MetaToolCatalog.h
    AI/Conversation/ReplyAssembly.h
    AI/Conversation/TokenBudget.h
    AI/ChatStore.h
    AI/SentinelProbe.h
    AI/MemoryStore.h
    AI/SkillRouter.h
    AI/Providers/Provider.h
    AI/Providers/ProviderJson.h
    AI/Providers/ThinkTagSplitter.h
    AI/Providers/ImmediateErrorReply.h
    AI/Providers/AnthropicProvider.h
    AI/Providers/AnthropicReply.h
    AI/Providers/OpenAICompatibleProvider.h
    AI/Providers/OpenAIProvider.h
    AI/Providers/OpenAIReply.h
    AI/Providers/GeminiProvider.h
    AI/Providers/GeminiReply.h
    AI/Providers/LocalProvider.h
    Misc/Diagnostics/AudioChecks.cpp
    UI/Widgets/Plot3D.cpp
    UI/Widgets/Plot3D/Plot3DNodes.cpp
    UI/Widgets/Plot3D/Plot3DOverlay.cpp
    UI/Widgets/Plot3D/Plot3DStereo.cpp
    UI/Widgets/Plot3D/Plot3DEyeMaterial.cpp
    UI/Widgets/ImageView.cpp
    UI/Widgets/ImageExport.cpp
    UI/Widgets/AudioExport.cpp
    UI/Widgets/Waterfall.cpp
    UI/Widgets/Waterfall/WaterfallTicks.cpp
    UI/Widgets/Waterfall/WaterfallColorMap.cpp
    UI/Widgets/Waterfall/WaterfallTiles.cpp
    UI/Widgets/Waterfall/WaterfallViewState.cpp
    UI/Widgets/Waterfall/WaterfallOverlay.cpp
    UI/Widgets/Waterfall/WaterfallRingTexture.cpp
    UI/Widgets/Waterfall/WaterfallSpectrogramNodes.cpp
    UI/Widgets/Painter.cpp
    UI/Widgets/PainterContext.cpp
    UI/Widgets/Painter/PainterGradient.cpp
    UI/Widgets/Painter/PainterBlur.cpp
    UI/Widgets/Painter/PainterFont.cpp
    UI/Widgets/Painter/PainterPattern.cpp
    UI/Widgets/PainterDataBridge.cpp
    UI/ImageProvider.cpp
    UI/Widgets/Output/Base.cpp
    UI/Widgets/Output/Button.cpp
    UI/Widgets/Output/Slider.cpp
    UI/Widgets/Output/Toggle.cpp
    UI/Widgets/Output/TextField.cpp
    UI/Widgets/Output/Panel.cpp
    UI/Widgets/Output/PanelLayout.cpp
    Misc/ShortcutGenerator.cpp
    AI/Logging.cpp
    AI/KeyVault.cpp
    AI/CommandRegistry.cpp
    AI/FileSandbox.cpp
    AI/ToolDispatcher.cpp
    AI/Tools/ToolBulkTools.cpp
    AI/Tools/ToolCatalog.cpp
    AI/Tools/ToolCompact.cpp
    AI/Tools/ToolDispatch.cpp
    AI/Tools/ToolFilesystemTools.cpp
    AI/Tools/ToolResolve.cpp
    AI/Tools/ToolSchemas.cpp
    AI/Tools/ToolScriptTools.cpp
    AI/Tools/ToolSupport.cpp
    AI/Tools/ToolTileTools.cpp
    AI/Assistant.cpp
    AI/DocSearch.cpp
    AI/Redactor.cpp
    AI/SseEventReader.cpp
    AI/ContextBuilder.cpp
    AI/Conversation.cpp
    AI/Conversation/AsyncToolRunner.cpp
    AI/Conversation/AutoVerifier.cpp
    AI/Conversation/ChatDigest.cpp
    AI/Conversation/HelpFetcher.cpp
    AI/Conversation/MetaToolRunner.cpp
    AI/Conversation/ToolTurnRunner.cpp
    AI/Conversation/HistorySurgery.cpp
    AI/Conversation/MetaToolCatalog.cpp
    AI/Conversation/ReplyAssembly.cpp
    AI/Conversation/TokenBudget.cpp
    AI/ChatStore.cpp
    AI/SentinelProbe.cpp
    AI/MemoryStore.cpp
    AI/SkillRouter.cpp
    AI/Providers/Provider.cpp
    AI/Providers/ProviderJson.cpp
    AI/Providers/ThinkTagSplitter.cpp
    AI/Providers/AnthropicProvider.cpp
    AI/Providers/AnthropicReply.cpp
    AI/Providers/OpenAICompatibleProvider.cpp
    AI/Providers/OpenAIProvider.cpp
    AI/Providers/OpenAIReply.cpp
    AI/Providers/GeminiProvider.cpp
    AI/Providers/GeminiReply.cpp
    AI/Providers/LocalProvider.cpp
  )

  if(WIN32)
    target_sources(SerialStudioUi PRIVATE Misc/ShortcutGenerator_Windows.cpp)
  elseif(APPLE)
    target_sources(SerialStudioUi PRIVATE Misc/ShortcutGenerator_macOS.cpp)
  elseif(UNIX)
    target_sources(SerialStudioUi PRIVATE Misc/ShortcutGenerator_Linux.cpp)
  endif()
endif()

#---------------------------------------------------------------------------------------------------
# Platform window decorations
#---------------------------------------------------------------------------------------------------
#
# Windows and Linux share the client-side-decoration pair; macOS gets the Objective-C++ NSWindow
# translation unit instead, which cannot join a unity TU.
#
#---------------------------------------------------------------------------------------------------

if(WIN32)
  target_sources(
    SerialStudioUi PRIVATE

    Platform/CSD.cpp
    Platform/NativeWindow_CSD.cpp
    Platform/CSD.h
  )
elseif(APPLE)
  target_sources(SerialStudioUi PRIVATE Platform/NativeWindow_macOS.mm)

  set_source_files_properties(Platform/NativeWindow_macOS.mm
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
  )
elseif(UNIX)
  target_sources(
    SerialStudioUi PRIVATE

    Platform/CSD.cpp
    Platform/NativeWindow_CSD.cpp
    Platform/CSD.h
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
  SerialStudioUi PUBLIC

  ${CMAKE_SOURCE_DIR}/core/Pipeline
  ${CMAKE_SOURCE_DIR}/core/Devices
  ${CMAKE_SOURCE_DIR}/core/Storage
  ${CMAKE_SOURCE_DIR}/core/Api
  ${CMAKE_SOURCE_DIR}/core/Ui
  ${CMAKE_SOURCE_DIR}/app/src
)

# The Pro widgets, the shortcut generator and the assistant gate their entry points on
# Licensing/CommercialToken.h, which the generated guards header backs.
if(BUILD_COMMERCIAL AND LICENSE_GUARDS_INCLUDE_DIR)
  target_include_directories(SerialStudioUi PRIVATE ${LICENSE_GUARDS_INCLUDE_DIR})
endif()

# kissfft is the real-FFT plan behind FFTPlot and the waterfall spectrogram.
target_link_libraries(
  SerialStudioUi PUBLIC

  ${QT_LIBS}

  SerialStudio::Core
  SerialStudio::Protocols

  kissfft
  luajit
  QCodeEditor
)

if(MSVC)
  target_compile_options(SerialStudioUi PRIVATE /W4)
endif()

if(BUILD_COMMERCIAL AND PRODUCTION_OPTIMIZATION)
  set_target_properties(SerialStudioUi PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
  )
endif()

if(SS_UNITY_BUILD)
  set_target_properties(SerialStudioUi PROPERTIES
    UNITY_BUILD ON
    UNITY_BUILD_BATCH_SIZE ${SS_UNITY_BATCH_SIZE}
  )

  if(MSVC)
    target_compile_options(SerialStudioUi PRIVATE /bigobj)
  endif()
endif()

if(COMMAND serial_studio_harden)
  serial_studio_harden(SerialStudioUi)
endif()
