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

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Custom properties
  //
  property real customXMin: 0
  property real customXMax: 0
  property real customYMin: 0
  property real customYMax: 0
  property var dataModel: null
  property var plotWidget: null
  property bool timeAxis: false
  property bool hasCustomRanges: false

  //
  // Window properties
  //
  staysOnTop: true
  title: qsTr("Axis Range Configuration")

  //
  // Direct CSD size hints (bypasses Page implicit-size propagation)
  //
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Display the dialog & load information from data model pointer
  //
  function openDialog(plot, model) {
    root.plotWidget = plot
    root.dataModel = model
    root.timeAxis = (model !== null && model.timeAxis === true)

    if (plot) {
      customXMin = plot.xMin
      customXMax = plot.xMax
      customYMin = plot.yMin
      customYMax = plot.yMax

      updateFields()
    }

    root.show()
    root.raise()
  }

  //
  // Update UI values based on plot widget data
  //
  function updateFields() {
    if (plotWidget) {
      if (!root.timeAxis) {
        xMinField.text = plotWidget.xMin.toFixed(4)
        xMaxField.text = plotWidget.xMax.toFixed(4)
      }

      yMinField.text = plotWidget.yMin.toFixed(4)
      yMaxField.text = plotWidget.yMax.toFixed(4)
    }
  }

  //
  // Apply the changes to the plot widget
  //
  function applyChanges() {
    if (!plotWidget)
      return

    if (!root.timeAxis) {
      const newXMin = parseFloat(xMinField.text)
      const newXMax = parseFloat(xMaxField.text)
      if (!isNaN(newXMin) && !isNaN(newXMax) && newXMin < newXMax) {
        plotWidget.xMin = newXMin
        plotWidget.xMax = newXMax
        customXMin = newXMin
        customXMax = newXMax
        hasCustomRanges = true
      } else {
        xMinField.text = plotWidget.xMin.toFixed(4)
        xMaxField.text = plotWidget.xMax.toFixed(4)
      }
    }

    const newYMin = parseFloat(yMinField.text)
    const newYMax = parseFloat(yMaxField.text)
    if (!isNaN(newYMin) && !isNaN(newYMax) && newYMin < newYMax) {
      plotWidget.yMin = newYMin
      plotWidget.yMax = newYMax
      customYMin = newYMin
      customYMax = newYMax
      hasCustomRanges = true
    } else {
      yMinField.text = plotWidget.yMin.toFixed(4)
      yMaxField.text = plotWidget.yMax.toFixed(4)
    }
  }

  //
  // Reset the plot widget configuration to initial state
  //
  function resetToAuto() {
    if (!plotWidget || !dataModel)
      return

    if (!root.timeAxis) {
      plotWidget.xMin = Qt.binding(function() { return dataModel.minX })
      plotWidget.xMax = Qt.binding(function() { return dataModel.maxX })
    }

    plotWidget.yMin = Qt.binding(function() { return dataModel.minY })
    plotWidget.yMax = Qt.binding(function() { return dataModel.maxY })

    hasCustomRanges = false
    updateFields()
  }

  //
  // Dialog controls
  //
  dialogContent: ColumnLayout {
    id: layout

    spacing: 4

    Label {
      opacity: 0.7
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      Layout.minimumWidth: 356
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.9)
      text: qsTr("Configure the visible range for the plot axes. Values update in real-time as you type.")
    }

    Item {
      implicitHeight: 4
    }

    Label {
      text: qsTr("X Axis")
      visible: !root.timeAxis
      font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    }

    GroupBox {
      Layout.fillWidth: true
      visible: !root.timeAxis

      background: Rectangle {
        radius: 2
        border.width: 1
        border.color: Cpp_ThemeManager.colors["groupbox_border"]
        color: Cpp_ThemeManager.colors["groupbox_background"]
      }

      GridLayout {
        columns: 2
        rowSpacing: 4
        columnSpacing: 8
        anchors.fill: parent

        Label {
          text: qsTr("Minimum:")
          color: Cpp_ThemeManager.colors["text"]
        }

        Widgets.LineField {
          id: xMinField

          selectByMouse: true
          Layout.fillWidth: true
          Layout.minimumWidth: 140
          validator: DoubleValidator {}
          font: Cpp_Misc_CommonFonts.monoFont
          placeholderText: qsTr("Enter min value")
          onTextChanged: {
            if (activeFocus && acceptableInput) {
              Qt.callLater(root.applyChanges)
            }
          }
        }

        Label {
          text: qsTr("Maximum:")
          color: Cpp_ThemeManager.colors["text"]
        }

        Widgets.LineField {
          id: xMaxField

          selectByMouse: true
          Layout.fillWidth: true
          Layout.minimumWidth: 140
          validator: DoubleValidator {}
          font: Cpp_Misc_CommonFonts.monoFont
          placeholderText: qsTr("Enter max value")
          onTextChanged: {
            if (activeFocus && acceptableInput) {
              Qt.callLater(root.applyChanges)
            }
          }
        }
      }
    }

    Item {
      implicitHeight: 4
    }

    Label {
      text: qsTr("Y Axis")
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
        rowSpacing: 4
        columnSpacing: 8
        anchors.fill: parent

        Label {
          text: qsTr("Minimum:")
          color: Cpp_ThemeManager.colors["text"]
        }

        Widgets.LineField {
          id: yMinField

          selectByMouse: true
          Layout.fillWidth: true
          Layout.minimumWidth: 140
          validator: DoubleValidator {}
          font: Cpp_Misc_CommonFonts.monoFont
          placeholderText: qsTr("Enter min value")
          onTextChanged: {
            if (activeFocus && acceptableInput) {
              Qt.callLater(root.applyChanges)
            }
          }
        }

        Label {
          text: qsTr("Maximum:")
          color: Cpp_ThemeManager.colors["text"]
        }

        Widgets.LineField {
          id: yMaxField

          selectByMouse: true
          Layout.fillWidth: true
          Layout.minimumWidth: 140
          validator: DoubleValidator {}
          font: Cpp_Misc_CommonFonts.monoFont
          placeholderText: qsTr("Enter max value")
          onTextChanged: {
            if (activeFocus && acceptableInput) {
              Qt.callLater(root.applyChanges)
            }
          }
        }
      }
    }

    Item {
      implicitHeight: 8
    }

    RowLayout {
      spacing: 8
      Layout.alignment: Qt.AlignRight

      Widgets.IconButton {
        text: qsTr("Reset")
        onClicked: root.resetToAuto()
        icon.source: "qrc:/icons/buttons/refresh.svg"
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
