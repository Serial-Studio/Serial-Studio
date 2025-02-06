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
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import "SetupPanes" as SetupPanes
import "../../Widgets" as Widgets

Widgets.Pane {
  id: root
  title: qsTr("Setup")
  icon: "qrc:/rcc/icons/panes/setup.svg"
  implicitHeight: layout.implicitHeight + 32

  //
  // Custom properties
  //
  property int setupMargin: 0
  property int displayedWidth: 360
  readonly property int maxItemWidth: layout.width - 8

  //
  // Displays the setup panel
  //
  function show() {
    setupMargin = 0
  }

  //
  // Hides the setup panel
  //
  function hide() {
    setupMargin = -1 * displayedWidth
  }

  //
  // Animations
  //
  visible: setupMargin > -1 * displayedWidth
  Behavior on setupMargin {NumberAnimation{}}

  //
  // Save settings
  //
  Settings {
    category: "SetupPanel"

    property alias language: settings.language
    property alias csvExport: csvLogging.checked
    property alias tcpPlugins: settings.tcpPlugins
    property alias consoleExport: consoleLogging.checked
    property alias selectedDriver: driverCombo.currentIndex
  }

  //
  // Use page item to set application palette
  //
  Page {
    implicitWidth: 0
    anchors.fill: parent
    anchors.topMargin: -16
    anchors.leftMargin: -9
    anchors.rightMargin: -9
    anchors.bottomMargin: -9

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

    //
    // Control arrangement
    //
    ColumnLayout {
      id: layout
      spacing: 4
      anchors {
        fill: parent
        topMargin: 10
        leftMargin: 9
        rightMargin: 9
        bottomMargin: 9
      }

      //

      //
      Label {
        text: qsTr("Device Setup") + ":"
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      } ComboBox {
        id: driverCombo
        Layout.fillWidth: true
        model: Cpp_IO_Manager.availableBuses
        displayText: qsTr("I/O Interface: %1").arg(currentText)
        onCurrentIndexChanged: {
          if (Cpp_IO_Manager.busType !== currentIndex)
            Cpp_IO_Manager.busType = currentIndex
        }
      }

      //
      // Spacer
      //
      Item {
        implicitHeight: 4
      }

      //
      // Comm mode selector
      //
      Label {
        text: qsTr("Frame Parsing") + ":"
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      } RadioButton {
        Layout.leftMargin: -6
        Layout.maximumHeight: 18
        Layout.maximumWidth: root.maxItemWidth
        text: qsTr("No Parsing (Device Sends JSON Data)")
        checked: Cpp_JSON_FrameBuilder.operationMode === SerialStudio.DeviceSendsJSON
        onCheckedChanged: {
          const shouldChange = Cpp_JSON_FrameBuilder.operationMode !== SerialStudio.DeviceSendsJSON
          if (checked && shouldChange)
            Cpp_JSON_FrameBuilder.operationMode = SerialStudio.DeviceSendsJSON
        }
      } RadioButton {
        Layout.leftMargin: -6
        Layout.maximumHeight: 18
        Layout.maximumWidth: root.maxItemWidth
        text: qsTr("Quick Plot (Comma Separated Values)")
        checked: Cpp_JSON_FrameBuilder.operationMode === SerialStudio.QuickPlot
        onCheckedChanged: {
          const shouldChange = Cpp_JSON_FrameBuilder.operationMode !== SerialStudio.QuickPlot
          if (checked && shouldChange)
            Cpp_JSON_FrameBuilder.operationMode = SerialStudio.QuickPlot
        }
      } RadioButton {
        Layout.leftMargin: -6
        Layout.maximumHeight: 18
        Layout.maximumWidth: root.maxItemWidth
        text: qsTr("Parse via JSON Project File")
        checked: Cpp_JSON_FrameBuilder.operationMode === SerialStudio.ProjectFile
        onCheckedChanged: {
          const shouldChange = Cpp_JSON_FrameBuilder.operationMode !== SerialStudio.ProjectFile
          if (checked && shouldChange)
            Cpp_JSON_FrameBuilder.operationMode = SerialStudio.ProjectFile
        }
      }

      //
      // Map file selector button
      //
      Button {
        Layout.fillWidth: true
        opacity: enabled ? 1 : 0.5
        Layout.maximumWidth: root.maxItemWidth
        onClicked: Cpp_JSON_FrameBuilder.loadJsonMap()
        enabled: Cpp_JSON_FrameBuilder.operationMode === SerialStudio.ProjectFile
        text: (Cpp_JSON_FrameBuilder.jsonMapFilename.length ?
                 qsTr("Change Project File (%1)").arg(Cpp_JSON_FrameBuilder.jsonMapFilename) :
                 qsTr("Select Project File") + "...")
      }

      //
      // Spacer
      //
      Item {
        implicitHeight: 4
      }

      //
      // Data export switches
      //
      Label {
        text: qsTr("Data Export") + ":"
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      }

      //
      // CSV generator
      //
      CheckBox {
        id: csvLogging
        Layout.leftMargin: -6
        Layout.maximumHeight: 18
        text: qsTr("Create CSV File")
        Layout.alignment: Qt.AlignLeft
        checked: Cpp_CSV_Export.exportEnabled
        Layout.maximumWidth: root.maxItemWidth

        onCheckedChanged:  {
          if (Cpp_CSV_Export.exportEnabled !== checked)
            Cpp_CSV_Export.exportEnabled = checked
        }
      }

      //
      // Console data export
      //
      CheckBox {
        id: consoleLogging
        Layout.leftMargin: -6
        Layout.maximumHeight: 18
        Layout.alignment: Qt.AlignLeft
        text: qsTr("Export Console Data")
        Layout.maximumWidth: root.maxItemWidth
        checked: Cpp_IO_ConsoleExport.exportEnabled

        onCheckedChanged:  {
          if (Cpp_IO_ConsoleExport.exportEnabled !== checked)
            Cpp_IO_ConsoleExport.exportEnabled = checked
        }
      }

      //
      // Spacer
      //
      Item {
        implicitHeight: 4
      }

      //
      // Tab bar
      //
      TabBar {
        id: tab
        implicitHeight: 24
        Layout.fillWidth: true
        Layout.maximumWidth: root.maxItemWidth

        TabButton {
          text: qsTr("Device")
          height: tab.height + 3
          width: implicitWidth + 2 * 8
        }

        TabButton {
          text: qsTr("Settings")
          height: tab.height + 3
          width: implicitWidth + 2 * 8
        }
      }

      //
      // Tab bar contents
      //
      StackLayout {
        id: stack
        clip: true
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: tab.currentIndex
        Layout.topMargin: -parent.spacing - 1
        implicitHeight: Math.max(hardware.implicitHeight, settings.implicitHeight)

        SetupPanes.Hardware {
          id: hardware
          Layout.fillWidth: true
          Layout.fillHeight: true
        }

        SetupPanes.Settings {
          id: settings
          Layout.fillWidth: true
          Layout.fillHeight: true
        }
      }
    }
  }
}
