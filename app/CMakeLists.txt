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
# Application module setup
#---------------------------------------------------------------------------------------------------
#
# Configures the main Serial Studio application module:
#
# Project name: app (internal module name)
# Version: Inherited from parent PROJECT_VERSION
# Language: C++ only
#
# This is a subproject of the main Serial-Studio CMake project
# and inherits configuration from the root CMakeLists.txt
#
#---------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20)
project(app LANGUAGES C CXX VERSION ${PROJECT_VERSION})

#---------------------------------------------------------------------------------------------------
# Qt automation and C++ standard
#---------------------------------------------------------------------------------------------------
#
# Qt Meta-Object Compiler (MOC) automation:
#   CMAKE_AUTOMOC=ON  - Auto-run moc on files with Q_OBJECT/Q_GADGET
#   CMAKE_AUTOUIC=ON  - Auto-compile .ui files to ui_*.h headers
#   CMAKE_AUTORCC=ON  - Auto-compile .qrc resource files to C++ code
#
# C++ standard: set once in the root CMakeLists.txt (CMAKE_CXX_STANDARD=20,
# CMAKE_CXX_STANDARD_REQUIRED=ON) and inherited here. Re-setting it in this file made two
# places able to disagree about the language the app compiles as.
#
# C++20 features used in Serial Studio:
#   - std::span, std::format (pending), concepts
#   - Designated initializers, constexpr improvements
#   - <=> three-way comparison operator
#
#---------------------------------------------------------------------------------------------------

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#---------------------------------------------------------------------------------------------------
# Qt 6 module dependencies
#---------------------------------------------------------------------------------------------------
#
# Configures required Qt 6 modules for GPL and commercial builds:
#
# Core modules (GPL + Commercial):
#   Qt6::Core          - Core non-GUI classes (QObject, QString, etc.)
#   Qt6::Gui           - GUI base classes (QWindow, QImage, etc.)
#   Qt6::Widgets       - Desktop widgets (QMainWindow, QPushButton, etc.)
#   Qt6::Qml           - QML engine for declarative UI
#   Qt6::Quick         - Qt Quick 2D renderer
#   Qt6::QuickControls2 - Modern UI controls (Material/Fluent design)
#   Qt6::Graphs        - 2D graphs and charts (replaces QtCharts)
#   Qt6::Svg           - SVG rendering support
#   Qt6::Sql           - Database abstraction layer (for settings)
#   Qt6::Bluetooth     - Bluetooth LE device discovery and communication
#   Qt6::Network       - TCP/UDP sockets and the HTTP client of the Network driver
#   Qt6::WebSockets    - WebSocket client for the Network driver (spec 0068)
#   Qt6::SerialPort    - RS-232/RS-485 serial communication
#   Qt6::Core5Compat   - Qt 5 compatibility (TextCodec, etc.)
#   Qt6::LinguistTools - Internationalization (lupdate, lrelease)
#
# Commercial-only modules:
#   Qt6::Mqtt          - MQTT client for IoT protocols
#   Qt6::SerialBus     - CAN Bus, Modbus TCP/RTU support
#
# OPC UA does NOT use Qt6::OpcUa (spec 0067). The driver talks to the open62541 static library
# vendored in lib/open62541, linked through target_link_open62541() below, so the shipped package
# carries no Qt OPC UA module and no backend plugin.
#
# Qt policies:
#   QTP0001=NEW - Use new Qt resource system (qt_add_resources)
#   QTP0002=NEW - Use new Qt translation system (qt_add_translations)
#   QTP0004=NEW - Use new Qt QML module system (qt_add_qml_module)
#
#---------------------------------------------------------------------------------------------------

set(QT_MODULES
  Svg
  Gui
  Qml
  Sql
  Core
  Quick
  Graphs
  Widgets
  Network
  Bluetooth
  GuiPrivate
  SerialPort
  WebSockets
  Core5Compat
  LinguistTools
  QuickControls2
)

# Qt6::GuiPrivate carries <rhi/qrhi.h>, which the waterfall's persistent ring texture needs to
# upload one scanline instead of a whole image per tick (spec 0075, R15.1). QRhi is semi-public:
# source-compatible within a Qt minor series only, so a Qt upgrade re-checks WaterfallRingTexture.
# The version-tie warning it prints is the acknowledged cost of that, so silence it.
set(QT_NO_PRIVATE_MODULE_WARNING ON)
set(QT_LIBS
  Qt6::Core
  Qt6::Svg
  Qt6::Sql
  Qt6::Gui
  Qt6::GuiPrivate
  Qt6::Qml
  Qt6::Quick
  Qt6::Graphs
  Qt6::Widgets
  Qt6::Network
  Qt6::Bluetooth
  Qt6::SerialPort
  Qt6::WebSockets
  Qt6::Core5Compat
  Qt6::QuickControls2
)

if(WITH_WEBENGINE)
  list(APPEND QT_MODULES WebEngineQuick)
  list(APPEND QT_LIBS Qt6::WebEngineQuick)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND QT_MODULES DBus)
  list(APPEND QT_LIBS Qt6::DBus)
endif()

if(BUILD_COMMERCIAL)
  set(QT_MODULES
    ${QT_MODULES}
    Mqtt
    Multimedia
    SerialBus
    ShaderTools
  )
  set(QT_LIBS
    ${QT_LIBS}
    Qt6::Mqtt
    Qt6::Multimedia
    Qt6::SerialBus
  )
endif()


find_package(
  Qt6 REQUIRED
  COMPONENTS
  ${QT_MODULES}
)

qt_standard_project_setup()
qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0002 NEW)
qt_policy(SET QTP0004 NEW)

#---------------------------------------------------------------------------------------------------
# MiniAudio feature switches, shared with SerialStudio::Devices
#---------------------------------------------------------------------------------------------------
#
# The MA_* switches decide which members miniaudio's structs carry, so the executable and any
# archive that includes ThirdParty/miniaudio.h must set exactly the same ones or the two disagree
# about the size of a ma_device. Defined before add_subdirectory(core) because that is where
# SerialStudio::Devices -- whose Audio driver includes the header -- calls it. The vendored .cpp,
# its warning suppressions and the ALSA link stay with the executable, which is the only target
# that compiles miniaudio's implementation.
#
#---------------------------------------------------------------------------------------------------

function(ss_apply_miniaudio_definitions TARGET)
  target_compile_definitions(${TARGET} PRIVATE
    MA_NO_DECODING
    MA_NO_ENCODING
    MA_NO_WAV
    MA_NO_FLAC
    MA_NO_MP3
    MA_NO_RESOURCE_MANAGER
    MA_NO_NODE_GRAPH
    MA_NO_ENGINE
    MA_NO_GENERATION
    MA_NO_CUSTOM
    MA_NO_NULL
  )

  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64)$")
    target_compile_definitions(${TARGET} PRIVATE MA_SUPPORT_SSE2)
  endif()

  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$" AND NOT APPLE)
    target_compile_definitions(${TARGET} PRIVATE MA_SUPPORT_NEON)
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
# Modular core libraries (spec 0076)
#---------------------------------------------------------------------------------------------------
#
# core/ holds the statically linked first-party layers that fold into this executable: the strict
# SerialStudio::Core and SerialStudio::Protocols, and the five transitional partitions of the old
# monolith (Pipeline, Devices, Storage, Api, Ui). The position is the contract: after Qt discovery
# and the policy block, so the libraries inherit Qt, CMAKE_AUTOMOC and the global
# optimisation/SIMD/PGO/sanitizer flags, and after QT_LIBS, which every partition links.
#
# The binary directory is spelled out because the source directory is outside app/.
#
#---------------------------------------------------------------------------------------------------

#---------------------------------------------------------------------------------------------------
# gRPC and Protobuf discovery (ENABLE_GRPC=ON)
#---------------------------------------------------------------------------------------------------
#
# Found here, before add_subdirectory(core), and not in the gRPC server block further down. The
# imported targets a find_package() creates are directory-scoped and only reach subdirectories
# added after the call, and SerialStudio::Pipeline and SerialStudio::Devices link gRPC::grpc++
# and protobuf::libprotobuf. Discovering them after the core tree left both archives pointing at
# targets their directories could not see, and every ENABLE_GRPC=ON configure failed at generate
# time. The protoc rule, the generated stubs and the server sources stay in the block below.
#
#---------------------------------------------------------------------------------------------------

if(ENABLE_GRPC)
  find_package(absl CONFIG REQUIRED)
  find_package(utf8_range CONFIG QUIET)
  find_package(re2 CONFIG REQUIRED)
  find_package(Protobuf CONFIG REQUIRED)
  find_package(gRPC CONFIG REQUIRED)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/core ${CMAKE_BINARY_DIR}/core)

#---------------------------------------------------------------------------------------------------
# Source file organization
#---------------------------------------------------------------------------------------------------
#
# Defines the application source files, headers, and QML files:
#
# Directory structure:
#   src/         - C++ implementation files (.cpp)
#   src/         - C++ header files (.h)
#   qml/         - QML declarative UI files (.qml)
#
# What is left here after spec 0076 stage 2 is the composition root and what only it may touch:
#   - main.cpp, SerialStudio.*, AppState.*, SessionContext.*, Misc/ModuleManager.*
#   - Misc/CLI.*, Misc/CLI/: the command line, and the SS_INAPP_TESTS selftest entry points
#   - SelfTest/, Benchmark/: tiers that run before or instead of the GUI
#   - Licensing/, ThirdParty/: the Pro token, its vendored crypto, and miniaudio's implementation
#   - API/GRPC/: generated by the protoc custom command below, which stays with this target
#
# Every other subsystem moved under core/ as a partition library; see core/CMakeLists.txt.
#
# Commercial-only sources are conditionally added based on BUILD_COMMERCIAL
#
# Note: include_directories() makes all headers accessible without relative paths (e.g.,
# #include "IO/Manager.h" instead of "../IO/Manager.h"). The partition roots are listed here for
# the same reason the libraries export them: the moved trees kept their internal paths, so this
# target -- and app/tests/, which recompiles their translation units -- resolves the unchanged
# includes without touching a single source file.
#
#---------------------------------------------------------------------------------------------------

include_directories(src)
include_directories(${CMAKE_SOURCE_DIR}/core/Pipeline)
include_directories(${CMAKE_SOURCE_DIR}/core/Devices)
include_directories(${CMAKE_SOURCE_DIR}/core/Storage)
include_directories(${CMAKE_SOURCE_DIR}/core/Api)
include_directories(${CMAKE_SOURCE_DIR}/core/Ui)

if(BUILD_COMMERCIAL AND LICENSE_GUARDS_INCLUDE_DIR)
  include_directories(${LICENSE_GUARDS_INCLUDE_DIR})
endif()

set(SOURCES
  src/Misc/ModuleManager.cpp
  src/Misc/CLI.cpp
  src/Misc/CLI/CliBusConfig.cpp
  src/Misc/CLI/CliIndustrialConfig.cpp
  src/Misc/CLI/CliSpecParsers.cpp
  src/SelfTest/SelfTest.cpp
  src/SelfTest/QmlInstantiationSuite.cpp
  src/Benchmark/HotpathBenchmark.cpp
  src/Benchmark/BenchmarkRunner.cpp
  src/Licensing/SimpleCrypt.cpp
  src/main.cpp
  src/AppState.cpp
  src/SessionContext.cpp
  src/SerialStudio.cpp
  src/SerialStudioFrameSupport.cpp
)

set(HEADERS
  src/Misc/ModuleManager.h
  src/Licensing/SimpleCrypt.h
  src/Misc/CLI.h
  src/Misc/CLI/CliBusConfig.h
  src/Misc/CLI/CliIndustrialConfig.h
  src/Misc/CLI/CliSpecParsers.h
  src/SelfTest/SelfTest.h
  src/ThirdParty/atomicops.h
  src/ThirdParty/fast_float.h
  src/ThirdParty/readerwriterqueue.h
  src/ThirdParty/readerwritercircularbuffer.h
  src/AppInfo.h
  src/AppState.h
  src/SessionContext.h
  src/SerialStudio.h
)

set(QML_SOURCES
  qml/AI/ProUpgradeNotice.qml
  qml/Commands/AppCommandBindings.qml
  qml/Commands/CommandModel.qml
  qml/Commands/DashboardCommandBindings.qml
  qml/Commands/ProjectEditorCommandBindings.qml
  qml/Commands/ProjectEditorMenuBindings.qml
  qml/Dialogs/About.qml
  qml/Dialogs/Acknowledgements.qml
  qml/Dialogs/AccelerometerConfigDialog.qml
  qml/Dialogs/Macros.qml
  qml/Dialogs/AxisRangeDialog.qml
  qml/Dialogs/Benchmark.qml
  qml/Dialogs/TriggerDialog.qml
  qml/Dialogs/BackupRecovery.qml
  qml/Dialogs/CrashRecovery.qml
  qml/Dialogs/PlayerDialog.qml
  qml/Dialogs/CsvPlayer.qml
  qml/Dialogs/Mdf4Player.qml
  qml/Dialogs/SqlitePlayer.qml
  qml/DatabaseExplorer/DatabaseCommandBindings.qml
  qml/DatabaseExplorer/DatabaseExplorer.qml
  qml/DatabaseExplorer/SessionList.qml
  qml/DatabaseExplorer/SessionDetail.qml
  qml/DatabaseExplorer/DriftReportDialog.qml
  qml/DatabaseExplorer/ReportOptionsDialog.qml
  qml/DatabaseExplorer/ReportProgressDialog.qml
  qml/Dialogs/Donate.qml
  qml/Dialogs/FileTransmission.qml
  qml/Dialogs/FrameParserTest.qml
  qml/Dialogs/IconPicker.qml
  qml/Dialogs/OnlineIconPicker.qml
  qml/Dialogs/Settings.qml
  qml/Dialogs/Settings/SettingsGeneralPage.qml
  qml/Dialogs/Settings/SettingsStartupPage.qml
  qml/Dialogs/Settings/SettingsPlottingPage.qml
  qml/Dialogs/Settings/SettingsLayoutPage.qml
  qml/Dialogs/Settings/SettingsTaskbarPage.qml
  qml/Dialogs/Settings/SettingsConsolePage.qml
  qml/Dialogs/Settings/SettingsExportPage.qml
  qml/Dialogs/Settings/SettingsNotificationsPage.qml
  qml/Dialogs/Welcome.qml
  qml/Dialogs/WorkspaceDialog.qml
  qml/Dialogs/ExtensionManager.qml
  qml/Dialogs/ExtensionManager/GridPage.qml
  qml/Dialogs/ExtensionManager/DetailPage.qml
  qml/Dialogs/ExtensionManager/ReposPage.qml
  qml/Dialogs/ExamplesBrowser.qml
  qml/Dialogs/HelpCenter.qml
  qml/Dialogs/ProblemCenter.qml
  qml/Dialogs/ExtensionConsent.qml
  qml/Dialogs/RemoteAttach.qml
  qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml
  qml/MainWindow/Panes/Dashboard/DashboardLayout.qml
  qml/MainWindow/Panes/Dashboard/ExternalWidgetWindow.qml
  qml/MainWindow/Panes/Dashboard/LayoutPatternPicker.qml
  qml/MainWindow/Panes/Dashboard/PaletteModel.qml
  qml/MainWindow/Panes/Dashboard/StartMenu.qml
  qml/MainWindow/Panes/Dashboard/Taskbar.qml
  qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml
  qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml
  qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml
  qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml
  qml/MainWindow/Panes/SetupPanes/Hardware.qml
  qml/MainWindow/Panes/Console.qml
  qml/MainWindow/Panes/Dashboard.qml
  qml/MainWindow/Panes/Setup.qml
  qml/MainWindow/Panes/Toolbar.qml
  qml/MainWindow/MainWindow.qml
  qml/ProjectEditor/Sections/ProjectStructure.qml
  qml/ProjectEditor/Sections/ProjectToolbar.qml
  qml/ProjectEditor/Views/ActionView.qml
  qml/ProjectEditor/Views/EditorNavActions.qml
  qml/ProjectEditor/Views/OutputWidgetView.qml
  qml/ProjectEditor/Views/DatasetView.qml
  qml/ProjectEditor/Views/MultiSelectionView.qml
  qml/ProjectEditor/Views/DataTablesView.qml
  qml/ProjectEditor/Views/SystemDatasetsView.qml
  qml/ProjectEditor/Views/UserTableView.qml
  qml/ProjectEditor/Views/WorkspacesView.qml
  qml/ProjectEditor/Views/WorkspaceView.qml
  qml/ProjectEditor/Views/AddWidgetDialog.qml
  qml/ProjectEditor/Views/ConstantsLibraryDialog.qml
  qml/ProjectEditor/Views/FlowDiagram.qml
  qml/ProjectEditor/Views/FlowDiagram/DiagramLayout.qml
  qml/ProjectEditor/Views/FlowDiagram/DiagramIcons.qml
  qml/ProjectEditor/Views/FlowDiagram/DiagramArrows.qml
  qml/ProjectEditor/Views/FlowDiagram/DiagramNode.qml
  qml/ProjectEditor/Views/FlowDiagram/DiagramMenuController.qml
  qml/ProjectEditor/Views/FlowDiagram/NodePill.qml
  qml/ProjectEditor/Views/FlowDiagram/NodeTransform.qml
  qml/ProjectEditor/Views/FlowDiagram/NodeCard.qml
  qml/ProjectEditor/Views/FlowDiagram/NodeFolder.qml
  qml/ProjectEditor/Views/NativeParserPane.qml
  qml/ProjectEditor/Views/GroupView.qml
  qml/ProjectEditor/Views/ProjectView.qml
  qml/ProjectEditor/Views/SourceView.qml
  qml/ProjectEditor/Views/SourceFrameParserView.qml
  qml/ProjectEditor/Views/CodeEditorMenu.qml
  qml/ProjectEditor/Views/ControlScriptView.qml
  qml/ProjectEditor/Views/TableDelegate.qml
  qml/ProjectEditor/Views/EditorSearchBand.qml
  qml/ProjectEditor/Views/WorkspaceFolderView.qml
  qml/ProjectEditor/Views/GroupsView.qml
  qml/ProjectEditor/Views/GroupFolderView.qml
  qml/ProjectEditor/Views/TableFolderView.qml
  qml/ProjectEditor/Views/GroupTemplateMenu.qml
  qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml
  qml/ProjectEditor/Dialogs/FrequencyMarkersEditor.qml
  qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml
  qml/ProjectEditor/ProjectEditor.qml
  qml/Widgets/Dashboard/Accelerometer.qml
  qml/Widgets/Dashboard/Bar.qml
  qml/Widgets/Dashboard/Clock.qml
  qml/Widgets/Dashboard/Compass.qml
  qml/Widgets/Dashboard/DashboardToolButton.qml
  qml/Widgets/Dashboard/BarPanel.qml
  qml/Widgets/Dashboard/DataGrid.qml
  qml/Widgets/Dashboard/ExtensionPlaceholder.qml
  qml/Widgets/Dashboard/ExtensionWidgetSettings.qml
  qml/Widgets/Dashboard/FFTPlot.qml
  qml/Widgets/Dashboard/Gauge.qml
  qml/Widgets/Dashboard/InstrumentBase.qml
  qml/Widgets/Dashboard/Meter.qml
  qml/Widgets/Dashboard/SwipePages.qml
  qml/Widgets/Dashboard/GPS.qml
  qml/Widgets/Dashboard/WebView.qml
  qml/Widgets/Dashboard/Gyroscope.qml
  qml/Widgets/Dashboard/LEDPanel.qml
  qml/Widgets/Dashboard/MultiPlot.qml
  qml/Widgets/Dashboard/Plot.qml
  qml/Widgets/Dashboard/PlotCommon.qml
  qml/Widgets/Dashboard/Stopwatch.qml
  qml/Widgets/Dashboard/ConsoleAnnotations.qml
  qml/Widgets/Dashboard/Terminal.qml
  qml/Widgets/Dashboard/WidgetTitleBar.qml
  qml/Widgets/Dashboard/WidgetToolbar.qml
  qml/Widgets/Dashboard/ValueFormat.js
  qml/Widgets/BigButton.qml
  qml/Widgets/BoundField.qml
  qml/Widgets/CommandMenu.qml
  qml/Widgets/CommandPalette.qml
  qml/Widgets/CommandToolbar.qml
  qml/Widgets/Combo.qml
  qml/Widgets/InfoBullet.qml
  qml/Widgets/LedIndicator.qml
  qml/Widgets/LineField.qml
  qml/Widgets/FileDropArea.qml
  qml/Widgets/IconButton.qml
  qml/Widgets/MarkdownIcons.js
  qml/Widgets/MarkdownTextView.qml
  qml/Widgets/MenuButton.qml
  qml/Widgets/MiniWindow.qml
  qml/Widgets/Pane.qml
  qml/Widgets/PaneSplitter.qml
  qml/Widgets/RibbonToolbar.qml
  qml/Widgets/RibbonSection.qml
  qml/Widgets/RibbonSpacer.qml
  qml/Widgets/PlotWidget.qml
  qml/Widgets/PlotWidget/PlotFormatter.qml
  qml/Widgets/PlotWidget/PlotInteraction.qml
  qml/Widgets/PlotWidget/PlotRulerOverlay.qml
  qml/Widgets/PlotWidget/PlotRulerMenu.qml
  qml/Widgets/PlotWidget/PlotMarkerPopup.qml
  qml/Widgets/PlotWidget/PlotCursor.qml
  qml/Widgets/PlotWidget/PlotTriggerLine.qml
  qml/Widgets/ProjectTableHeader.qml
  qml/Widgets/ProjectTableRow.qml
  qml/Widgets/ProNotice.qml
  qml/Widgets/SearchField.qml
  qml/Widgets/SegmentedControl.qml
  qml/Widgets/SmartWindow.qml
  qml/Widgets/SmartDialog.qml
  qml/Widgets/SubMenuCombo.qml
  qml/Widgets/TaskbarButton.qml
  qml/Widgets/ToolbarButton.qml
  qml/Widgets/VisualRange.qml
  qml/Widgets/WindowMirror.qml
  qml/DialogLoader.qml
  qml/main.qml
)

if(WITH_WEBENGINE)
  list(APPEND QML_SOURCES qml/Widgets/MarkdownWebView.qml)
  list(APPEND QML_SOURCES qml/Widgets/Dashboard/WebEngineSurface.qml)
endif()

if(BUILD_COMMERCIAL)
  set(HEADERS
    ${HEADERS}
    src/Licensing/Trial.h
    src/Licensing/MachineID.h
    src/Licensing/LemonSqueezy.h
    src/Licensing/CommercialToken.h
    src/Licensing/MonotonicClock.h
    src/Licensing/OfflinePublicKey.h
    src/Licensing/OfflineCertificate.h
    src/Licensing/OfflineLicense.h
    src/Licensing/OfflineSelfTest.h
    src/Licensing/GuardSelfTest.h
    src/ThirdParty/miniaudio.h
    src/ThirdParty/tweetnacl.h
    src/ThirdParty/ed25519_verify.h
  )
  set(SOURCES
    ${SOURCES}
    src/Licensing/Trial.cpp
    src/Licensing/CommercialToken.cpp
    src/ThirdParty/miniaudio.cpp
    src/Licensing/MachineID.cpp
    src/Licensing/LemonSqueezy.cpp
    src/Licensing/MonotonicClock.cpp
    src/Licensing/OfflineCertificate.cpp
    src/Licensing/OfflineLicense.cpp
    src/Licensing/OfflineSelfTest.cpp
    src/Licensing/GuardSelfTest.cpp
    src/ThirdParty/tweetnacl.c
    src/ThirdParty/ed25519_verify.cpp
  )

  set(QML_SOURCES
    ${QML_SOURCES}
    qml/Dialogs/LicenseManagement.qml
    qml/Dialogs/OfflineActivation.qml
    qml/Dialogs/ShortcutGenerator.qml
    qml/Dialogs/RuntimeReconfigure.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/UsbControlComposer.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml
    qml/Dialogs/OpcUaTrustDialog.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/OpcUa.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/OpcUaTagBrowser.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/S7.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/S7VariablesDialog.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/EthernetIp.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/DriverTagPickerDialog.qml
  qml/MainWindow/Panes/SetupPanes/Drivers/EipTagsDialog.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/Iec104.qml
    qml/ProjectEditor/Views/MqttPublisherView.qml
    qml/ProjectEditor/Views/InfluxSinkView.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml
    qml/Widgets/Dashboard/Plot3D.qml
    qml/Widgets/Dashboard/ImageView.qml
    qml/Widgets/Dashboard/NotificationLog.qml
    qml/Widgets/Dashboard/Painter.qml
    qml/Widgets/Dashboard/Waterfall.qml
    qml/ProjectEditor/Dialogs/PainterCodeDialog.qml
    qml/ProjectEditor/Dialogs/PainterTestDialog.qml
    qml/Widgets/Dashboard/Output/DashboardButton.qml
    qml/Widgets/Dashboard/Output/DashboardSlider.qml
    qml/Widgets/Dashboard/Output/DashboardToggle.qml
    qml/Widgets/Dashboard/Output/DashboardTextField.qml
    qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml
    qml/AI/AssistantPanel.qml
    qml/AI/AssistantPanel/AssistantTopBar.qml
    qml/AI/AssistantPanel/AssistantConversation.qml
    qml/AI/AssistantPanel/AssistantComposer.qml
    qml/AI/ChatSidebar.qml
    qml/AI/KeyManagerDialog.qml
    qml/AI/MemoryManagerDialog.qml
    qml/AI/MessageBubble.qml
    qml/AI/MessageList.qml
    qml/AI/ToolCallCard.qml
  )

  if(WITH_WEBENGINE)
    list(APPEND QML_SOURCES qml/AI/MessageWebView.qml)
  endif()
endif()

#---------------------------------------------------------------------------------------------------
# gRPC server support (optional)
#---------------------------------------------------------------------------------------------------
#
# When ENABLE_GRPC=ON (the packages themselves are found above, before add_subdirectory(core)):
#   - Compiles the static .proto file into C++ stubs
#   - Adds the gRPC server, proto generator, and conversion utilities
#   - Links gRPC and Protobuf libraries
#   - Defines ENABLE_GRPC preprocessor macro
#
# The gRPC server mirrors the existing TCP/JSON API on port 8888, using
# google.protobuf.Struct for dynamic command parameters.
#
#---------------------------------------------------------------------------------------------------

if(ENABLE_GRPC)
  get_target_property(GRPC_CPP_PLUGIN gRPC::grpc_cpp_plugin LOCATION)
  set(PROTO_FILE "${CMAKE_SOURCE_DIR}/doc/grpc/serialstudio.proto")
  set(PROTO_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/proto")
  file(MAKE_DIRECTORY ${PROTO_OUT_DIR})

  set(PROTO_GENERATED_SRCS
    "${PROTO_OUT_DIR}/serialstudio.pb.cc"
    "${PROTO_OUT_DIR}/serialstudio.grpc.pb.cc"
  )
  set(PROTO_GENERATED_HDRS
    "${PROTO_OUT_DIR}/serialstudio.pb.h"
    "${PROTO_OUT_DIR}/serialstudio.grpc.pb.h"
  )

  get_target_property(PROTOBUF_INCLUDE_DIR protobuf::libprotobuf INTERFACE_INCLUDE_DIRECTORIES)

  add_custom_command(
    OUTPUT ${PROTO_GENERATED_SRCS} ${PROTO_GENERATED_HDRS}
    COMMAND protobuf::protoc
    ARGS
      --proto_path "${CMAKE_SOURCE_DIR}/doc/grpc/"
      --proto_path "${PROTOBUF_INCLUDE_DIR}"
      --cpp_out "${PROTO_OUT_DIR}"
      --grpc_out "${PROTO_OUT_DIR}"
      --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
      "${PROTO_FILE}"
    DEPENDS "${PROTO_FILE}"
    COMMENT "Generating gRPC C++ stubs from ${PROTO_FILE}"
  )

  include_directories(${PROTO_OUT_DIR})

  if(MSVC)
    set_source_files_properties(${PROTO_GENERATED_SRCS}
      PROPERTIES COMPILE_FLAGS "/w"
    )
  else()
    set_source_files_properties(${PROTO_GENERATED_SRCS}
      PROPERTIES COMPILE_FLAGS "-w"
    )
  endif()

  set(SOURCES
    ${SOURCES}
    src/API/GRPC/GRPCServer.cpp
    src/API/GRPC/ProtoGenerator.cpp
    src/API/GRPC/ConversionUtils.cpp
    ${PROTO_GENERATED_SRCS}
  )

  set(HEADERS
    ${HEADERS}
    src/API/GRPC/GRPCServer.h
    src/API/GRPC/PendingCall.h
    src/API/GRPC/ProtoGenerator.h
    src/API/GRPC/ConversionUtils.h
    ${PROTO_GENERATED_HDRS}
  )

  #
  # Pipeline, Devices and Ui compile #ifdef ENABLE_GRPC regions, so they need the define; the first
  # two also include API/GRPC/GRPCServer.h inside those regions, which pulls grpcpp and the
  # generated serialstudio.grpc.pb.h. The protoc command stays with this target, so the archives
  # get a plain target to wait on rather than a second copy of the rule.
  #
  add_custom_target(ss_proto_generated DEPENDS ${PROTO_GENERATED_HDRS})

  foreach(_ss_grpc_lib SerialStudioPipeline SerialStudioDevices SerialStudioUi)
    target_compile_definitions(${_ss_grpc_lib} PRIVATE ENABLE_GRPC)
  endforeach()

  foreach(_ss_grpc_lib SerialStudioPipeline SerialStudioDevices)
    target_include_directories(${_ss_grpc_lib} PRIVATE ${PROTO_OUT_DIR})
    target_link_libraries(${_ss_grpc_lib} PUBLIC gRPC::grpc++ protobuf::libprotobuf)
    add_dependencies(${_ss_grpc_lib} ss_proto_generated)
  endforeach()
endif()

#---------------------------------------------------------------------------------------------------
# Platform-specific source files and resources
#---------------------------------------------------------------------------------------------------
#
# Adds the platform-specific resources each target OS bundles into the executable:
#
# Windows (WIN32):
#   - deploy/windows/info.rc: Application metadata (version, icon, manifest)
#
# macOS (APPLE):
#   - deploy/macOS/AppIcon.icns: Application icon bundle
#   - deploy/macOS/Assets.car: macOS UI assets (compiled asset catalog)
#   - deploy/macOS/info-*.plist: Bundle metadata (GPL or Pro variant)
#
# Linux (UNIX AND NOT APPLE):
#   - deploy/linux/*.desktop, installed by the deployment block further down
#
# The platform window decorations themselves (Platform/CSD.cpp, NativeWindow_CSD.cpp and the
# Objective-C++ NativeWindow_macOS.mm, whose unity-build exclusion moved with it) live in
# SerialStudio::Ui under the same per-OS gates.
#
# Platform differences:
#   - Windows: Native window frame via DWM, RC file for executable metadata
#   - macOS: NSWindow with custom title bar, Info.plist for bundle info
#   - Linux: Custom CSD for GNOME/KDE, .desktop file for app menu integration
#
#---------------------------------------------------------------------------------------------------

if(WIN32)
  set(SOURCES
    ${SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/deploy/windows/info.rc"
  )
  add_compile_definitions(
     WIN32_LEAN_AND_MEAN
     NOMINMAX
     _WINSOCKAPI_
  )
elseif(APPLE)
  if(BUILD_COMMERCIAL)
    set(INFO_MACOSX "${CMAKE_CURRENT_SOURCE_DIR}/deploy/macOS/info-pro.plist")
  else()
    set(INFO_MACOSX "${CMAKE_CURRENT_SOURCE_DIR}/deploy/macOS/info-gpl3.plist")
  endif()

  set(ICON_MACOSX "${CMAKE_CURRENT_SOURCE_DIR}/deploy/macOS/AppIcon.icns")
  set(ASSETS_MACOSX "${CMAKE_CURRENT_SOURCE_DIR}/deploy/macOS/Assets.car")

  set_source_files_properties(
    ${ICON_MACOSX}
    ${ASSETS_MACOSX}
    PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
  )

  set(SOURCES
    ${SOURCES}
    ${ICON_MACOSX}
    ${ASSETS_MACOSX}
  )
endif()

#---------------------------------------------------------------------------------------------------
# Qt resource compilation
#---------------------------------------------------------------------------------------------------
#
# Compiles Qt resource files (.qrc) into C++ code embedded in the executable:
#
# RES_RCC (rcc/rcc.qrc):
#   - Application assets: icons, images, fonts
#   - DataModel schemas and templates
#   - Default project files and examples
#   - Embedded resources accessible via qrc:/ URLs
#
# QTR_RCC (translations/translations.qrc):
#   - Compiled translation files (.qm)
#   - Languages: English, Spanish, Chinese, German, French, etc.
#   - Loaded at runtime based on system locale
#
# Why embed resources?
#   - Ensures assets are always available (no missing file errors)
#   - Simplifies deployment (single executable + Qt libs)
#   - Faster access than filesystem reads (mmap'd in memory)
#
# Note: Resources are compiled into the binary, increasing executable size
#
#---------------------------------------------------------------------------------------------------

qt_add_resources(RES_RCC ${CMAKE_CURRENT_SOURCE_DIR}/rcc/rcc.qrc)
qt_add_resources(QTR_RCC ${CMAKE_CURRENT_SOURCE_DIR}/translations/translations.qrc)

#---------------------------------------------------------------------------------------------------
# Executable target and library linking
#---------------------------------------------------------------------------------------------------
#
# Creates the main Serial Studio executable and links all dependencies:
#
# qt_add_executable():
#   - Creates the executable target with name ${PROJECT_EXECUTABLE}
#   - Includes all C++ sources, headers, QML, and resources
#   - MANUAL_FINALIZATION: Delays Qt-specific finalization to end of file
#     (allows setting properties before qt_finalize_executable)
#
# Linked libraries:
#   Qt modules: Core, Gui, Widgets, Qml, Quick, Graphs, SerialPort, etc.
#   Vendored (static):
#     - kissfft: Fast Fourier Transform (FFT plot calculations)
#     - OpenSSL: Cryptography/SSL/TLS, prebuilt (skipped when ENABLE_GRPC=ON -- gRPC's BoringSSL)
#     - libusb, hidapi: USB/HID device drivers (Pro only)
#     - gRPC, protobuf: gRPC server (when ENABLE_GRPC=ON)
#   Bundled:
#     - QCodeEditor: Syntax highlighting for DataModel/JavaScript editor
#     - QSimpleUpdater: Auto-update framework (checks for new versions)
#     - mdf: MDF4/MF4 file format parser (Pro: replay CAN/LIN data)
#
#---------------------------------------------------------------------------------------------------

qt_add_executable(
  ${PROJECT_EXECUTABLE}
  ${SOURCES}
  ${HEADERS}
  ${QML_RCC}
  ${RES_RCC}
  ${QTR_RCC}
  ${QT_RCC_TRANSLATIONS}
  ${APP_RCC_TRANSLATIONS}
  MANUAL_FINALIZATION
)

if(MSVC)
  target_compile_options(${PROJECT_EXECUTABLE} PRIVATE /W4)
endif()

#---------------------------------------------------------------------------------------------------
# Stereo stroke shaders (Pro)
#---------------------------------------------------------------------------------------------------
#
# The 3D plot's anaglyph passes need a material that narrows the pipeline's color-write mask, and
# a custom material needs its own shader stages. These are vendored rather than borrowed from Qt's
# built-in scene-graph set so the uniform block and the code that fills it are both ours and cannot
# drift when Qt reorganizes its shaders. BASE puts them at :/serial-studio.com/shaders/<name>.qsb,
# which is what Plot3DStereo.h names. BATCHABLE emits the rewritten vertex variant the batch
# renderer asks for when it merges nodes: without it a merged batch would request a variant the
# .qsb does not carry, fail to build its pipeline, and be dropped -- an invisible stereo pass.
#
#---------------------------------------------------------------------------------------------------

if(BUILD_COMMERCIAL)
  qt_add_shaders(${PROJECT_EXECUTABLE} "plot3d_eye_shaders"
    BATCHABLE
    PREFIX "/serial-studio.com/shaders"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    FILES
      shaders/plot3d_eye.vert
      shaders/plot3d_eye.frag
  )
endif()

target_link_libraries(
  ${PROJECT_EXECUTABLE} PUBLIC

  ${QT_LIBS}

  SerialStudio::Core
  SerialStudio::Protocols

  SerialStudio::Ui
  SerialStudio::Api
  SerialStudio::Storage
  SerialStudio::Devices
  SerialStudio::Pipeline

  luajit
  kissfft
  QCodeEditor
  QSimpleUpdater
  mdf
)

target_link_openssl(${PROJECT_EXECUTABLE})
target_link_open62541(${PROJECT_EXECUTABLE})
target_link_libplctag(${PROJECT_EXECUTABLE})
target_link_mimalloc(${PROJECT_EXECUTABLE})

if(WIN32 AND MSVC)
  add_custom_command(TARGET ${PROJECT_EXECUTABLE} POST_BUILD
    COMMAND mt.exe -nologo
            -manifest "${CMAKE_CURRENT_SOURCE_DIR}/deploy/windows/serial-studio.manifest"
            "-outputresource:$<TARGET_FILE:${PROJECT_EXECUTABLE}>;#1"
    COMMENT "Embedding the Serial Studio application manifest"
    VERBATIM)
endif()

if(BUILD_COMMERCIAL)
  target_link_libraries(${PROJECT_EXECUTABLE} PUBLIC usb-1.0)
  target_link_libraries(${PROJECT_EXECUTABLE} PUBLIC hidapi::hidapi)
endif()

if(SS_INAPP_TESTS)
  target_compile_definitions(${PROJECT_EXECUTABLE} PRIVATE SS_INAPP_TESTS)
endif()

if(ENABLE_GRPC)
  target_link_libraries(${PROJECT_EXECUTABLE} PUBLIC
    gRPC::grpc++
    protobuf::libprotobuf
  )
  target_compile_definitions(${PROJECT_EXECUTABLE} PRIVATE ENABLE_GRPC)
endif()

#---------------------------------------------------------------------------------------------------
# Winsock (Windows)
#---------------------------------------------------------------------------------------------------
#
# The Network driver's TCP reachability probe calls socket/connect/WSAPoll/getaddrinfo directly
# rather than through QTcpSocket, so it needs ws2_32 explicitly: Qt keeps it as a private
# dependency and does not propagate it to consumers.

if(WIN32)
  target_link_libraries(${PROJECT_EXECUTABLE} PRIVATE ws2_32)
endif()

#---------------------------------------------------------------------------------------------------
# Executable-specific hardening flags (Linux)
#---------------------------------------------------------------------------------------------------
#
# Applies security hardening link flags directly to the executable:
#
# Why target_link_options and not add_link_options?
#   - These flags are only valid for executables, not static libraries
#   - Applying them globally would break library builds
#   - target_link_options ensures they only apply to the final executable
#
# Hardening flags (Linux only):
#   -Wl,-z,relro      - Make GOT (Global Offset Table) read-only after relocation
#   -Wl,-z,now        - Resolve all symbols at startup (full RELRO)
#   -Wl,-z,noexecstack - Mark stack memory as non-executable (NX bit)
#   -pie              - Position Independent Executable (required for ASLR)
#
# Why Linux only?
#   - macOS uses different linker (ld64) with different flags
#   - Windows hardening is handled via /DYNAMICBASE and /NXCOMPAT
#   - These are GNU ld-specific flags
#
# Security benefits:
#   - RELRO: Prevents GOT overwrites (common exploit technique)
#   - NX stack: Prevents shellcode execution on stack
#   - PIE: Enables ASLR (randomizes executable base address)
#
#---------------------------------------------------------------------------------------------------

if(ENABLE_HARDENING AND UNIX AND NOT APPLE)
  target_link_options(
    ${PROJECT_EXECUTABLE} PRIVATE
    -Wl,-z,relro
    -Wl,-z,now
    -Wl,-z,noexecstack
    -pie
  )
endif()

#---------------------------------------------------------------------------------------------------
# Per-target hardening deltas + code signing
#---------------------------------------------------------------------------------------------------
#
# serial_studio_harden(): MSVC/Clang/Apple hardening flags that must be PRIVATE to the executable
# (so lib/ static libraries built with -w are unaffected). serial_studio_sign(): POST_BUILD code
# signing, a no-op unless ENABLE_CODE_SIGNING and the platform's signing cache vars are set.
# Both are defined in cmake/Hardening.cmake and cmake/Signing.cmake (included from the root).
#
#---------------------------------------------------------------------------------------------------

serial_studio_harden(${PROJECT_EXECUTABLE})
serial_studio_sign(${PROJECT_EXECUTABLE})

#---------------------------------------------------------------------------------------------------
# QML module compilation and caching
#---------------------------------------------------------------------------------------------------
#
# Compiles QML files into a Qt Quick module with ahead-of-time compilation:
#
# qt_add_qml_module() benefits:
#   - QML type registration: Makes C++ types available in QML automatically
#   - QML caching: Pre-compiles QML to bytecode for faster startup
#   - Type safety: Enables QML linter (qmllint) and type checking
#   - Import resolution: Manages QML import paths and dependencies
#
# Configuration:
#   URI: gui - QML module identifier (import gui 1.0)
#   VERSION: 1.0 - Module version for QML imports
#   QML_FILES: ${QML_SOURCES} - All .qml files in qml/ directory
#   RESOURCE_PREFIX: /serial-studio.com/ - Qt resource path prefix
#
# QML file categories:
#   - qml/MainWindow/: Main window and toolbar
#   - qml/Dialogs/: Modal dialogs (About, Settings, etc.)
#   - qml/Widgets/: Reusable UI widgets
#   - qml/ProjectEditor/: DataModel project editor UI
#   - qml/Widgets/Dashboard/: Dashboard widget implementations
#
# Performance impact:
#   - ~30-40% faster startup time vs. runtime QML compilation
#   - Smaller memory footprint (cached bytecode)
#   - Better error detection at build time
#
#---------------------------------------------------------------------------------------------------

set_source_files_properties(qml/Widgets/Dashboard/ValueFormat.js qml/Widgets/MarkdownIcons.js
  PROPERTIES QT_QML_SKIP_QMLDIR_ENTRY FALSE)

qt_add_qml_module(
  ${PROJECT_EXECUTABLE}
  URI gui
  VERSION 1.0
  QML_FILES ${QML_SOURCES}
  RESOURCE_PREFIX /serial-studio.com/
)

#---------------------------------------------------------------------------------------------------
# QML lint target
#---------------------------------------------------------------------------------------------------
#
# qt_add_qml_module() already generates <target>_qmllint when Qt's qmllint is installed, but its
# name depends on the build flavour (serial-studio-pro vs serial-studio-gpl3). `ss_qmllint` is
# the stable name CI builds, so the workflow does not have to know which flavour it configured.
# Nothing ran qmllint before spec 0075 (finding G12 / M7): unqualified property access and
# binding loops were caught only by a human noticing the runtime warning.
#
#---------------------------------------------------------------------------------------------------

if(TARGET ${PROJECT_EXECUTABLE}_qmllint)
  add_custom_target(ss_qmllint)
  add_dependencies(ss_qmllint ${PROJECT_EXECUTABLE}_qmllint)
else()
  message(STATUS "qmllint not available; ss_qmllint target not created")
endif()

#---------------------------------------------------------------------------------------------------
# MiniAudio audio input library configuration (Pro only)
#---------------------------------------------------------------------------------------------------
#
# Configures miniaudio.h for audio device input in commercial builds:
#
# When BUILD_COMMERCIAL=ON:
#
# Disabled features (not needed for audio input):
#   MA_NO_DECODING         - Disable audio file decoding (WAV, MP3, FLAC)
#   MA_NO_ENCODING         - Disable audio file encoding
#   MA_NO_WAV/FLAC/MP3     - Disable codec support (only need raw PCM)
#   MA_NO_RESOURCE_MANAGER - Disable resource loading system
#   MA_NO_NODE_GRAPH       - Disable audio graph processing
#   MA_NO_ENGINE           - Disable high-level engine API
#   MA_NO_GENERATION       - Disable waveform generation
#   MA_NO_CUSTOM           - Disable custom backend support
#   MA_NO_NULL             - Disable null audio backend
#
# Platform-specific configuration:
#   Linux: Link against libasound (ALSA) for audio input
#   Windows: Uses WASAPI (built-in, no extra libs needed)
#   macOS: Uses CoreAudio (built-in, no extra libs needed)
#
# SIMD optimizations:
#   x86-64: MA_SUPPORT_SSE2 - Vectorized audio processing
#   ARM64:  MA_SUPPORT_NEON - NEON SIMD for faster processing
#           (Not on Apple Silicon - handled differently)
#
# Why these settings?
#   - Reduces binary size by ~200KB (disabling unused features)
#   - Faster compilation (less template instantiation)
#   - Only need raw PCM capture for oscilloscope/FFT features
#
#---------------------------------------------------------------------------------------------------

if (BUILD_COMMERCIAL)
  ss_apply_miniaudio_definitions(${PROJECT_EXECUTABLE})

  if(MSVC)
    set_source_files_properties(src/ThirdParty/miniaudio.cpp
      PROPERTIES COMPILE_OPTIONS "/wd4505"
    )
  else()
    set_source_files_properties(src/ThirdParty/miniaudio.cpp
      PROPERTIES COMPILE_OPTIONS "-Wno-unused-function"
    )
  endif()

  if(LINUX)
    target_link_libraries(${PROJECT_EXECUTABLE} PRIVATE asound)
  endif()
endif()

#---------------------------------------------------------------------------------------------------
# TweetNaCl (offline license Ed25519 verification, Pro only)
#---------------------------------------------------------------------------------------------------
#
# tweetnacl.c is vendored byte-verbatim from the canonical public-domain release
# (v20140427, https://tweetnacl.cr.yp.to/) and compiled as C -- the only reason the
# project() call above enables the C language. It cannot compile as C++: its
# zero-initialized `static const` tables (_0, gf0) have no initializers and sigma[16]
# is initialized from a 17-byte string literal, both legal C but hard errors in C++.
# Blanket-suppress warnings on the vendored code (it is not held to the project's
# strict flags) and keep it out of any unity build, since its short file-local helper
# names (A, M, add, pack, ...) would collide with other translation units. On cl.exe,
# serial_studio_harden's /sdl promotes C4146 (constant-time masking, e.g. `-(h[16]>>7)`)
# to an error that /w cannot demote, so this file alone gets /sdl- (source-level
# options come after target options and win); clang-cl never receives /sdl.
#
#---------------------------------------------------------------------------------------------------

if(BUILD_COMMERCIAL)
  if(MSVC)
    if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
      set_source_files_properties(src/ThirdParty/tweetnacl.c PROPERTIES COMPILE_OPTIONS "/w")
    else()
      set_source_files_properties(src/ThirdParty/tweetnacl.c
        PROPERTIES COMPILE_OPTIONS "/w;/sdl-"
      )
    endif()
  else()
    set_source_files_properties(src/ThirdParty/tweetnacl.c PROPERTIES COMPILE_OPTIONS "-w")
  endif()

  set_source_files_properties(src/ThirdParty/tweetnacl.c
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
  )
endif()

#---------------------------------------------------------------------------------------------------
# Unity (jumbo) build -- opt-in, used by CI
#---------------------------------------------------------------------------------------------------
#
# SS_UNITY_BUILD=ON batches this target's C++ sources into unity TUs to cut compile
# time; CI compiles the application twice per platform (PGO generate + use) with no
# compiler cache, so the savings land on every push. SS_UNITY_BATCH_SIZE (cache var,
# default 8) sets sources per TU: it amortizes header parsing while keeping enough TUs
# for the build to parallelize (batch 0 -- one monolithic jumbo TU -- serializes the
# whole target on one compiler process, so keep the TU count above the runner's core
# count). CI passes 32 via the SS_UNITY_BATCH workflow env var, sized for the
# post-spec-0070 source count. Default is OFF: local builds, the ss_unit_tests tier,
# and every lib/ third-party target are unaffected. Both PGO stages must configure
# with the same value -- the unity layout determines the object-file <-> profile-data
# mapping, and it has to be identical across stages.
#
# /bigobj: a unity TU on the MSVC ABI (cl.exe and clang-cl) can exceed the default
# COFF section limit; the flag lifts it and has no codegen effect.
#
# Exclusions (SKIP_UNITY_BUILD_INCLUSION) are generated/vendored TUs only:
#   - rcc-generated TUs: every rcc output defines the same file-local symbols
#     (qt_resource_data/name/struct), so no two of them may share the jumbo TU.
#   - src/ThirdParty/*.cpp: vendored single-file libraries full of file-local
#     symbols. (miniaudio.cpp, tweetnacl.c and the proto-generated TUs are already
#     excluded implicitly -- per-source COMPILE_OPTIONS/COMPILE_FLAGS opt a file out
#     of unity -- but they are pinned here so a warning-flag cleanup cannot silently
#     pull ~90k-line miniaudio into a batch. NativeWindow_macOS.mm is skipped in
#     core/Ui, which now owns it.)
#
# First-party TUs are NOT excluded: every file-local name is unique app-wide, verified
# by compiling the whole target as ONE monolithic jumbo TU (batch size 0) on 2026-08-20
# (shared logDriverError moved into HAL_Driver, colliding helpers/constants renamed with
# subsystem prefixes, file-scope `using namespace` directives contained in namespace
# blocks). A "redefinition"/"ambiguous"/-Wshadow diagnostic inside a unity_*_cxx.cxx TU
# means a NEW collision: rename the file-local helper or contain the directive -- do not
# add the file to the exclusion list. To re-verify the full closure, build once with
# -DSS_UNITY_BATCH_SIZE=0: batching only hides collisions, it never removes them.
#
#---------------------------------------------------------------------------------------------------

if(SS_UNITY_BUILD)
  set_target_properties(${PROJECT_EXECUTABLE} PROPERTIES
    UNITY_BUILD ON
    UNITY_BUILD_BATCH_SIZE ${SS_UNITY_BATCH_SIZE}
  )

  if(MSVC)
    target_compile_options(${PROJECT_EXECUTABLE} PRIVATE /bigobj)
  endif()

  set_source_files_properties(${RES_RCC} ${QTR_RCC}
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
  )

  set_source_files_properties(
    src/ThirdParty/ed25519_verify.cpp
    src/ThirdParty/miniaudio.cpp
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
  )
endif()

#---------------------------------------------------------------------------------------------------
# Platform-specific deployment configuration
#---------------------------------------------------------------------------------------------------
#
# Configures executable properties and installation for each platform:
#
# Windows (WIN32):
#   - Links Dwmapi.lib: Desktop Window Manager API (for custom window frame)
#   - WIN32_EXECUTABLE=TRUE: GUI application (no console window)
#
# macOS (APPLE):
#   - MACOSX_BUNDLE=TRUE: Creates .app bundle (required for macOS apps)
#   - MACOSX_BUNDLE_ICON_FILE: Sets AppIcon.icns as bundle icon
#   - MACOSX_BUNDLE_INFO_PLIST: Uses Info.plist for bundle metadata
#
# Linux (UNIX):
#   - Installs executable to /usr/bin or /usr/local/bin
#   - Installs .svg icon to /usr/share/pixmaps (for app launchers)
#   - Installs .metainfo.xml to /usr/share/metainfo (AppStream metadata)
#   - Installs .desktop file to /usr/share/applications (app menu entry)
#
# Deployment differences:
#   Windows: Single .exe with Qt DLLs in same directory
#   macOS:   .app bundle with embedded frameworks
#   Linux:   Executable + separate Qt shared libraries in system paths
#
# Linux metadata files:
#   - .desktop: Name, icon, categories, command to launch app
#   - .metainfo.xml: Screenshots, description, changelog for app stores
#   - .svg: Scalable icon for HiDPI displays
#
#---------------------------------------------------------------------------------------------------

if(WIN32)
  target_link_libraries(${PROJECT_EXECUTABLE} PRIVATE
    Dwmapi.lib Shell32.lib Advapi32.lib Ole32.lib Uuid.lib Propsys.lib)
  set_target_properties(${PROJECT_EXECUTABLE} PROPERTIES
    WIN32_EXECUTABLE TRUE
  )
elseif(APPLE)
  target_link_libraries(${PROJECT_EXECUTABLE} PRIVATE "-framework IOKit")
  set_target_properties(
    ${PROJECT_EXECUTABLE} PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_ICON_FILE ${ICON_MACOSX}
    MACOSX_BUNDLE_INFO_PLIST ${INFO_MACOSX}
  )
elseif(UNIX)
  install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/deploy/linux/${PROJECT_EXECUTABLE}.svg
    DESTINATION share/pixmaps
  )
  install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/deploy/linux/serial-studio.metainfo.xml
    DESTINATION share/metainfo
  )
  install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/deploy/linux/${PROJECT_EXECUTABLE}.desktop
    DESTINATION share/applications
  )
  install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/deploy/linux/serial-studio-ssproj.xml
    DESTINATION share/mime/packages
  )
endif()

#---------------------------------------------------------------------------------------------------
# Commercial binary hardening — symbol stripping and visibility
#---------------------------------------------------------------------------------------------------
#
# In Release builds of the commercial edition, strip non-essential symbols from the final binary.
# Also sets default symbol visibility to hidden so only Qt MOC-required exports remain in the
# dynamic symbol table.
#---------------------------------------------------------------------------------------------------

if(BUILD_COMMERCIAL AND PRODUCTION_OPTIMIZATION)
  set_target_properties(${PROJECT_EXECUTABLE} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
  )

  if(APPLE)
    add_custom_command(TARGET ${PROJECT_EXECUTABLE} POST_BUILD
      COMMAND ${CMAKE_STRIP} -x "$<TARGET_FILE:${PROJECT_EXECUTABLE}>"
      COMMENT "Stripping local symbols from commercial binary (macOS)"
    )
  elseif(UNIX)
    add_custom_command(TARGET ${PROJECT_EXECUTABLE} POST_BUILD
      COMMAND ${CMAKE_STRIP} --strip-unneeded "$<TARGET_FILE:${PROJECT_EXECUTABLE}>"
      COMMENT "Stripping unneeded symbols from commercial binary (Linux)"
    )
  elseif(WIN32)
    target_link_options(${PROJECT_EXECUTABLE} PRIVATE
      $<$<BOOL:${MSVC}>:/DEBUG:NONE>
    )
  endif()
endif()

#---------------------------------------------------------------------------------------------------
# Qt automatic deployment (macdeployqt, windeployqt, linuxdeploy)
#---------------------------------------------------------------------------------------------------
#
# Finalizes the Qt executable and generates deployment scripts:
#
# qt_finalize_executable():
#   - Finalizes Qt-specific setup (resource compilation, MOC, etc.)
#   - Must be called after all target properties are set
#   - Required when using MANUAL_FINALIZATION in qt_add_executable
#
# install() targets:
#   BUNDLE:  macOS .app bundles → install to top-level directory
#   RUNTIME: Executables → install to bin/ directory
#
# qt_generate_deploy_qml_app_script():
#   Generates platform-specific deployment script to bundle Qt dependencies
#
# Platform-specific deployment options:
#
#   macOS (macdeployqt):
#     -hardened-runtime     - Enable Hardened Runtime for notarization
#     -appstore-compliant   - Strip unsupported frameworks for App Store
#     MACOS_BUNDLE_POST_BUILD - Run deployment automatically after build
#
#   Windows (windeployqt):
#     -force-openssl        - Force inclusion of OpenSSL DLLs
#     --release             - Deploy release Qt DLLs (not debug)
#     --no-translations     - Exclude Qt translations (we embed our own)
#     --no-compiler-runtime - Skip windeployqt's CRT handling; the MSVC runtime
#                             DLLs are bundled app-local via InstallRequiredSystemLibraries
#
#   Linux:
#     NO_UNSUPPORTED_PLATFORM_ERROR - Don't fail on Linux (manual deployment)
#     DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM - Deploy QML modules
#
# Deployment script:
#   - Automatically copies Qt libraries, plugins, QML modules
#   - Runs at install time: cmake --install build --prefix /path
#   - Creates a self-contained deployment package
#
#---------------------------------------------------------------------------------------------------

qt_finalize_executable(${PROJECT_EXECUTABLE})

# Every DESTINATION below reads CMAKE_INSTALL_BINDIR, which was only ever defined because a
# vendored lib/ subdirectory includes GNUInstallDirs before app/ is processed. Say it here.
include(GNUInstallDirs)

install(
  TARGETS ${PROJECT_EXECUTABLE}
  BUNDLE  DESTINATION .
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(MSVC)
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP ON)
  include(InstallRequiredSystemLibraries)
  install(
    FILES       ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
endif()

set(SS_PACKAGE_TYPE "" CACHE STRING "Package-type stamp for ss-config.json (CI use only)")
if(SS_PACKAGE_TYPE)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
    set(SS_PACKAGE_ARCH "arm64")
  else()
    set(SS_PACKAGE_ARCH "x86_64")
  endif()
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/ss-config.json
    "{\"packageType\":\"${SS_PACKAGE_TYPE}\",\"arch\":\"${SS_PACKAGE_ARCH}\"}\n")
  install(
    FILES       ${CMAKE_CURRENT_BINARY_DIR}/ss-config.json
    DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
endif()

set(deploy_tool_options_arg "")
if(APPLE)
  set(deploy_tool_options_arg "-hardened-runtime -appstore-compliant")
elseif(WIN32)
  set(deploy_tool_options_arg "-force-openssl --release --no-translations --no-compiler-runtime")
endif()

qt_generate_deploy_qml_app_script(
  TARGET ${PROJECT_EXECUTABLE}
  OUTPUT_SCRIPT deploy_script
  MACOS_BUNDLE_POST_BUILD
  NO_UNSUPPORTED_PLATFORM_ERROR
  DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
  DEPLOY_TOOL_OPTIONS ${deploy_tool_options_arg}
)

install(SCRIPT ${deploy_script})

#---------------------------------------------------------------------------------------------------
# CPack installer/package generation
#---------------------------------------------------------------------------------------------------
#
# Configures CPack to generate platform-specific installers:
#
# Common package metadata (all platforms):
#   CPACK_MONOLITHIC_INSTALL    - Single package (not component-based)
#   CPACK_PACKAGE_NAME          - "Serial Studio GPLv3" or "Serial Studio Pro"
#   CPACK_PACKAGE_VENDOR        - "Alex Spataru"
#   CPACK_PACKAGE_VERSION       - Semantic version (e.g., 4.1.0)
#   CPACK_PACKAGE_DESCRIPTION   - README.md content
#   CPACK_RESOURCE_FILE_LICENSE - GPL-3.0-or-later.txt or Commercial license
#   CPACK_PACKAGE_HOMEPAGE_URL  - Website for support/downloads
#
# Platform-specific package generators:
#
#   Windows (WIX):
#     - Creates .msi installer using WiX Toolset
#     - CPACK_WIX_PRODUCT_ICON: Installer icon (icon.ico)
#     - CPACK_WIX_UI_BANNER: Top banner in installer wizard (493x58)
#     - CPACK_WIX_UI_DIALOG: Installer dialog background (493x312)
#     - CPACK_WIX_LICENSE_RTF: License agreement displayed during install
#     - CPACK_WIX_UPGRADE_GUID: Unique ID for upgrade detection
#       (same GUID = upgrade replaces old version, different = side-by-side)
#     - CPACK_WIX_ARCHITECTURE: x64 only (no 32-bit support)
#     - CPACK_WIX_PROGRAM_MENU_FOLDER: Start Menu folder (. = no subfolder)
#
#   macOS (DragNDrop):
#     - Creates .dmg disk image
#     - User drags .app bundle to /Applications
#     - No installer wizard (simple drag-and-drop)
#
#   Linux (TGZ):
#     - Creates .tar.gz archive
#     - Users extract and run binary manually
#     - For distro-specific packages, use system package tools
#       (e.g., checkinstall, debuild, rpmbuild)
#
# To generate installer:
#   cmake --build build --target package
#   Creates: Serial-Studio-GPL3-v4.0.3.msi (Windows)
#            Serial-Studio-GPL3-v4.0.3.dmg (macOS)
#            Serial-Studio-GPL3-v4.0.3.tar.gz (Linux)
#
#---------------------------------------------------------------------------------------------------

set(CPACK_MONOLITHIC_INSTALL              ON)
set(CPACK_PACKAGE_NAME                    ${PROJECT_DISPNAME})
set(CPACK_PACKAGE_VENDOR                  ${PROJECT_VENDOR})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY     ${PROJECT_DESCRIPTION_SUMMARY})
set(CPACK_PACKAGE_VERSION_MAJOR           ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR           ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH           ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION                 ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_FILE        ${PROJECT_DESCRIPTION_FILE})
set(CPACK_PACKAGE_FILE_NAME               ${PROJECT_FILE_NAME})
set(CPACK_RESOURCE_FILE_LICENSE           ${PROJECT_FILE_LICENSE})
set(CPACK_PACKAGE_HOMEPAGE_URL            ${PROJECT_CONTACT})

if(WIN32)
  set(CPACK_PACKAGE_ICON                 "${CMAKE_CURRENT_SOURCE_DIR}\\\\deploy\\\\windows\\\\icon.ico")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY    "${CPACK_PACKAGE_NAME}")
else()
  set(CPACK_PACKAGE_ICON                 "${CMAKE_CURRENT_SOURCE_DIR}/deploy/PackageIcon.png")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY    "${CPACK_PACKAGE_NAME}")
endif()

if(NOT CPACK_GENERATOR)
  if(WIN32)
    set(CPACK_GENERATOR                  "WIX")
    set(CPACK_WIX_PRODUCT_ICON           "${CMAKE_CURRENT_SOURCE_DIR}\\\\deploy\\\\windows\\\\icon.ico")
    set(CPACK_WIX_UI_BANNER              "${CMAKE_CURRENT_SOURCE_DIR}\\\\deploy\\\\windows\\\\banner.bmp")
    set(CPACK_WIX_UI_DIALOG              "${CMAKE_CURRENT_SOURCE_DIR}\\\\deploy\\\\windows\\\\dialog.bmp")
    set(CPACK_WIX_LICENSE_RTF            "${CMAKE_CURRENT_SOURCE_DIR}\\\\deploy\\\\windows\\\\license.rtf")
    set(CPACK_WIX_PROPERTY_ARPHELPLINK   "${PROJECT_CONTACT}")
    set(CPACK_WIX_PROPERTY_ARPCONTACT    "${PROJECT_CONTACT}")
    set(CPACK_WIX_UPGRADE_GUID           "a21bfa96-8805-4c47-8fa7-3b3daee6efce")
    set(CPACK_WIX_PROGRAM_MENU_FOLDER    ".")
    set(CPACK_WIX_ARCHITECTURE           "x64")
    set(CPACK_PACKAGE_EXECUTABLES        ${PROJECT_EXECUTABLE} "${PROJECT_DISPNAME}")
  elseif(APPLE)
    set(CPACK_GENERATOR                  "DragNDrop")
  else()
    set(CPACK_GENERATOR                  "TGZ")
  endif()
endif()

include(CPack)

if(SS_BUILD_TESTS)
  add_subdirectory(tests)
endif()
