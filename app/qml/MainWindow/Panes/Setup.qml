/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "SetupPanes" as SetupPanes
import "../../Widgets" as Widgets

Widgets.Pane {
  id: root
  title: qsTr("Device Setup")
  icon: "qrc:/icons/panes/setup.svg"

  //
  // Custom properties
  //
  property int setupMargin: 0
  property int displayedWidth: 344
  readonly property int maxItemWidth: column.width - 8

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
    //
    // Misc settings
    //
    property alias auto: commAuto.checked
    property alias manual: commManual.checked
    property alias tabIndex: tab.currentIndex
    property alias csvExport: csvLogging.checked

    //
    // App settings
    //
    property alias language: settings.language
    property alias tcpPlugins: settings.tcpPlugins
  }

  //
  // Update manual/auto checkboxes
  //
  Connections {
    target: Cpp_JSON_Generator
    function onOperationModeChanged() {
      commAuto.checked = (Cpp_JSON_Generator.operationMode === 1)
      commManual.checked = (Cpp_JSON_Generator.operationMode === 0)
    }
  }

  //
  // Control arrangement
  //
  ColumnLayout {
    id: column
    spacing: 4
    anchors.fill: parent
    anchors.topMargin: -6

    //
    // Device type selector
    //
    Label {
      text: qsTr("Device Setup") + ":"
      font: Cpp_Misc_CommonFonts.customUiFont(10, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    } ComboBox {
      id: _driverCombo
      Layout.fillWidth: true
      model: Cpp_IO_Manager.availableDrivers()
      displayText: qsTr("I/O Interface: %1").arg(currentText)
      onCurrentIndexChanged: Cpp_IO_Manager.selectedDriver = currentIndex
    }

    //
    // CSV generator
    //
    Switch {
      id: csvLogging
      Layout.leftMargin: -6
      text: qsTr("Create CSV File")
      Layout.alignment: Qt.AlignLeft
      checked: Cpp_CSV_Export.exportEnabled
      palette.highlight: Cpp_ThemeManager.colors["csv_switch"]

      onCheckedChanged:  {
        if (Cpp_CSV_Export.exportEnabled !== checked)
          Cpp_CSV_Export.exportEnabled = checked
      }
    }

    //
    // Spacer
    //
    Item {
      height: 4
    }

    //
    // Comm mode selector
    //
    Label {
      text: qsTr("Frame Parsing") + ":"
      font: Cpp_Misc_CommonFonts.customUiFont(10, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    } RadioButton {
      id: commAuto
      Layout.maximumHeight: 18
      Layout.maximumWidth: root.maxItemWidth
      checked: Cpp_JSON_Generator.operationMode === 1
      text: qsTr("No Parsing (Device Sends JSON Data)")
      onCheckedChanged: {
        if (checked && Cpp_JSON_Generator.operationMode !== 1)
          Cpp_JSON_Generator.setOperationMode(1)
        else if (!checked)
          Cpp_JSON_Generator.setOperationMode(0)
      }
    } RadioButton {
      id: commManual
      Layout.maximumHeight: 18
      Layout.maximumWidth: root.maxItemWidth
      text: qsTr("Parse via JSON Project File")
      checked: Cpp_JSON_Generator.operationMode === 0
      onCheckedChanged: {
        if (checked && Cpp_JSON_Generator.operationMode !== 0)
          Cpp_JSON_Generator.setOperationMode(0)
        else if (!checked)
          Cpp_JSON_Generator.setOperationMode(1)
      }
    }

    //
    // Map file selector button
    //
    Button {
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      enabled: commManual.checked
      Layout.maximumWidth: root.maxItemWidth
      onClicked: Cpp_JSON_Generator.loadJsonMap()
      text: (Cpp_JSON_Generator.jsonMapFilename.length ?
               qsTr("Change Project File (%1)").arg(Cpp_JSON_Generator.jsonMapFilename) :
               qsTr("Select Project File") + "...")
    }

    //
    // Spacer
    //
    Item {
      height: 4
    }

    //
    // Tab bar
    //
    TabBar {
      height: 24
      id: tab
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

      SetupPanes.Hardware {
        id: hardware
        Layout.fillWidth: true
        Layout.fillHeight: true
        background: Rectangle {
          radius: 2
          border.width: 1
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }
      }

      SetupPanes.Settings {
        id: settings
        Layout.fillWidth: true
        Layout.fillHeight: true
        background: Rectangle {
          radius: 2
          border.width: 1
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }
      }
    }
  }
}
