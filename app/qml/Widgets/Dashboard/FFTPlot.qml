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
import QtGraphs
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../"
import "../../Dialogs" as Dialogs

Item {
  id: root

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property string widgetId
  required property FFTPlotModel model

  //
  // Window flags
  //
  property bool hasToolbar: true

  //
  // Custom properties
  //
  property bool showAreaUnderPlot: false

  //
  // User-controlled visibility preferences (persisted, ANDed with size thresholds)
  //
  property bool userShowXLabel: true
  property bool userShowYLabel: true

  //
  // Set downsample size based on widget size & zoom factor
  //
  function setDownsampleFactor()
  {
    const z = plot.zoom
    model.dataW = plot.plotArea.width * z
    model.dataH = plot.plotArea.height * z
  }

  //
  // Sync model width/height with widget, then restore persisted settings
  //
  Component.onCompleted: {
    root.setDownsampleFactor()

    const s = Cpp_JSON_ProjectModel.widgetSettings(widgetId)

    if (s["showAreaUnderPlot"] !== undefined)
      root.showAreaUnderPlot = s["showAreaUnderPlot"]

    if (s["userShowXLabel"] !== undefined)
      root.userShowXLabel = s["userShowXLabel"]

    if (s["userShowYLabel"] !== undefined)
      root.userShowYLabel = s["userShowYLabel"]
  }

  //
  // Enable/disable features depending on window size, ANDed with user preferences
  //
  onWidthChanged: updateWidgetOptions()
  onHeightChanged: updateWidgetOptions()
  function updateWidgetOptions() {
    plot.yLabelVisible = root.userShowYLabel && (root.width >= 196)
    plot.xLabelVisible = root.userShowXLabel && (root.height >= (196 * 2/3))
    root.hasToolbar = (root.width >= toolbar.implicitWidth) && (root.height >= 220)
  }

  //
  // Axis range configuration dialog
  //
  Dialogs.AxisRangeDialog {
    id: axisRangeDialog
  }

  //
  // Update curve at 24 Hz
  //
  Connections {
    target: Cpp_Misc_TimerEvents

    function onUiTimeout() {
      if (root.visible && root.model) {
        root.model.draw(upperSeries)
        lowerSeries.clear()
        lowerSeries.append(root.model.minX, root.model.minY)
        lowerSeries.append(root.model.maxX, root.model.minY)
      }
    }
  }

  //
  // Add toolbar
  //
  RowLayout {
    id: toolbar

    spacing: 4
    visible: root.hasToolbar
    height: root.hasToolbar ? 48 : 0

    anchors {
      leftMargin: 8
      top: parent.top
      left: parent.left
      right: parent.right
    }

    ToolButton {
      width: 24
      height: 24
      onClicked: {
        root.showAreaUnderPlot = !root.showAreaUnderPlot
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "showAreaUnderPlot", root.showAreaUnderPlot)
      }
      icon.width: 18
      icon.height: 18
      opacity: enabled ? 1 : 0.5
      icon.color: "transparent"
      checked: root.showAreaUnderPlot
      icon.source: "qrc:/rcc/icons/dashboard-buttons/area.svg"
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    ToolButton {
      width: 24
      height: 24
      onClicked: {
        root.userShowXLabel = !root.userShowXLabel
        root.updateWidgetOptions()
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "userShowXLabel", root.userShowXLabel)
      }
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      checked: root.userShowXLabel
      icon.source: "qrc:/rcc/icons/dashboard-buttons/x.svg"
    }

    ToolButton {
      width: 24
      height: 24
      onClicked: {
        root.userShowYLabel = !root.userShowYLabel
        root.updateWidgetOptions()
        Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "userShowYLabel", root.userShowYLabel)
      }
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      checked: root.userShowYLabel
      icon.source: "qrc:/rcc/icons/dashboard-buttons/y.svg"
    }

    Rectangle {
      implicitWidth: 1
      implicitHeight: 24
      color: Cpp_ThemeManager.colors["widget_border"]
    }

    ToolButton {
      width: 24
      height: 24
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      checked: plot.showCrosshairs
      onClicked: plot.showCrosshairs = !plot.showCrosshairs
      icon.source: "qrc:/rcc/icons/dashboard-buttons/crosshair.svg"
    }

    ToolButton {
      width: 24
      height: 24
      icon.width: 18
      icon.height: 18
      checked: !model.running
      icon.color: "transparent"
      icon.source: model.running?
                     "qrc:/rcc/icons/dashboard-buttons/pause.svg" :
                     "qrc:/rcc/icons/dashboard-buttons/resume.svg"
      onClicked: model.running = !model.running
    }

    ToolButton {
      width: 24
      height: 24
      onClicked: {
        plot.xAxis.pan = 0
        plot.yAxis.pan = 0
        plot.xAxis.zoom = 1
        plot.yAxis.zoom = 1
        plot.xMin = Qt.binding(function() { return root.model.minX })
        plot.xMax = Qt.binding(function() { return root.model.maxX })
        plot.yMin = Qt.binding(function() { return root.model.minY })
        plot.yMax = Qt.binding(function() { return root.model.maxY })
      }
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      opacity: enabled ? 1 : 0.5
      enabled: plot.xAxis.zoom !== 1 || plot.yAxis.zoom !== 1
      icon.source: "qrc:/rcc/icons/dashboard-buttons/return.svg"
    }

    ToolButton {
      width: 24
      height: 24
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      icon.source: "qrc:/rcc/icons/toolbar/settings.svg"
      onClicked: axisRangeDialog.openDialog(plot, root.model)
    }

    Item {
      Layout.fillWidth: true
    }
  }

  //
  // Plot widget
  //
  PlotWidget {
    id: plot

    anchors {
      margins: 8
      left: parent.left
      right: parent.right
      top: toolbar.bottom
      bottom: parent.bottom
    }

    xMin: root.model.minX
    xMax: root.model.maxX
    yMin: root.model.minY
    yMax: root.model.maxY
    curveColors: [root.color]
    yLabel: qsTr("Magnitude (dB)")
    xLabel: qsTr("Frequency (Hz)")
    mouseAreaEnabled: windowRoot.focused
    xAxis.tickInterval: plot.xTickInterval
    yAxis.tickInterval: plot.yTickInterval

    onZoomChanged: root.setDownsampleFactor()
    onWidthChanged: root.setDownsampleFactor()
    onHeightChanged: root.setDownsampleFactor()

    Component.onCompleted: {
      graph.addSeries(areaSeries)
      graph.addSeries(upperSeries)
      graph.addSeries(lowerSeries)
    }

    LineSeries {
      id: upperSeries

      width: 2
    }

    LineSeries {
      id: lowerSeries

      width: 0
      visible: false
    }

    AreaSeries {
      id: areaSeries

      upperSeries: upperSeries
      lowerSeries: lowerSeries
      borderColor: "transparent"
      visible: root.showAreaUnderPlot
      color: Qt.rgba(root.color.r, root.color.g, root.color.b, 0.2)
    }
  }
}
