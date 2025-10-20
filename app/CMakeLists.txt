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

#-------------------------------------------------------------------------------
# Project setup
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20)
project(app LANGUAGES CXX VERSION ${PROJECT_VERSION})

#-------------------------------------------------------------------------------
# C++ options
#-------------------------------------------------------------------------------

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#-------------------------------------------------------------------------------
# Add external dependencies (Qt)
#-------------------------------------------------------------------------------

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

#-------------------------------------------------------------------------------
# Import source code
#-------------------------------------------------------------------------------

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
  src/Plugins/Server.cpp
  src/IO/Drivers/Network.cpp
  src/IO/Drivers/UART.cpp
  src/IO/Drivers/BluetoothLE.cpp
  src/IO/Checksum.cpp
  src/IO/Console.cpp
  src/IO/Manager.cpp
  src/IO/ConsoleExport.cpp
  src/IO/FileTransmission.cpp
  src/IO/FrameReader.cpp
  src/JSON/FrameParser.cpp
  src/JSON/ProjectModel.cpp
  src/JSON/FrameBuilder.cpp
  src/JSON/Frame.cpp
  src/CSV/Player.cpp
  src/CSV/Export.cpp
  src/main.cpp
  src/SerialStudio.cpp
)

# Specify required headers for FOSS version
set(HEADERS
  src/Misc/ModuleManager.h
  src/Misc/Utilities.h
  src/Misc/CommonFonts.h
  src/Misc/ThemeManager.h
  src/Misc/TimerEvents.h
  src/Misc/Translator.h
  src/Misc/WorkspaceManager.h
  src/UI/Dashboard.h
  src/UI/DashboardWidget.h
  src/UI/Taskbar.h
  src/UI/WindowManager.h
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
  src/Plugins/Server.h
  src/Platform/NativeWindow.h
  src/IO/Console.h
  src/IO/Drivers/UART.h
  src/IO/Drivers/Network.h
  src/IO/Drivers/BluetoothLE.h
  src/IO/Manager.h
  src/IO/HAL_Driver.h
  src/IO/Checksum.h
  src/IO/ConsoleExport.h
  src/IO/CircularBuffer.h
  src/IO/FileTransmission.h
  src/IO/FrameReader.h
  src/JSON/FrameParser.h
  src/JSON/ProjectModel.h
  src/JSON/Frame.h
  src/JSON/FrameBuilder.h
  src/CSV/Export.h
  src/CSV/Player.h
  src/ThirdParty/atomicops.h
  src/ThirdParty/readerwriterqueue.h
  src/ThirdParty/readerwritercircularbuffer.h
  src/AppInfo.h
  src/DSP.h
  src/SerialStudio.h
)

# Add headers & sources for commercial build
if(BUILD_COMMERCIAL)
  set(HEADERS
    ${HEADERS}
    src/MQTT/Client.h
    src/IO/Drivers/Audio.h
    src/IO/Drivers/CANBus.h
    src/IO/Drivers/Modbus.h
    src/UI/Widgets/Plot3D.h
    src/Licensing/Trial.h
    src/Licensing/MachineID.h
    src/Licensing/SimpleCrypt.h
    src/Licensing/LemonSqueezy.h
    src/ThirdParty/miniaudio.h
  )
  set(SOURCES
    ${SOURCES}
    src/MQTT/Client.cpp
    src/IO/Drivers/Audio.cpp
    src/IO/Drivers/CANBus.cpp
    src/IO/Drivers/Modbus.cpp
    src/UI/Widgets/Plot3D.cpp
    src/Licensing/Trial.cpp
    src/ThirdParty/miniaudio.cpp
    src/Licensing/MachineID.cpp
    src/Licensing/SimpleCrypt.cpp
    src/Licensing/LemonSqueezy.cpp
  )
endif()

#-------------------------------------------------------------------------------
# Platform-specific configuration
#-------------------------------------------------------------------------------

if(WIN32)
  set(WIN_RC "${CMAKE_CURRENT_SOURCE_DIR}/deploy/windows/info.rc")
  set(SOURCES ${SOURCES} ${WIN_RC} "src/Platform/NativeWindow_Windows.cpp")
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
  set(SOURCES ${SOURCES} ${ICON_MACOSX} ${ASSETS_MACOSX} "src/Platform/NativeWindow_macOS.mm")
  set_source_files_properties(
    "src/Platform/NativeWindow_macOS.mm" PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
  )
elseif(UNIX)
  set(SOURCES ${SOURCES} "src/Platform/NativeWindow_UNIX.cpp")
endif()

#-------------------------------------------------------------------------------
# Add resources
#-------------------------------------------------------------------------------

qt_add_resources(RES_RCC ${CMAKE_CURRENT_SOURCE_DIR}/rcc/rcc.qrc)
qt_add_resources(QTR_RCC ${CMAKE_CURRENT_SOURCE_DIR}/translations/translations.qrc)

#-------------------------------------------------------------------------------
# Create executable
#-------------------------------------------------------------------------------

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
)

target_link_openssl(
  ${PROJECT_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/../lib/OpenSSL
)

#-------------------------------------------------------------------------------
# Import QML code
#-------------------------------------------------------------------------------

set(QML_SOURCES
  qml/Dialogs/About.qml
  qml/Dialogs/Acknowledgements.qml
  qml/Dialogs/CsvPlayer.qml
  qml/Dialogs/Donate.qml
  qml/Dialogs/FileTransmission.qml
  qml/Dialogs/IconPicker.qml
  qml/Dialogs/LicenseManagement.qml
  qml/Dialogs/MQTTConfiguration.qml
  qml/Dialogs/Settings.qml
  qml/Dialogs/Welcome.qml
  qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml
  qml/MainWindow/Panes/Dashboard/StartMenu.qml
  qml/MainWindow/Panes/Dashboard/Taskbar.qml
  qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml
  qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml
  qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml
  qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml
  qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml
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
  qml/Widgets/Dashboard/Plot3D.qml
  qml/Widgets/Dashboard/Terminal.qml
  qml/Widgets/BigButton.qml
  qml/Widgets/InfoBullet.qml
  qml/Widgets/JSONDropArea.qml
  qml/Widgets/MenuButton.qml
  qml/Widgets/MiniWindow.qml
  qml/Widgets/Pane.qml
  qml/Widgets/PlotWidget.qml
  qml/Widgets/ProNotice.qml
  qml/Widgets/SmartWindow.qml
  qml/Widgets/SubMenuCombo.qml
  qml/Widgets/TaskbarButton.qml
  qml/Widgets/ToolbarButton.qml
  qml/Widgets/VisualRange.qml
  qml/DialogLoader.qml
  qml/main.qml
)

qt_add_qml_module(
  ${PROJECT_EXECUTABLE}
  URI gui
  VERSION 1.0
  QML_FILES ${QML_SOURCES}
  RESOURCE_PREFIX /serial-studio.com/
)

#-------------------------------------------------------------------------------
# MiniAudio configuration
#-------------------------------------------------------------------------------

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

#-------------------------------------------------------------------------------
# Deployment options
#-------------------------------------------------------------------------------

if(WIN32)
  if(MINGW)
    target_link_libraries(${PROJECT_EXECUTABLE} PRIVATE dwmapi)
  else()
    target_link_libraries(${PROJECT_EXECUTABLE} PRIVATE Dwmapi.lib)
  endif()

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

#-------------------------------------------------------------------------------
# Qt Deployment
#-------------------------------------------------------------------------------

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

#-------------------------------------------------------------------------------
# Packaging
#-------------------------------------------------------------------------------

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

set(CPACK_MONOLITHIC_INSTALL ON)

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
