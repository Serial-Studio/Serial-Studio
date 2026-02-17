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
project(app LANGUAGES CXX VERSION ${PROJECT_VERSION})

#---------------------------------------------------------------------------------------------------
# Qt automation and C++ standard
#---------------------------------------------------------------------------------------------------
#
# Qt Meta-Object Compiler (MOC) automation:
#   CMAKE_AUTOMOC=ON  - Auto-run moc on files with Q_OBJECT/Q_GADGET
#   CMAKE_AUTOUIC=ON  - Auto-compile .ui files to ui_*.h headers
#   CMAKE_AUTORCC=ON  - Auto-compile .qrc resource files to C++ code
#
# C++ standard:
#   CMAKE_CXX_STANDARD=20 - Use C++20 features (modules, concepts, ranges)
#   CMAKE_CXX_STANDARD_REQUIRED=ON - Hard requirement (no fallback to C++17)
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
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
#   Qt6::SerialPort    - RS-232/RS-485 serial communication
#   Qt6::Core5Compat   - Qt 5 compatibility (TextCodec, etc.)
#   Qt6::LinguistTools - Internationalization (lupdate, lrelease)
#
# Commercial-only modules:
#   Qt6::Mqtt          - MQTT client for IoT protocols
#   Qt6::SerialBus     - CAN Bus, Modbus TCP/RTU support
#
# Qt policies:
#   QTP0001=NEW - Use new Qt resource system (qt_add_resources)
#   QTP0002=NEW - Use new Qt translation system (qt_add_translations)
#   QTP0004=NEW - Use new Qt QML module system (qt_add_qml_module)
#
#---------------------------------------------------------------------------------------------------

# Specify required modules for FOSS version
set(QT_MODULES
  Svg
  Gui
  Qml
  Sql
  Core
  Quick
  Graphs
  Widgets
  Bluetooth
  SerialPort
  Core5Compat
  LinguistTools
  QuickControls2
)

# Specify required libraries for FOSS version
set(QT_LIBS
  Qt6::Core
  Qt6::Svg
  Qt6::Sql
  Qt6::Gui
  Qt6::Qml
  Qt6::Quick
  Qt6::Graphs
  Qt6::Widgets
  Qt6::Bluetooth
  Qt6::SerialPort
  Qt6::Core5Compat
  Qt6::QuickControls2
)

# Add required modules for Pro version if needed
if(BUILD_COMMERCIAL)
  set(QT_MODULES
    ${QT_MODULES}
    Mqtt
    SerialBus
  )
  set(QT_LIBS
    ${QT_LIBS}
    Qt6::Mqtt
    Qt6::SerialBus
  )
endif()

# Find the Qt package and its modules
find_package(
  Qt6 REQUIRED
  COMPONENTS
  ${QT_MODULES}
)

# Qt project setup & policies
qt_standard_project_setup()
qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0002 NEW)
qt_policy(SET QTP0004 NEW)

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
# Source file categories:
#   - Misc/: Utilities (ThemeManager, Translator, ModuleManager)
#   - UI/: Dashboard widgets and window management
#   - IO/: Hardware drivers (UART, Network, BluetoothLE, etc.)
#   - DataModel/: Frame parsing and project model
#   - CSV/: CSV file playback and export
#   - MDF4/: MDF4 file playback (Pro feature)
#   - MQTT/: MQTT client (Pro only)
#   - Licensing/: License validation (Pro only)
#   - Platform/: Platform-specific code (CSD, native windows)
#
# Commercial-only sources are conditionally added based on BUILD_COMMERCIAL
#
# Note: include_directories(src) makes all headers accessible without
# relative paths (e.g., #include "IO/Manager.h" instead of "../IO/Manager.h")
#
#---------------------------------------------------------------------------------------------------

include_directories(src)

# Specify required sources for FOSS version
set(SOURCES
  src/Misc/ThemeManager.cpp
  src/Misc/CommonFonts.cpp
  src/Misc/Utilities.cpp
  src/Misc/Translator.cpp
  src/Misc/ModuleManager.cpp
  src/Misc/TimerEvents.cpp
  src/Misc/WorkspaceManager.cpp
  src/UI/DashboardWidget.cpp
  src/UI/Dashboard.cpp
  src/UI/Taskbar.cpp
  src/UI/WindowManager.cpp
  src/UI/WidgetRegistry.cpp
  src/UI/Widgets/LEDPanel.cpp
  src/UI/Widgets/Gauge.cpp
  src/UI/Widgets/Plot.cpp
  src/UI/Widgets/Compass.cpp
  src/UI/Widgets/Bar.cpp
  src/UI/Widgets/FFTPlot.cpp
  src/UI/Widgets/Accelerometer.cpp
  src/UI/Widgets/DataGrid.cpp
  src/UI/Widgets/Terminal.cpp
  src/UI/Widgets/Gyroscope.cpp
  src/UI/Widgets/GPS.cpp
  src/UI/Widgets/MultiPlot.cpp
  src/UI/DeclarativeWidgets/DeclarativeWidget.cpp
  src/UI/DeclarativeWidgets/StaticTable.cpp
  src/API/Server.cpp
  src/API/CommandRegistry.cpp
  src/API/CommandHandler.cpp
  src/API/MCPHandler.cpp
  src/API/PathPolicy.cpp
  src/API/Handlers/IOManagerHandler.cpp
  src/API/Handlers/UARTHandler.cpp
  src/API/Handlers/NetworkHandler.cpp
  src/API/Handlers/BluetoothLEHandler.cpp
  src/API/Handlers/CSVExportHandler.cpp
  src/API/Handlers/ProjectHandler.cpp
  src/API/Handlers/ConsoleHandler.cpp
  src/API/Handlers/CSVPlayerHandler.cpp
  src/API/Handlers/DashboardHandler.cpp
  src/IO/Drivers/Network.cpp
  src/IO/Drivers/UART.cpp
  src/IO/Drivers/BluetoothLE.cpp
  src/IO/Checksum.cpp
  src/Console/Handler.cpp
  src/IO/Manager.cpp
  src/Console/Export.cpp
  src/IO/FileTransmission.cpp
  src/IO/FrameReader.cpp
  src/DataModel/FrameParser.cpp
  src/DataModel/ProjectModel.cpp
  src/DataModel/FrameBuilder.cpp
  src/DataModel/Frame.cpp
  src/DataModel/FrameConsumer.cpp
  src/DataModel/FrameParserTestDialog.cpp
  src/CSV/Player.cpp
  src/CSV/Export.cpp
  src/MDF4/Player.cpp
  src/MDF4/Export.cpp
  src/main.cpp
  src/SerialStudio.cpp
)

# Specify required headers for FOSS version
set(HEADERS
  src/Misc/ModuleManager.h
  src/Misc/Utilities.h
  src/Misc/CommonFonts.h
  src/Misc/JsonValidator.h
  src/Misc/ThemeManager.h
  src/Misc/TimerEvents.h
  src/Misc/Translator.h
  src/Misc/WorkspaceManager.h
  src/UI/Dashboard.h
  src/UI/DashboardWidget.h
  src/UI/Taskbar.h
  src/UI/WindowManager.h
  src/UI/WidgetRegistry.h
  src/UI/Widgets/GPS.h
  src/UI/Widgets/MultiPlot.h
  src/UI/Widgets/Gauge.h
  src/UI/Widgets/Plot.h
  src/UI/Widgets/DataGrid.h
  src/UI/Widgets/FFTPlot.h
  src/UI/Widgets/Gyroscope.h
  src/UI/Widgets/Bar.h
  src/UI/Widgets/Accelerometer.h
  src/UI/Widgets/LEDPanel.h
  src/UI/Widgets/Compass.h
  src/UI/Widgets/Terminal.h
  src/UI/DeclarativeWidgets/DeclarativeWidget.h
  src/UI/DeclarativeWidgets/StaticTable.h
  src/API/Server.h
  src/API/CommandProtocol.h
  src/API/MCPProtocol.h
  src/API/CommandRegistry.h
  src/API/CommandHandler.h
  src/API/MCPHandler.h
  src/API/PathPolicy.h
  src/API/Handlers/IOManagerHandler.h
  src/API/Handlers/UARTHandler.h
  src/API/Handlers/NetworkHandler.h
  src/API/Handlers/BluetoothLEHandler.h
  src/API/Handlers/CSVExportHandler.h
  src/API/Handlers/ProjectHandler.h
  src/API/Handlers/ConsoleHandler.h
  src/API/Handlers/CSVPlayerHandler.h
  src/API/Handlers/DashboardHandler.h
  src/Platform/NativeWindow.h
  src/Console/Handler.h
  src/Console/Export.h
  src/IO/Drivers/UART.h
  src/IO/Drivers/Network.h
  src/IO/Drivers/BluetoothLE.h
  src/IO/Manager.h
  src/IO/HAL_Driver.h
  src/IO/Checksum.h
  src/IO/CircularBuffer.h
  src/IO/FileTransmission.h
  src/IO/FrameReader.h
  src/DataModel/FrameParser.h
  src/DataModel/ProjectModel.h
  src/DataModel/Frame.h
  src/DataModel/FrameBuilder.h
  src/DataModel/FrameConsumer.h
  src/DataModel/FrameParserTestDialog.h
  src/CSV/Export.h
  src/CSV/Player.h
  src/MDF4/Player.h
  src/MDF4/Export.h
  src/ThirdParty/atomicops.h
  src/ThirdParty/readerwriterqueue.h
  src/ThirdParty/readerwritercircularbuffer.h
  src/AppInfo.h
  src/DSP.h
  src/Concepts.h
  src/SerialStudio.h
)

# Specify required QML files for FOSS version
set(QML_SOURCES
  qml/Dialogs/About.qml
  qml/Dialogs/Acknowledgements.qml
  qml/Dialogs/AccelerometerConfigDialog.qml
  qml/Dialogs/AxisRangeDialog.qml
  qml/Dialogs/CsvPlayer.qml
  qml/Dialogs/Mdf4Player.qml
  qml/Dialogs/Donate.qml
  qml/Dialogs/FileTransmission.qml
  qml/Dialogs/IconPicker.qml
  qml/Dialogs/Settings.qml
  qml/Dialogs/Welcome.qml
  qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml
  qml/MainWindow/Panes/Dashboard/DashboardLayout.qml
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
  qml/ProjectEditor/Views/DatasetView.qml
  qml/ProjectEditor/Views/FrameParserView.qml
  qml/ProjectEditor/Views/GroupView.qml
  qml/ProjectEditor/Views/ProjectView.qml
  qml/ProjectEditor/Views/TableDelegate.qml
  qml/ProjectEditor/ProjectEditor.qml
  qml/Widgets/Dashboard/Accelerometer.qml
  qml/Widgets/Dashboard/Bar.qml
  qml/Widgets/Dashboard/Compass.qml
  qml/Widgets/Dashboard/DataGrid.qml
  qml/Widgets/Dashboard/FFTPlot.qml
  qml/Widgets/Dashboard/Gauge.qml
  qml/Widgets/Dashboard/GPS.qml
  qml/Widgets/Dashboard/Gyroscope.qml
  qml/Widgets/Dashboard/LEDPanel.qml
  qml/Widgets/Dashboard/MultiPlot.qml
  qml/Widgets/Dashboard/Plot.qml
  qml/Widgets/Dashboard/Terminal.qml
  qml/Widgets/BigButton.qml
  qml/Widgets/InfoBullet.qml
  qml/Widgets/FileDropArea.qml
  qml/Widgets/MenuButton.qml
  qml/Widgets/MiniWindow.qml
  qml/Widgets/Pane.qml
  qml/Widgets/PlotWidget.qml
  qml/Widgets/ProNotice.qml
  qml/Widgets/SmartWindow.qml
  qml/Widgets/SmartDialog.qml
  qml/Widgets/SubMenuCombo.qml
  qml/Widgets/TaskbarButton.qml
  qml/Widgets/ToolbarButton.qml
  qml/Widgets/VisualRange.qml
  qml/DialogLoader.qml
  qml/main.qml
)

# Add commercial source files
if(BUILD_COMMERCIAL)
  set(HEADERS
    ${HEADERS}
    src/API/Handlers/ModbusHandler.h
    src/API/Handlers/CANBusHandler.h
    src/API/Handlers/MQTTHandler.h
    src/API/Handlers/MDF4ExportHandler.h
    src/API/Handlers/AudioHandler.h
    src/API/Handlers/MDF4PlayerHandler.h
    src/MQTT/Client.h
    src/IO/Drivers/Audio.h
    src/IO/Drivers/CANBus.h
    src/IO/Drivers/Modbus.h
    src/UI/Widgets/Plot3D.h
    src/DataModel/DBCImporter.h
    src/Licensing/Trial.h
    src/Licensing/MachineID.h
    src/Licensing/SimpleCrypt.h
    src/Licensing/LemonSqueezy.h
    src/ThirdParty/miniaudio.h
  )
  set(SOURCES
    ${SOURCES}
    src/API/Handlers/ModbusHandler.cpp
    src/API/Handlers/CANBusHandler.cpp
    src/API/Handlers/MQTTHandler.cpp
    src/API/Handlers/MDF4ExportHandler.cpp
    src/API/Handlers/AudioHandler.cpp
    src/API/Handlers/MDF4PlayerHandler.cpp
    src/MQTT/Client.cpp
    src/IO/Drivers/Audio.cpp
    src/IO/Drivers/CANBus.cpp
    src/IO/Drivers/Modbus.cpp
    src/UI/Widgets/Plot3D.cpp
    src/DataModel/DBCImporter.cpp
    src/Licensing/Trial.cpp
    src/ThirdParty/miniaudio.cpp
    src/Licensing/MachineID.cpp
    src/Licensing/SimpleCrypt.cpp
    src/Licensing/LemonSqueezy.cpp
  )
  set(QML_SOURCES
    ${QML_SOURCES}
    qml/Dialogs/LicenseManagement.qml
    qml/Dialogs/MQTTConfiguration.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml
    qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml
    qml/Widgets/Dashboard/Plot3D.qml
  )
endif()

#---------------------------------------------------------------------------------------------------
# Platform-specific source files and resources
#---------------------------------------------------------------------------------------------------
#
# Adds platform-specific code and resources for each target OS:
#
# Windows (WIN32):
#   - deploy/windows/info.rc: Application metadata (version, icon, manifest)
#   - src/Platform/CSD.cpp: Client-Side Decorations (custom window frame)
#   - src/Platform/NativeWindow_CSD.cpp: Windows-specific CSD implementation
#
# macOS (APPLE):
#   - deploy/macOS/AppIcon.icns: Application icon bundle
#   - deploy/macOS/Assets.car: macOS UI assets (compiled asset catalog)
#   - deploy/macOS/info-*.plist: Bundle metadata (GPL or Pro variant)
#   - src/Platform/NativeWindow_macOS.mm: Objective-C++ native window code
#     (SKIP_UNITY_BUILD_INCLUSION: Obj-C++ doesn't work with unity builds)
#
# Linux (UNIX AND NOT APPLE):
#   - src/Platform/CSD.cpp: Client-Side Decorations for modern Linux DEs
#   - src/Platform/NativeWindow_CSD.cpp: X11/Wayland CSD implementation
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
    "src/Platform/CSD.cpp"
    "src/Platform/NativeWindow_CSD.cpp"
  )
  set(HEADERS
    ${HEADERS}
    "src/Platform/CSD.h"
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
    "src/Platform/NativeWindow_macOS.mm"
  )

  set_source_files_properties("src/Platform/NativeWindow_macOS.mm" 
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
  )
elseif(UNIX)
  set(SOURCES
    ${SOURCES}
    "src/Platform/CSD.cpp"
    "src/Platform/NativeWindow_CSD.cpp"
  )
  set(HEADERS
    ${HEADERS}
    "src/Platform/CSD.h"
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
#   Third-party:
#     - kissfft: Fast Fourier Transform (FFT plot calculations)
#     - QCodeEditor: Syntax highlighting for DataModel/JavaScript editor
#     - QSimpleUpdater: Auto-update framework (checks for new versions)
#     - mdf: MDF4/MF4 file format parser (Pro: replay CAN/LIN data)
#     - OpenSSL: Cryptography, SSL/TLS (for MQTT, secure connections)
#
# target_link_openssl():
#   - Custom CMake function to link OpenSSL static libraries
#   - Platform-specific: libssl.a, libcrypto.a (or .lib on Windows)
#   - Ensures secure communications and license validation work
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

target_link_libraries(
  ${PROJECT_EXECUTABLE} PUBLIC

  ${QT_LIBS}

  kissfft
  QCodeEditor
  QSimpleUpdater
  mdf
)

target_link_openssl(
  ${PROJECT_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/OpenSSL
)

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
    -Wl,-z,relro                 # Relocation Read-Only
    -Wl,-z,now                   # Resolve all symbols at load time
    -Wl,-z,noexecstack           # Non-executable stack
    -pie                         # Position Independent Executable
  )
endif()

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

qt_add_qml_module(
  ${PROJECT_EXECUTABLE}
  URI gui
  VERSION 1.0
  QML_FILES ${QML_SOURCES}
  RESOURCE_PREFIX /serial-studio.com/
)

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
  target_compile_definitions(${PROJECT_EXECUTABLE} PRIVATE
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

  if(LINUX)
    target_link_libraries(${PROJECT_EXECUTABLE} PRIVATE asound)
  endif()

  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64)$")
    target_compile_definitions(${PROJECT_EXECUTABLE} PRIVATE MA_SUPPORT_SSE2)
  endif()

  if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$" AND NOT APPLE)
    target_compile_definitions(${PROJECT_EXECUTABLE} PRIVATE MA_SUPPORT_NEON)
  endif()
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
  target_link_libraries(${PROJECT_EXECUTABLE} PRIVATE Dwmapi.lib)
  set_target_properties(${PROJECT_EXECUTABLE} PROPERTIES
    WIN32_EXECUTABLE TRUE
  )
elseif(APPLE)
  set_target_properties(
    ${PROJECT_EXECUTABLE} PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_ICON_FILE ${ICON_MACOSX}
    MACOSX_BUNDLE_INFO_PLIST ${INFO_MACOSX}
  )
elseif(UNIX)
  install(TARGETS ${PROJECT_EXECUTABLE} RUNTIME DESTINATION bin)
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
#     --no-compiler-runtime - Don't include MSVC runtime (already bundled)
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

install(
  TARGETS ${PROJECT_EXECUTABLE}
  BUNDLE  DESTINATION .
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

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
#   CPACK_PACKAGE_VERSION       - Semantic version (e.g., 3.2.3)
#   CPACK_PACKAGE_DESCRIPTION   - README.md content
#   CPACK_RESOURCE_FILE_LICENSE - GPL-3.0-only.txt or Commercial license
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
#   # Creates: Serial-Studio-GPL3-v3.2.3.msi (Windows)
#   #          Serial-Studio-GPL3-v3.2.3.dmg (macOS)
#   #          Serial-Studio-GPL3-v3.2.3.tar.gz (Linux)
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
