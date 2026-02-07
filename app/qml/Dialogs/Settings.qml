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

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Window options
  //
  title: qsTr("Preferences")

  //
  // Settings persistence
  //
  Settings {
    category: "Preferences"
    property alias plugins: _apiServer.checked
    property alias dashboardPoints: _points.value
    property alias dashboardActionPanel: _actionsPanel.checked
    property alias alwaysShowTaskbarBt: _taskbarButtons.checked
    property alias dashboardAutoHideToolbar: _autoHideToolbar.checked
  }

  //
  // Window controls
  //
  contentItem: ColumnLayout {
    id: layout
    spacing: 4

      //
      // General settings
      //
      Label {
        text: qsTr("General")
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      } GroupBox {
        Layout.fillWidth: true
        Layout.minimumWidth: 356

        background: Rectangle {
          radius: 2
          border.width: 1
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        GridLayout {
          columns: 2
          rowSpacing: 4
          columnSpacing: 8
          anchors.fill: parent

          //
          // Language selector
          //
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
            onCurrentIndexChanged: {
              if (currentIndex !== Cpp_Misc_Translator.language)
                Cpp_Misc_Translator.setLanguage(currentIndex)
            }
          }

          //
          // Theme selector
          //
          Label {
            text: qsTr("Theme")
            opacity: enabled ? 1 : 0.5
            color: Cpp_ThemeManager.colors["text"]
          } ComboBox {
            id: _themeCombo
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            currentIndex: Cpp_ThemeManager.theme
            model: Cpp_ThemeManager.availableThemes
            onCurrentIndexChanged: {
              if (currentIndex !== Cpp_ThemeManager.theme)
                Cpp_ThemeManager.setTheme(currentIndex)
            }
          }

          //
          // Workspace
          //
          Label {
            text: qsTr("Workspace Folder")
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_IO_Manager.isConnected
            color: Cpp_ThemeManager.colors["text"]
          } RowLayout {
            spacing: 2
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_IO_Manager.isConnected

            TextField {
              readOnly: true
              Layout.fillWidth: true
              Layout.minimumWidth: 256
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
              icon.source: "qrc:/rcc/icons/buttons/open.svg"
              onClicked: Cpp_Misc_WorkspaceManager.selectPath()
              icon.color: Cpp_ThemeManager.colors["button_text"]
            }
          }
        }
      }

      //
      // Spacer
      //
      Item {
        implicitHeight: 4
      }

      //
      // Miscellaneous settings
      //
      Label {
        text: qsTr("Miscellaneous")
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      } GroupBox {
        Layout.fillWidth: true

        background: Rectangle {
          radius: 2
          border.width: 1
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        GridLayout {
          columns: 2
          rowSpacing: 4
          columnSpacing: 8
          anchors.fill: parent

          //
          // API Server enabled
          //
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

          //
          // Auto-updater
          //
          Label {
            color: Cpp_ThemeManager.colors["text"]
            text: qsTr("Automatically Check for Updates")
          } Switch {
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: mainWindow.automaticUpdates
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== mainWindow.automaticUpdates)
                mainWindow.automaticUpdates = checked
            }
          }
        }
      }

      //
      // Spacer
      //
      Item {
        implicitHeight: 4
      }

      //
      // Dashboard settings
      //
      Label {
        text: qsTr("Dashboard")
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      } GroupBox {
        Layout.fillWidth: true

        background: Rectangle {
          radius: 2
          border.width: 1
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        GridLayout {
          columns: 2
          rowSpacing: 4
          columnSpacing: 8
          anchors.fill: parent

          //
          // Points
          //
          Label {
            text: qsTr("Point Count")
            color: Cpp_ThemeManager.colors["text"]
          } SpinBox {
            id: _points

            from: 2
            to: 1e6
            editable: true
            Layout.fillWidth: true
            value: Cpp_UI_Dashboard.points
            onValueChanged: {
              if (value !== Cpp_UI_Dashboard.points)
                Cpp_UI_Dashboard.points = value
            }
          }

          //
          // Refresh rate
          //
          Label {
            text: qsTr("UI Refresh Rate (Hz)")
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

          //
          // Console
          //
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

          //
          // Auto-hide toolbar
          //
          Label {
            text: qsTr("Auto-Hide Toolbar on Dashboard")
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

          //
          // Taskbar buttons
          //
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

          //
          // Frame extraction
          //
          Label {
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_IO_Manager.isConnected
            color: Cpp_ThemeManager.colors["text"]
            text: qsTr("Use Separate Thread for Frame Extraction")
          } Switch {
            Layout.rightMargin: -8
            opacity: enabled ? 1 : 0.5
            Layout.alignment: Qt.AlignRight
            enabled: !Cpp_IO_Manager.isConnected
            checked: Cpp_IO_Manager.threadedFrameExtraction
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_IO_Manager.threadedFrameExtraction)
                Cpp_IO_Manager.threadedFrameExtraction = checked
            }
          }
        }
      }

      //
      // Spacer
      //
      Item {
        implicitHeight: 4
      }

      //
      // Console settings
      //
      Label {
        text: qsTr("Console")
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      }

      GroupBox {
        Layout.fillWidth: true

        background: Rectangle {
          radius: 2
          border.width: 1
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        GridLayout {
          columns: 2
          rowSpacing: 4
          columnSpacing: 8
          anchors.fill: parent

          Label {
            text: qsTr("Font Family")
            color: Cpp_ThemeManager.colors["text"]
          }

          ComboBox {
            id: _consoleFontFamily
            Layout.fillWidth: true
            model: Cpp_Console_Handler.availableFonts

            Component.onCompleted: {
              currentIndex = find(Cpp_Console_Handler.fontFamily)
            }

            onCurrentTextChanged: {
              if (currentText !== Cpp_Console_Handler.fontFamily)
                Cpp_Console_Handler.fontFamily = currentText
            }
          }

          Label {
            text: qsTr("Font Size")
            color: Cpp_ThemeManager.colors["text"]
          }

          SpinBox {
            id: _consoleFontSize
            from: 6
            to: 72
            editable: true
            Layout.fillWidth: true
            value: Cpp_Console_Handler.fontSize

            onValueChanged: {
              if (value !== Cpp_Console_Handler.fontSize)
                Cpp_Console_Handler.fontSize = value
            }
          }
        }
      }

      //
      // Spacer
      //
      Item {
        implicitHeight: 4
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
          text: qsTr("Reset")
          horizontalPadding: 8
          opacity: enabled ? 1 : 0.5
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/rcc/icons/buttons/refresh.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
          onClicked: {
            Cpp_ThemeManager.theme = 0
            Cpp_UI_Dashboard.points = 100
            Cpp_Misc_TimerEvents.fps = 60
            Cpp_UI_Dashboard.precision = 2
            Cpp_API_Server.enabled = false
            mainWindow.automaticUpdates  = true
            Cpp_UI_Dashboard.terminalEnabled = false
            Cpp_IO_Manager.threadedFrameExtraction = false
            Cpp_UI_Dashboard.autoHideToolbar = false
            Cpp_UI_Dashboard.showTaskbarButtons = false
            Cpp_Console_Handler.setFontFamily(Cpp_Misc_CommonFonts.monoFont.family)
            Cpp_Console_Handler.setFontSize(Cpp_Misc_CommonFonts.monoFont.pointSize)
          }
        }

        Item {
          implicitWidth: 96
          Layout.fillWidth: true
        }

        Button {
          icon.width: 18
          icon.height: 18
          text: qsTr("Close")
          horizontalPadding: 8
          onClicked: root.hide()
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/rcc/icons/buttons/close.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
        }

        Button {
          icon.width: 18
          icon.height: 18
          text: qsTr("Apply")
          horizontalPadding: 8
          onClicked: root.hide()
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/rcc/icons/buttons/apply.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
        }
      }
    }
}
