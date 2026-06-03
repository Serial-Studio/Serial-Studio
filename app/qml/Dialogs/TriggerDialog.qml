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
  // Shared metrics so every row lines up like a front panel
  //
  readonly property int labelColumn: 64
  readonly property int controlColumn: 184

  //
  // Reflect external trigger-level changes (e.g. dragging the level line on the
  // plot) into the field, unless the user is currently editing it.
  //
  Connections {
    target: root.model
    enabled: root.model !== null && root.visible

    function onSweepChanged() {
      if (!levelField.activeFocus)
        levelField.text = root.model.triggerLevel.toFixed(4)
    }
  }

  //
  // Dialog controls
  //
  dialogContent: ColumnLayout {
    id: layout

    spacing: 4
    enabled: root.model !== null

    //
    // Mode section
    //
    Label {
      text: qsTr("Mode")
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

      ColumnLayout {
        spacing: 6
        anchors.fill: parent

        ColumnLayout {
          spacing: 2
          Layout.fillWidth: true

          Repeater {
            model: [qsTr("Auto"), qsTr("Normal"), qsTr("Single")]

            delegate: RadioButton {
              required property int index
              required property string modelData

              text: modelData
              checked: root.model && root.model.sweepMode === root.modeValues[index]
              onClicked: {
                if (root.model)
                  root.model.sweepMode = root.modeValues[index]
              }
            }
          }
        }

        Label {
          opacity: 0.6
          Layout.fillWidth: true
          wrapMode: Text.WordWrap
          Layout.preferredWidth: root.labelColumn + root.controlColumn
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.customUiFont(0.8)
          text: qsTr("Auto free-runs when nothing crosses the level.") + "\n"
                + qsTr("Normal waits for a crossing.") + "\n"
                + qsTr("Single captures one sweep, then stops.")
        }
      }
    }

    Item {
      implicitHeight: 4
    }

    //
    // Trigger section
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
        columnSpacing: 10
        anchors.fill: parent

        Label {
          visible: root.isMultiPlot
          text: qsTr("Source:")
          Layout.minimumWidth: root.labelColumn
          color: Cpp_ThemeManager.colors["text"]
        }

        ComboBox {
          id: sourceBox

          visible: root.isMultiPlot
          Layout.fillWidth: true
          Layout.minimumWidth: root.controlColumn
          model: root.model ? root.model.labels : []
          currentIndex: (root.isMultiPlot && root.model) ? root.model.triggerSource : 0
          onActivated: {
            if (root.model)
              root.model.triggerSource = currentIndex
          }
        }

        Label {
          text: qsTr("Level:")
          Layout.minimumWidth: root.labelColumn
          color: Cpp_ThemeManager.colors["text"]
        }

        Widgets.LineField {
          id: levelField

          selectByMouse: true
          Layout.fillWidth: true
          Layout.minimumWidth: root.controlColumn
          validator: DoubleValidator {}
          font: Cpp_Misc_CommonFonts.monoFont
          placeholderText: qsTr("Value to cross")
          onTextChanged: {
            if (activeFocus && acceptableInput && root.model)
              Qt.callLater(function() { root.model.triggerLevel = parseFloat(levelField.text) })
          }
        }

        Label {
          text: qsTr("Slope:")
          Layout.minimumWidth: root.labelColumn
          color: Cpp_ThemeManager.colors["text"]
        }

        RowLayout {
          spacing: -1
          Layout.fillWidth: true
          Layout.minimumWidth: root.controlColumn

          Button {
            checkable: true
            autoExclusive: true
            text: qsTr("Rising")
            Layout.fillWidth: true
            icon.color: Cpp_ThemeManager.colors["text"]
            icon.source: "qrc:/icons/buttons/rising-edge.svg"
            ToolTip.text: qsTr("Trigger on an upward crossing")
            checked: root.model && root.model.triggerEdge === SerialStudio.TriggerRising
            onClicked: {
              if (root.model)
                root.model.triggerEdge = SerialStudio.TriggerRising
            }
          }

          Button {
            checkable: true
            autoExclusive: true
            text: qsTr("Falling")
            Layout.fillWidth: true
            icon.color: Cpp_ThemeManager.colors["text"]
            icon.source: "qrc:/icons/buttons/falling-edge.svg"
            ToolTip.text: qsTr("Trigger on a downward crossing")
            checked: root.model && root.model.triggerEdge === SerialStudio.TriggerFalling
            onClicked: {
              if (root.model)
                root.model.triggerEdge = SerialStudio.TriggerFalling
            }
          }
        }
      }
    }

    Item {
      implicitHeight: 4
    }

    //
    // Timing section
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

      ColumnLayout {
        spacing: 6
        anchors.fill: parent

        GridLayout {
          columns: 3
          rowSpacing: 6
          columnSpacing: 10
          Layout.fillWidth: true

          Label {
            text: qsTr("Timebase:")
            Layout.minimumWidth: root.labelColumn
            color: Cpp_ThemeManager.colors["text"]
          }

          Widgets.LineField {
            id: timebaseField

            selectByMouse: true
            Layout.fillWidth: true
            Layout.minimumWidth: root.controlColumn - 28
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
            text: qsTr("ms")
            opacity: 0.6
            color: Cpp_ThemeManager.colors["text"]
          }

          Label {
            text: qsTr("Holdoff:")
            Layout.minimumWidth: root.labelColumn
            color: Cpp_ThemeManager.colors["text"]
          }

          Widgets.LineField {
            id: holdoffField

            selectByMouse: true
            Layout.fillWidth: true
            Layout.minimumWidth: root.controlColumn - 28
            validator: DoubleValidator { bottom: 0 }
            font: Cpp_Misc_CommonFonts.monoFont
            placeholderText: qsTr("0")
            onTextChanged: {
              if (activeFocus && acceptableInput && root.model)
                Qt.callLater(function() { root.model.holdoff = parseFloat(holdoffField.text) })
            }
          }

          Label {
            text: qsTr("ms")
            opacity: 0.6
            color: Cpp_ThemeManager.colors["text"]
          }
        }

        Label {
          opacity: 0.6
          Layout.fillWidth: true
          wrapMode: Text.WordWrap
          Layout.preferredWidth: root.labelColumn + root.controlColumn
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.customUiFont(0.8)
          text: qsTr("Leave timebase empty to use the plot's time range; lower it to zoom in "
                     + "on a fast signal. Holdoff ignores new triggers for a moment after each.")
        }
      }
    }

    Item {
      implicitHeight: 8
    }

    RowLayout {
      spacing: 8
      Layout.fillWidth: true

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
