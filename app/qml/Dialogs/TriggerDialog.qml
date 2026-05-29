/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

import SerialStudio

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Custom properties
  //
  property var model: null
  property bool isMultiPlot: false

  //
  // Mode/edge value tables (avoid hardcoded enum integers in bindings)
  //
  readonly property var modeValues: [SerialStudio.SweepAuto,
                                      SerialStudio.SweepNormal,
                                      SerialStudio.SweepSingle]

  //
  // Window properties
  //
  staysOnTop: true
  title: qsTr("Trigger Settings")

  //
  // Direct CSD size hints (bypasses Page implicit-size propagation)
  //
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Display the dialog and load information from the data model pointer
  //
  function openDialog(dataModel, multiPlot) {
    root.model = dataModel
    root.isMultiPlot = multiPlot === true

    if (root.model) {
      levelField.text = root.model.triggerLevel.toFixed(4)
      holdoffField.text = root.model.holdoff.toFixed(0)
      timebaseField.text = root.model.sweepTimebase > 0
                           ? root.model.sweepTimebase.toFixed(0)
                           : ""
    }

    root.show()
    root.raise()
  }

  //
  // Dialog controls
  //
  dialogContent: ColumnLayout {
    id: layout

    spacing: 4
    enabled: root.model !== null

    Label {
      opacity: 0.7
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      Layout.minimumWidth: 380
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.9)
      text: qsTr("Lock a repeating signal in place, like the Auto button on an oscilloscope. "
                 + "Each sweep starts at the same point on the waveform, so it holds still "
                 + "instead of scrolling past.")
    }

    Item {
      implicitHeight: 4
    }

    //
    // Trigger section: what starts each sweep
    //
    Label {
      text: qsTr("Trigger")
      font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    }

    GroupBox {
      Layout.fillWidth: true

      background: Rectangle {
        radius: 2
        border.width: 1
        border.color: Cpp_ThemeManager.colors["groupbox_border"]
        color: Cpp_ThemeManager.colors["groupbox_background"]
      }

      GridLayout {
        columns: 2
        rowSpacing: 6
        columnSpacing: 8
        anchors.fill: parent

        Label {
          text: qsTr("Mode:")
          color: Cpp_ThemeManager.colors["text"]
        }

        ComboBox {
          id: modeBox

          Layout.fillWidth: true
          Layout.minimumWidth: 180
          model: [qsTr("Auto"), qsTr("Normal"), qsTr("Single")]
          currentIndex: root.model ? root.model.sweepMode : 0
          onActivated: {
            if (root.model)
              root.model.sweepMode = root.modeValues[currentIndex]
          }
        }

        Label {
          visible: root.isMultiPlot
          text: qsTr("Signal:")
          color: Cpp_ThemeManager.colors["text"]
        }

        ComboBox {
          id: sourceBox

          visible: root.isMultiPlot
          Layout.fillWidth: true
          Layout.minimumWidth: 180
          model: root.model ? root.model.labels : []
          currentIndex: root.model ? root.model.triggerSource : 0
          onActivated: {
            if (root.model)
              root.model.triggerSource = currentIndex
          }
        }

        Label {
          text: qsTr("Level:")
          color: Cpp_ThemeManager.colors["text"]
        }

        Widgets.LineField {
          id: levelField

          selectByMouse: true
          Layout.fillWidth: true
          Layout.minimumWidth: 180
          validator: DoubleValidator {}
          font: Cpp_Misc_CommonFonts.monoFont
          placeholderText: qsTr("Value to cross")
          onTextChanged: {
            if (activeFocus && acceptableInput && root.model)
              Qt.callLater(function() { root.model.triggerLevel = parseFloat(levelField.text) })
          }
        }

        Label {
          text: qsTr("Edge:")
          color: Cpp_ThemeManager.colors["text"]
        }

        RowLayout {
          spacing: 12
          Layout.fillWidth: true

          RadioButton {
            text: qsTr("Rising")
            checked: root.model && root.model.triggerEdge === SerialStudio.TriggerRising
            onClicked: {
              if (root.model)
                root.model.triggerEdge = SerialStudio.TriggerRising
            }
          }

          RadioButton {
            text: qsTr("Falling")
            checked: root.model && root.model.triggerEdge === SerialStudio.TriggerFalling
            onClicked: {
              if (root.model)
                root.model.triggerEdge = SerialStudio.TriggerFalling
            }
          }
        }
      }
    }

    Label {
      opacity: 0.6
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      Layout.minimumWidth: 380
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.8)
      text: qsTr("A new sweep begins each time the signal crosses the level in the chosen "
                 + "direction. Auto also free-runs when no crossing is found.")
    }

    Item {
      implicitHeight: 4
    }

    //
    // Timing section: how wide each sweep is and how often it repeats
    //
    Label {
      text: qsTr("Timing")
      font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    }

    GroupBox {
      Layout.fillWidth: true

      background: Rectangle {
        radius: 2
        border.width: 1
        border.color: Cpp_ThemeManager.colors["groupbox_border"]
        color: Cpp_ThemeManager.colors["groupbox_background"]
      }

      GridLayout {
        columns: 2
        rowSpacing: 6
        columnSpacing: 8
        anchors.fill: parent

        Label {
          text: qsTr("Timebase (ms):")
          color: Cpp_ThemeManager.colors["text"]
        }

        Widgets.LineField {
          id: timebaseField

          selectByMouse: true
          Layout.fillWidth: true
          Layout.minimumWidth: 180
          validator: DoubleValidator { bottom: 0 }
          font: Cpp_Misc_CommonFonts.monoFont
          placeholderText: qsTr("Match time range")
          onTextChanged: {
            if (activeFocus && root.model)
              Qt.callLater(function() {
                root.model.sweepTimebase = timebaseField.text.length > 0
                                           ? parseFloat(timebaseField.text)
                                           : 0
              })
          }
        }

        Label {
          text: qsTr("Holdoff (ms):")
          color: Cpp_ThemeManager.colors["text"]
        }

        Widgets.LineField {
          id: holdoffField

          selectByMouse: true
          Layout.fillWidth: true
          Layout.minimumWidth: 180
          validator: DoubleValidator { bottom: 0 }
          font: Cpp_Misc_CommonFonts.monoFont
          placeholderText: qsTr("0")
          onTextChanged: {
            if (activeFocus && acceptableInput && root.model)
              Qt.callLater(function() { root.model.holdoff = parseFloat(holdoffField.text) })
          }
        }
      }
    }

    Label {
      opacity: 0.6
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      Layout.minimumWidth: 380
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.8)
      text: qsTr("Timebase sets how much time one sweep shows; leave it empty to use the "
                 + "plot's time range. Lower it to zoom in on a fast signal. Holdoff ignores "
                 + "new triggers for a moment after each one.")
    }

    Item {
      implicitHeight: 8
    }

    RowLayout {
      spacing: 8
      Layout.alignment: Qt.AlignRight

      Widgets.IconButton {
        text: qsTr("Capture Next")
        icon.source: "qrc:/icons/buttons/refresh.svg"
        ToolTip.text: qsTr("Arm for one more single-shot capture")
        visible: root.model && root.model.sweepMode === SerialStudio.SweepSingle
        onClicked: {
          if (root.model)
            root.model.armSweep()
        }
      }

      Item {
        Layout.fillWidth: true
      }

      Widgets.IconButton {
        text: qsTr("Close")
        onClicked: root.close()
        icon.source: "qrc:/icons/buttons/close.svg"
      }
    }
  }
}
