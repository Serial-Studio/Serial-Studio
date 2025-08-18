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
import QtCore as QtCore

import "../"

Item {
  id: root
  clip: true

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property PlotModel model

  //
  // Window flags
  //
  property bool hasToolbar: true

  //
  // Custom properties
  //
  property bool interpolate: true
  property bool showAreaUnderPlot: false

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
  // Sync model width/height with widget
  //
  Component.onCompleted: root.setDownsampleFactor()

  //
  // Save settings
  //
  QtCore.Settings {
    id: settings
    category: "Plot"
    property alias displayArea: root.showAreaUnderPlot
    property alias interpolateEnabled: root.interpolate
  }

  //
  // Enable/disable features depending on window size
  //
  onWidthChanged: updateWidgetOptions()
  onHeightChanged: updateWidgetOptions()
  function updateWidgetOptions() {
    plot.yLabelVisible = (root.width >= 196)
    plot.xLabelVisible = (root.height >= (196 * 2/3))
    root.hasToolbar = (root.width >= toolbar.implicitWidth) && (root.height >= 220)
  }

  //
  // Update curve at 24 Hz
  //
  Connections {
    target: Cpp_Misc_TimerEvents

    function onUiTimeout() {
      if (root.visible && root.model) {
        if (root.interpolate) {
          root.model.draw(upperSeries)

          if (root.showAreaUnderPlot) {
            lowerSeries.clear()
            lowerSeries.append(root.model.minX, root.model.minY)
            lowerSeries.append(root.model.maxX, root.model.minY)
          }
        }

        else
          root.model.draw(scatterSeries)
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
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      checked: root.interpolate
      icon.source: root.interpolate?
                     "qrc:/rcc/icons/dashboard-buttons/interpolate-on.svg" :
                     "qrc:/rcc/icons/dashboard-buttons/interpolate-off.svg"
      onClicked: {
        root.interpolate = !root.interpolate
        if (!root.interpolate)
          root.showAreaUnderPlot = false
      }
    }

    ToolButton {
      width: 24
      height: 24
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      enabled: root.interpolate
      opacity: enabled ? 1 : 0.5
      checked: root.showAreaUnderPlot
      icon.source: "qrc:/rcc/icons/dashboard-buttons/area.svg"
      onClicked: root.showAreaUnderPlot = !root.showAreaUnderPlot
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
      checked: plot.xLabelVisible
      onClicked: plot.xLabelVisible = !plot.xLabelVisible
      icon.source: "qrc:/rcc/icons/dashboard-buttons/x.svg"
    }

    ToolButton {
      width: 24
      height: 24
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      checked: plot.yLabelVisible
      onClicked: plot.yLabelVisible = !plot.yLabelVisible
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
      onClicked: model.running = !model.running
      icon.source: model.running?
                     "qrc:/rcc/icons/dashboard-buttons/pause.svg" :
                     "qrc:/rcc/icons/dashboard-buttons/resume.svg"
    }

    ToolButton {
      width: 24
      height: 24
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      opacity: enabled ? 1 : 0.5
      enabled: plot.xAxis.zoom !== 1 || plot.yAxis.zoom !== 1
      icon.source: "qrc:/rcc/icons/dashboard-buttons/return.svg"
      onClicked: {
        plot.xAxis.pan = 0
        plot.yAxis.pan = 0
        plot.xAxis.zoom = 1
        plot.yAxis.zoom = 1
      }
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
    yLabel: root.model.yLabel
    xLabel: root.model.xLabel
    mouseAreaEnabled: windowRoot.focused
    xAxis.tickInterval: root.model.xTickInterval
    yAxis.tickInterval: root.model.yTickInterval

    onZoomChanged: root.setDownsampleFactor()
    onWidthChanged: root.setDownsampleFactor()
    onHeightChanged: root.setDownsampleFactor()

    Connections {
      target: root.windowRoot
      function onFocusedChanged() {
        plot.mouseAreaEnabled = root.windowRoot.focused
      }
    }

    Component.onCompleted: {
      graph.addSeries(areaSeries)
      graph.addSeries(upperSeries)
      graph.addSeries(lowerSeries)
      graph.addSeries(scatterSeries)
    }

    ScatterSeries {
      id: scatterSeries
      visible: !root.interpolate
      pointDelegate: Rectangle {
        width: 2
        height: 2
        radius: 1
        color: root.color
      }
    }

    LineSeries {
      id: upperSeries
      width: 2
      visible: root.interpolate
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
