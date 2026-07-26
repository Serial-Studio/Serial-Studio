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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
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
  // Log-axis handling: the plot widget's world coordinates are log10 units on a log
  // axis, so the dialog converts to true values for display and back on commit
  //
  readonly property bool logX: plotWidget !== null && plotWidget.logX === true
  readonly property bool logY: plotWidget !== null && plotWidget.logY === true

  function toDisplayValue(worldValue, log) {
    return log ? Math.pow(10, worldValue) : worldValue
  }

  function toWorldValue(displayValue, log) {
    return log ? Math.log10(Math.max(displayValue, 1e-12)) : displayValue
  }

  function formatValue(worldValue, log) {
    if (log)
      return Number(toDisplayValue(worldValue, log).toPrecision(6)).toString()

    return worldValue.toFixed(4)
  }

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
        xMinField.text = formatValue(plotWidget.xMin, root.logX)
        xMaxField.text = formatValue(plotWidget.xMax, root.logX)
      }

      yMinField.text = formatValue(plotWidget.yMin, root.logY)
      yMaxField.text = formatValue(plotWidget.yMax, root.logY)
    }
  }

  //
  // Apply the changes to the plot widget
  //
  function applyChanges() {
    if (!plotWidget)
      return

    if (!root.timeAxis) {
      const newXMin = toWorldValue(parseFloat(xMinField.text), root.logX)
      const newXMax = toWorldValue(parseFloat(xMaxField.text), root.logX)
      if (!isNaN(newXMin) && !isNaN(newXMax) && newXMin < newXMax) {
        plotWidget.xMin = newXMin
        plotWidget.xMax = newXMax
        customXMin = newXMin
        customXMax = newXMax
        hasCustomRanges = true
      } else {
        if (!xMinField.activeFocus)
          xMinField.text = formatValue(plotWidget.xMin, root.logX)

        if (!xMaxField.activeFocus)
          xMaxField.text = formatValue(plotWidget.xMax, root.logX)
      }
    }

    const newYMin = toWorldValue(parseFloat(yMinField.text), root.logY)
    const newYMax = toWorldValue(parseFloat(yMaxField.text), root.logY)
    if (!isNaN(newYMin) && !isNaN(newYMax) && newYMin < newYMax) {
      plotWidget.yMin = newYMin
      plotWidget.yMax = newYMax
      customYMin = newYMin
      customYMax = newYMax
      hasCustomRanges = true
    } else {
      if (!yMinField.activeFocus)
        yMinField.text = formatValue(plotWidget.yMin, root.logY)

      if (!yMaxField.activeFocus)
        yMaxField.text = formatValue(plotWidget.yMax, root.logY)
    }
  }

  //
  // Reset the plot widget configuration to initial state
  //
  function resetToAuto() {
    if (!plotWidget || !dataModel)
      return

    if (!root.timeAxis) {
      plotWidget.xMin = Qt.binding(function() { return dataModel ? dataModel.minX : 0 })
      plotWidget.xMax = Qt.binding(function() { return dataModel ? dataModel.maxX : 0 })
    }

    plotWidget.yMin = Qt.binding(function() { return dataModel ? dataModel.minY : 0 })
    plotWidget.yMax = Qt.binding(function() { return dataModel ? dataModel.maxY : 0 })

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
