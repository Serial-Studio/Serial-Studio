/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Proto file export dialog
  //
  FileDialog {
    id: _protoFileDialog

    fileMode: FileDialog.SaveFile
    nameFilters: ["Protocol Buffers (*.proto)"]
    onAccepted: {
      if (Cpp_GrpcAvailable && Cpp_GRPC_Server)
        Cpp_GRPC_Server.exportProto(selectedFile.toString().replace("file://", ""))
    }
  }

  //
  // Window options
  //
  title: qsTr("Preferences")

  //
  // Window controls
  //
  dialogContent: ColumnLayout {
    id: layout

    spacing: 12
    anchors.centerIn: parent

    //
    // Tab bar
    //
    TabBar {
      id: _tab

      implicitHeight: 24
      Layout.fillWidth: true

      TabButton {
        text: qsTr("General")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Dashboard")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Console")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }
    }

    //
    // Tab contents
    //
    StackLayout {
      id: stack

      clip: true
      Layout.fillWidth: true
      Layout.minimumWidth: 480
      currentIndex: _tab.currentIndex
      Layout.topMargin: -parent.spacing - 1
      implicitHeight: Math.max(
                        generalTab.implicitHeight,
                        dashboardTab.implicitHeight,
                        consoleTab.implicitHeight
                        )

      //
      // General tab
      //
      Item {
        id: generalTab

        Layout.fillWidth: true
        Layout.fillHeight: true
        implicitHeight: generalLayout.implicitHeight + 16

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        GridLayout {
          id: generalLayout

          columns: 2
          rowSpacing: 4
          columnSpacing: 8
          anchors.fill: parent
          anchors.margins: 8

          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 2
            text: qsTr("Appearance")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("Language")
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_IO_Manager.isConnected
            color: Cpp_ThemeManager.colors["text"]
          } ComboBox {
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_IO_Manager.isConnected
            currentIndex: Cpp_Misc_Translator.language
            model: Cpp_Misc_Translator.availableLanguages
            onActivated: {
              Cpp_Misc_Translator.language = currentIndex
            }
          }

          Label {
            text: qsTr("Theme")
            color: Cpp_ThemeManager.colors["text"]
          } ComboBox {
            id: _themeCombo

            Layout.fillWidth: true
            currentIndex: Cpp_ThemeManager.theme
            model: Cpp_ThemeManager.availableThemes
            onActivated: {
              Cpp_ThemeManager.theme = currentIndex
            }
          }

          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 6
            text: qsTr("Files & Updates")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            opacity: enabled ? 1 : 0.5
            text: qsTr("Workspace Folder")
            enabled: !Cpp_IO_Manager.isConnected
            color: Cpp_ThemeManager.colors["text"]
          } RowLayout {
            spacing: 2
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_IO_Manager.isConnected

            TextField {
              readOnly: true
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignVCenter
              text: Cpp_Misc_WorkspaceManager.shortPath
            }

            Button {
              icon.width: 18
              icon.height: 18
              Layout.fillWidth: false
              Layout.maximumWidth: 24
              Layout.maximumHeight: 24
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_Misc_WorkspaceManager.selectPath()
              icon.color: Cpp_ThemeManager.colors["button_text"]
              icon.source: "qrc:/rcc/icons/buttons/open.svg"
            }
          }

          Label {
            color: Cpp_ThemeManager.colors["text"]
            text: qsTr("Automatically Check for Updates")
          } Switch {
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_Misc_ModuleManager.automaticUpdates
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_Misc_ModuleManager.automaticUpdates)
                Cpp_Misc_ModuleManager.automaticUpdates = checked
            }
          }

          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 6
            text: qsTr("Advanced")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("Auto-Hide Toolbar")
            color: Cpp_ThemeManager.colors["text"]
          } Switch {
            id: _autoHideToolbar

            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_UI_Dashboard.autoHideToolbar
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_UI_Dashboard.autoHideToolbar)
                Cpp_UI_Dashboard.autoHideToolbar = checked
            }
          }

          Label {
            color: Cpp_ThemeManager.colors["text"]
            text: qsTr("Enable API Server (Port 7777)")
          } Switch {
            id: _apiServer

            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_API_Server.enabled
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_API_Server.enabled)
                Cpp_API_Server.enabled = checked
            }
          }

          Label {
            enabled: _apiServer.checked
            opacity: enabled ? 1 : 0.5
            color: Cpp_ThemeManager.colors["text"]
            text: qsTr("Allow External API Connections")
          } Switch {
            Layout.rightMargin: -8
            opacity: enabled ? 1 : 0.5
            enabled: _apiServer.checked
            Layout.alignment: Qt.AlignRight
            checked: Cpp_API_Server.externalConnections
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_API_Server.externalConnections)
                Cpp_API_Server.externalConnections = checked
            }
          }

          Label {
            visible: Cpp_GrpcAvailable
            opacity: enabled ? 1 : 0.5
            enabled: _apiServer.checked
            color: Cpp_ThemeManager.colors["text"]
            text: qsTr("Export Protobuf File")
          } Button {
            visible: Cpp_GrpcAvailable
            opacity: enabled ? 1 : 0.5
            enabled: _apiServer.checked
            Layout.alignment: Qt.AlignRight
            text: qsTr("Export...")
            onClicked: _protoFileDialog.open()
          }

          Item { Layout.fillHeight: true }
          Item { Layout.fillHeight: true }
        }
      }

      //
      // Dashboard tab
      //
      Item {
        id: dashboardTab

        Layout.fillWidth: true
        Layout.fillHeight: true
        implicitHeight: dashboardLayout.implicitHeight + 16

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
          color: Cpp_ThemeManager.colors["groupbox_background"]
        }

        GridLayout {
          id: dashboardLayout

          columns: 2
          rowSpacing: 4
          columnSpacing: 8
          anchors.fill: parent
          anchors.margins: 8

          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.topMargin: 2
            Layout.columnSpan: 2
            text: qsTr("Data Plotting")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("Point Count")
            color: Cpp_ThemeManager.colors["text"]
          } SpinBox {
            id: _points

            from: 1
            to: 1e6
            editable: true
            Layout.fillWidth: true
            value: Cpp_UI_Dashboard.points
            onValueChanged: {
              if (value !== Cpp_UI_Dashboard.points)
                Cpp_UI_Dashboard.points = value
            }

            Connections {
              target: Cpp_UI_Dashboard
              function onPointsChanged() {
                if (_points.value !== Cpp_UI_Dashboard.points)
                  _points.value = Cpp_UI_Dashboard.points
              }
            }
          }

          Label {
            text: qsTr("UI Refresh Rate (Hz)")
            color: Cpp_ThemeManager.colors["text"]
          } SpinBox {
            id: _refreshRate

            from: 1
            to: 240
            editable: true
            Layout.fillWidth: true
            value: Cpp_Misc_TimerEvents.fps
            onValueChanged: {
              if (value !== Cpp_Misc_TimerEvents.fps)
                Cpp_Misc_TimerEvents.fps = value
            }
          }

          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 6
            text: qsTr("Dashboard Font")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("Font Family")
            color: Cpp_ThemeManager.colors["text"]
          } ComboBox {
            id: _widgetFontFamily

            Layout.fillWidth: true
            model: Cpp_Misc_CommonFonts.availableFonts
            currentIndex: Cpp_Misc_CommonFonts.widgetFontIndex

            onActivated: {
              Cpp_Misc_CommonFonts.widgetFontFamily = currentText
            }
          }

          Label {
            text: qsTr("Font Size")
            color: Cpp_ThemeManager.colors["text"]
          } RowLayout {
            spacing: 4
            Layout.fillWidth: true

            ComboBox {
              id: _widgetSizePreset

              Layout.fillWidth: true
              model: [qsTr("Small"), qsTr("Normal"), qsTr("Large"), qsTr("Extra Large"), qsTr("Custom")]

              Component.onCompleted: {
                const scale = Cpp_Misc_CommonFonts.widgetFontScale
                if (Math.abs(scale - 0.85) < 0.01)
                  currentIndex = 0
                else if (Math.abs(scale - 1.00) < 0.01)
                  currentIndex = 1
                else if (Math.abs(scale - 1.25) < 0.01)
                  currentIndex = 2
                else if (Math.abs(scale - 1.50) < 0.01)
                  currentIndex = 3
                else
                  currentIndex = 4
              }

              onActivated: (index) => {
                             const scales = [0.85, 1.00, 1.25, 1.50]
                             if (index < 4)
                               Cpp_Misc_CommonFonts.widgetFontScale = scales[index]
                           }
            }

            SpinBox {
              id: _widgetFontCustom

              from: 50
              to: 300
              editable: true
              visible: _widgetSizePreset.currentIndex === 4
              value: Math.round(Cpp_Misc_CommonFonts.widgetFontScale * 100)

              textFromValue: (val) => val + "%"
              valueFromText: (text) => parseInt(text)

              onValueModified: {
                Cpp_Misc_CommonFonts.widgetFontScale = value / 100.0
              }
            }
          }

          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 6
            text: qsTr("Layout")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("Show Actions Panel")
            color: Cpp_ThemeManager.colors["text"]
          } Switch {
            id: _actionsPanel

            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_UI_Dashboard.showActionPanel
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_UI_Dashboard.showActionPanel)
                Cpp_UI_Dashboard.showActionPanel = checked
            }
          }

          Label {
            text: qsTr("Always Show Taskbar Buttons")
            color: Cpp_ThemeManager.colors["text"]
          } Switch {
            id: _taskbarButtons

            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_UI_Dashboard.showTaskbarButtons
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_UI_Dashboard.showTaskbarButtons)
                Cpp_UI_Dashboard.showTaskbarButtons = checked
            }
          }

          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
            visible: Cpp_CommercialBuild
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 6
            visible: Cpp_CommercialBuild
            text: qsTr("Image Export")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            visible: Cpp_CommercialBuild
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
            visible: Cpp_CommercialBuild
          }

          Label {
            visible: Cpp_CommercialBuild
            text: qsTr("Save Images by Default")
            color: Cpp_ThemeManager.colors["text"]
          } Switch {
            id: _saveImages

            Layout.rightMargin: -8
            visible: Cpp_CommercialBuild
            Layout.alignment: Qt.AlignRight
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            checked: Cpp_CommercialBuild && Cpp_Image_Export.exportEnabled
            onCheckedChanged: {
              if (Cpp_CommercialBuild && checked !== Cpp_Image_Export.exportEnabled)
                Cpp_Image_Export.exportEnabled = checked
            }
          }

          Item { Layout.fillHeight: true }
          Item { Layout.fillHeight: true }
        }
      }

      //
      // Console tab
      //
      Item {
        id: consoleTab

        Layout.fillWidth: true
        Layout.fillHeight: true
        implicitHeight: consoleLayout.implicitHeight + 16

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
          color: Cpp_ThemeManager.colors["groupbox_background"]
        }

        GridLayout {
          id: consoleLayout

          columns: 2
          rowSpacing: 4
          columnSpacing: 8
          anchors.fill: parent
          anchors.margins: 8

          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 2
            text: qsTr("Display")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("Display Mode")
            color: Cpp_ThemeManager.colors["text"]
          } ComboBox {
            Layout.fillWidth: true
            model: Cpp_Console_Handler.displayModes
            currentIndex: Cpp_Console_Handler.displayMode
            onCurrentIndexChanged: {
              if (currentIndex !== Cpp_Console_Handler.displayMode)
                Cpp_Console_Handler.displayMode = currentIndex
            }
          }

          Label {
            text: qsTr("Font Family")
            color: Cpp_ThemeManager.colors["text"]
          } ComboBox {
            id: _consoleFontFamily

            Layout.fillWidth: true
            model: Cpp_Console_Handler.availableFonts
            currentIndex: Cpp_Console_Handler.fontFamilyIndex

            onActivated: {
              Cpp_Console_Handler.fontFamily = currentText
            }
          }

          Label {
            text: qsTr("Font Size")
            color: Cpp_ThemeManager.colors["text"]
          } SpinBox {
            id: _consoleFontSize

            from: 6
            to: 72
            editable: true
            Layout.fillWidth: true
            value: Cpp_Console_Handler.fontSize

            onValueModified: {
              Cpp_Console_Handler.fontSize = value
            }
          }

          Label {
            text: qsTr("Show Timestamps")
            color: Cpp_ThemeManager.colors["text"]
          } Switch {
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_Console_Handler.showTimestamp
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_Console_Handler.showTimestamp)
                Cpp_Console_Handler.showTimestamp = checked
            }
          }

          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 6
            text: qsTr("Data Transmission")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("Line Ending")
            color: Cpp_ThemeManager.colors["text"]
          } ComboBox {
            Layout.fillWidth: true
            model: Cpp_Console_Handler.lineEndings
            currentIndex: Cpp_Console_Handler.lineEnding
            onCurrentIndexChanged: {
              if (currentIndex !== Cpp_Console_Handler.lineEnding)
                Cpp_Console_Handler.lineEnding = currentIndex
            }
          }

          Label {
            text: qsTr("Input Mode")
            color: Cpp_ThemeManager.colors["text"]
          } ComboBox {
            Layout.fillWidth: true
            model: Cpp_Console_Handler.dataModes
            currentIndex: Cpp_Console_Handler.dataMode
            onCurrentIndexChanged: {
              if (currentIndex !== Cpp_Console_Handler.dataMode)
                Cpp_Console_Handler.dataMode = currentIndex
            }
          }

          Label {
            text: qsTr("Checksum")
            color: Cpp_ThemeManager.colors["text"]
          } ComboBox {
            Layout.fillWidth: true
            model: Cpp_Console_Handler.checksumMethods
            currentIndex: Cpp_Console_Handler.checksumMethod
            onCurrentIndexChanged: {
              if (currentIndex !== Cpp_Console_Handler.checksumMethod)
                Cpp_Console_Handler.checksumMethod = currentIndex
            }
          }

          Label {
            text: qsTr("Echo Sent Data")
            color: Cpp_ThemeManager.colors["text"]
          } Switch {
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_Console_Handler.echo
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_Console_Handler.echo)
                Cpp_Console_Handler.echo = checked
            }
          }

          Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          } Label {
            Layout.columnSpan: 2
            Layout.topMargin: 2
            text: qsTr("Escape Codes")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          } Item {
            implicitHeight: 2
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("VT100 Emulation")
            color: Cpp_ThemeManager.colors["text"]
            opacity: Cpp_Console_Handler.imageWidgetActive ? 0.8 : 1
          } Switch {
            id: _vt100Emulation

            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_Console_Handler.vt100Emulation
            enabled: !Cpp_Console_Handler.imageWidgetActive
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            opacity: Cpp_Console_Handler.imageWidgetActive ? 0.8 : 1
            onCheckedChanged: {
              if (checked !== Cpp_Console_Handler.vt100Emulation)
                Cpp_Console_Handler.vt100Emulation = checked
            }
          }

          Label {
            text: qsTr("ANSI Colors")
            enabled: _vt100Emulation.checked
            color: Cpp_ThemeManager.colors["text"]
            opacity: Cpp_Console_Handler.imageWidgetActive ? 0.8 : (enabled ? 1 : 0.5)
          } Switch {
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_Console_Handler.ansiColors
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            enabled: _vt100Emulation.checked && !Cpp_Console_Handler.imageWidgetActive
            opacity: Cpp_Console_Handler.imageWidgetActive ? 0.8 : (enabled ? 1 : 0.5)
            onCheckedChanged: {
              if (checked !== Cpp_Console_Handler.ansiColors)
                Cpp_Console_Handler.ansiColors = checked
            }
          }

          Item { Layout.fillHeight: true }
          Item { Layout.fillHeight: true }
        }
      }
    }

    //
    // Buttons
    //
    RowLayout {
      spacing: 4
      Layout.fillWidth: true

      Button {
        icon.width: 18
        icon.height: 18
        horizontalPadding: 8
        text: qsTr("Reset")
        opacity: enabled ? 1 : 0.5
        Layout.alignment: Qt.AlignVCenter
        icon.color: Cpp_ThemeManager.colors["button_text"]
        icon.source: "qrc:/rcc/icons/buttons/refresh.svg"
        onClicked: {
          Cpp_ThemeManager.theme = 0
          Cpp_UI_Dashboard.points = 100
          Cpp_Misc_TimerEvents.fps = 60
          Cpp_API_Server.enabled = false
          Cpp_API_Server.externalConnections = false
          Cpp_Misc_ModuleManager.automaticUpdates = true
          Cpp_UI_Dashboard.autoHideToolbar = false
          Cpp_UI_Dashboard.showTaskbarButtons = false
          Cpp_Console_Handler.fontFamily = Cpp_Misc_CommonFonts.monoFont.family
          Cpp_Console_Handler.fontSize = Cpp_Misc_CommonFonts.monoFont.pointSize
          Cpp_Console_Handler.echo = false
          Cpp_Console_Handler.showTimestamp = false
          Cpp_Console_Handler.vt100Emulation = true
          Cpp_Console_Handler.ansiColors = true
          Cpp_Console_Handler.dataMode = 0
          Cpp_Console_Handler.displayMode = 0
          Cpp_Console_Handler.lineEnding = 1
          Cpp_Console_Handler.checksumMethod = 0
          Cpp_Misc_CommonFonts.widgetFontScale = 1.0
          Cpp_Misc_CommonFonts.widgetFontFamily = Cpp_Misc_CommonFonts.monoFont.family
        }
      }

      Item {
        Layout.fillWidth: true
      }

      Button {
        icon.width: 18
        icon.height: 18
        horizontalPadding: 8
        text: qsTr("Close")
        onClicked: root.hide()
        Layout.alignment: Qt.AlignVCenter
        icon.color: Cpp_ThemeManager.colors["button_text"]
        icon.source: "qrc:/rcc/icons/buttons/close.svg"
      }

      Button {
        icon.width: 18
        icon.height: 18
        horizontalPadding: 8
        text: qsTr("Apply")
        onClicked: root.hide()
        Layout.alignment: Qt.AlignVCenter
        icon.color: Cpp_ThemeManager.colors["button_text"]
        icon.source: "qrc:/rcc/icons/buttons/apply.svg"
      }
    }
  }
}
