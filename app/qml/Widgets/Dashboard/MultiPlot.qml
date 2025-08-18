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

  //
  // Widget data inputs
  //
  required property color color
  required property MultiPlotModel model
  required property var windowRoot

  //
  // Window flags
  //
  property bool hasToolbar: true

  //
  // Custom properties
  //
  property bool interpolate: true
  property bool showLegends: true

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
    category: "MultiPlot"
    property alias interpolateEnabled: root.interpolate
  }

  //
  // Enable/disable features depending on window size
  //
  onWidthChanged: updateWidgetOptions()
  onHeightChanged: updateWidgetOptions()
  function updateWidgetOptions() {
    root.showLegends = (root.width >= 320)
    plot.yLabelVisible = (root.width >= 196)
    plot.xLabelVisible = (root.height >= (196 * 2/3))
    root.hasToolbar = (root.width >= toolbar.implicitWidth) && (root.height >= 220)
  }

  //
  // Update widget at 24 Hz
  //
  Connections {
    target: Cpp_Misc_TimerEvents

    function onUiTimeout() {
      if (root.visible) {
        root.model.updateData()

        const count = plot.graph.seriesList.length
        for (let i = 0; i < count; ++i) {
          let ptr = plot.graph.seriesList[i]
          if (ptr.visible)
            root.model.draw(ptr, ptr.curveIndex)
          else
            ptr.clear()
        }
      }
    }
  }

  //
  // Re-draw whole plot when curves changes
  //
  Connections {
    target: root.model

    function onCurvesChanged() {
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
      onClicked: root.interpolate = !root.interpolate
      icon.source: root.interpolate?
                     "qrc:/rcc/icons/dashboard-buttons/interpolate-on.svg" :
                     "qrc:/rcc/icons/dashboard-buttons/interpolate-off.svg"
    }

    ToolButton {
      width: 24
      height: 24
      icon.width: 18
      icon.height: 18
      icon.color: "transparent"
      checked: root.showLegends
      onClicked: root.showLegends = !root.showLegends
      icon.source: "qrc:/rcc/icons/dashboard-buttons/labels.svg"
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
  // Plot + Legends
  //
  RowLayout {
    spacing: 4

    anchors {
      margins: 8
      left: parent.left
      right: parent.right
      top: toolbar.bottom
      bottom: parent.bottom
    }

    //
    // Plot widget
    //
    PlotWidget {
      id: plot
      xMin: root.model.minX
      xMax: root.model.maxX
      yMin: root.model.minY
      yMax: root.model.maxY
      xLabel: qsTr("Samples")
      Layout.fillWidth: true
      Layout.fillHeight: true
      yLabel: root.model.yLabel
      curveColors: root.model.colors
      mouseAreaEnabled: windowRoot.focused
      xAxis.tickInterval: root.model.xTickInterval
      yAxis.tickInterval: root.model.yTickInterval

      onZoomChanged: root.setDownsampleFactor()
      onWidthChanged: root.setDownsampleFactor()
      onHeightChanged: root.setDownsampleFactor()

      //
      // Register line series
      //
      Instantiator {
        model: root.model.count
        delegate: LineSeries {
          property int curveIndex: index
          Component.onCompleted: plot.graph.addSeries(this)
          visible: root.interpolate && root.model.visibleCurves[index]
        }
      }

      //
      // Register scatter series
      //
      Instantiator {
        model: root.model.count
        delegate: ScatterSeries {
          property int curveIndex: index
          Component.onCompleted: plot.graph.addSeries(this)
          visible: !root.interpolate && root.model.visibleCurves[index]
          pointDelegate: Rectangle {
            width: 2
            height: 2
            radius: 1
            color: root.model.colors[index]
          }
        }
      }
    }

    //
    // Legends widget
    //
    Item {
      opacity: plot.opacity
      Layout.fillHeight: true
      visible: root.showLegends
      implicitWidth: _legends.implicitWidth + 8

      Rectangle {
        width: parent.width
        anchors.centerIn: parent
        height: Math.min(_legends.implicitHeight + 8, parent.height)

        border.width: 1
        color: Cpp_ThemeManager.colors["widget_base"]
        border.color: Cpp_ThemeManager.colors["widget_border"]

        Flickable {
          clip: true
          anchors.margins: 4
          contentWidth: width
          anchors.fill: parent
          contentHeight: _legends.implicitHeight

          ScrollBar.vertical: ScrollBar {
            id: scroll
          }

          ColumnLayout {
            id: _legends
            spacing: 4
            width: parent.width

            Repeater {
              model: root.model.count
              delegate: Switch {
                Layout.fillWidth: true
                text: root.model.labels[index]
                Layout.alignment: Qt.AlignVCenter
                checked: root.model.visibleCurves[index]
                palette.highlight: root.model.colors[index]
                font: Cpp_Misc_CommonFonts.customMonoFont(0.8)
                palette.text: Cpp_ThemeManager.colors["widget_text"]
                onCheckedChanged: {
                  if (checked !== root.model.visibleCurves[index])
                    root.model.modifyCurveVisibility(index, checked)
                }
              }
            }
          }
        }
      }
    }
  }
}
