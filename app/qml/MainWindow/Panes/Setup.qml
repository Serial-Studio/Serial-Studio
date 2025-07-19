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

import SerialStudio
import "SetupPanes" as SetupPanes
import "../../Widgets" as Widgets

Widgets.Pane {
  id: root
  title: qsTr("Device Setup")
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
    property alias csvExport: csvLogging.checked
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
        onClicked: Cpp_JSON_ProjectModel.openJsonFile()
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
      // Driver selection
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
          if (currentIndex < 0 || count <= currentIndex)
            currentIndex = 0

          if (Cpp_IO_Manager.busType !== currentIndex)
            Cpp_IO_Manager.busType = currentIndex
        }
      }

      //
      // Hardware setup
      //
      SetupPanes.Hardware {
        id: hardware
        Layout.fillWidth: true
        Layout.fillHeight: true
      }
    }
  }
}
