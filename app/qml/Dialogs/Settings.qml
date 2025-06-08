/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Window {
  id: root

  //
  // Window options
  //
  title: qsTr("Preferences")
  modality: Qt.WindowModal
  minimumWidth: layout.implicitWidth + 32
  maximumWidth: layout.implicitWidth + 32
  minimumHeight: layout.implicitHeight + 32
  maximumHeight: layout.implicitHeight + 32
  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.WindowTitleHint |
        Qt.WindowCloseButtonHint
  }

  Settings {
    category: "Preferences"
    property alias plugins: _tcpPlugins.checked
    property alias dashboardPoints: _points.value
    property alias dashboardPrecision: _decimalDigits.value
  }

  //
  // Close shortcut
  //
  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  //
  // Use page item to set application palette
  //
  Page {
    anchors.fill: parent
    palette.mid: Cpp_ThemeManager.colors["mid"]
    palette.dark: Cpp_ThemeManager.colors["dark"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.link: Cpp_ThemeManager.colors["link"]
    palette.light: Cpp_ThemeManager.colors["light"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.shadow: Cpp_ThemeManager.colors["shadow"]
    palette.accent: Cpp_ThemeManager.colors["accent"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.midlight: Cpp_ThemeManager.colors["midlight"]
    palette.highlight: Cpp_ThemeManager.colors["highlight"]
    palette.windowText: Cpp_ThemeManager.colors["window_text"]
    palette.brightText: Cpp_ThemeManager.colors["bright_text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.toolTipBase: Cpp_ThemeManager.colors["tooltip_base"]
    palette.toolTipText: Cpp_ThemeManager.colors["tooltip_text"]
    palette.linkVisited: Cpp_ThemeManager.colors["link_visited"]
    palette.alternateBase: Cpp_ThemeManager.colors["alternate_base"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    ColumnLayout {
      id: layout
      spacing: 4
      anchors.margins: 16
      anchors.fill: parent

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
          } ComboBox {
            id: _themeCombo
            Layout.fillWidth: true
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
            color: Cpp_ThemeManager.colors["text"]
          } RowLayout {
            spacing: 2

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
          // Plugins enabled
          //
          Label {
            text: qsTr("Enable TCP Plugins (Port 7777)")
          } Switch {
            id: _tcpPlugins
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_Plugins_Bridge.enabled
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_Plugins_Bridge.enabled)
                Cpp_Plugins_Bridge.enabled = checked
            }
          }

          //
          // Software rendering
          //
          Label {
            text: qsTr("Force Software Rendering")
          } Switch {
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_Misc_ModuleManager.softwareRendering
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (Cpp_Misc_ModuleManager.softwareRendering !== checked)
                Cpp_Misc_ModuleManager.softwareRendering = checked
            }
          }

          //
          // Auto-updater
          //
          Label {
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
          } SpinBox {
            id: _points

            from: 2
            to: 20000
            editable: true
            Layout.fillWidth: true
            value: Cpp_UI_Dashboard.points
            onValueChanged: {
              if (value !== Cpp_UI_Dashboard.points)
                Cpp_UI_Dashboard.points = value
            }
          }

          //
          // Decimal digits
          //
          Label {
            text: qsTr("Decimal Precision")
          } SpinBox {
            id: _decimalDigits

            to: 5
            from: 0
            editable: true
            Layout.fillWidth: true
            value: Cpp_UI_Dashboard.precision
            onValueChanged: {
              if (value !== Cpp_UI_Dashboard.precision)
                Cpp_UI_Dashboard.precision = value
            }
          }

          //
          // Console
          //
          Label {
            text: qsTr("Enable Console Widget")
          } Switch {
            id: _consoleWidget
            Layout.rightMargin: -8
            Layout.alignment: Qt.AlignRight
            checked: Cpp_UI_Dashboard.terminalEnabled
            palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
            onCheckedChanged: {
              if (checked !== Cpp_UI_Dashboard.terminalEnabled)
                Cpp_UI_Dashboard.terminalEnabled = checked
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
          text: qsTr("Reset") + " "
          opacity: enabled ? 1 : 0.5
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/rcc/icons/buttons/refresh.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
          onClicked: {
            Cpp_ThemeManager.theme = 0
            Cpp_UI_Dashboard.points = 100
            Cpp_UI_Dashboard.precision = 2
            Cpp_Plugins_Bridge.enabled = false
            mainWindow.automaticUpdates  = true
            Cpp_UI_Dashboard.terminalEnabled = false
            Cpp_Misc_ModuleManager.softwareRendering = false
          }
        }

        Item {
          implicitWidth: 96
          Layout.fillWidth: true
        }

        Button {
          icon.width: 18
          icon.height: 18
          onClicked: root.close()
          text: qsTr("Close") + " "
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/rcc/icons/buttons/close.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
        }

        Button {
          icon.width: 18
          icon.height: 18
          onClicked: root.close()
          text: qsTr("Apply") + " "
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/rcc/icons/buttons/apply.svg"
          icon.color: Cpp_ThemeManager.colors["button_text"]
        }
      }
    }
  }
}
