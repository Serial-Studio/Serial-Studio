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
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtGraphs
import SerialStudio
import QtQuick.Layouts
import QtQuick.Controls

import "../"

Item {
  id: root

  //
  // Widget data inputs
  //
  required property color color
  required property MultiPlotModel model
  required property MiniWindow windowRoot

  //
  // Window flags
  //
  property bool hasToolbar: true

  //
  // Custom properties
  //
  property bool running: true
  property bool interpolate: true
  property bool showLegends: true

  //
  // Enable/disable features depending on window size
  //
  onWidthChanged: updateWidgetOptions()
  onHeightChanged: updateWidgetOptions()
  function updateWidgetOptions() {
    root.showLegends = (root.width >= 256)
    plot.yLabelVisible = (root.width >= 196)
    plot.xLabelVisible = (root.height >= (196 * 2/3))
    root.hasToolbar = (root.width >= toolbar.implicitWidth)
  }

  //
  // Update widget at 24 Hz
  //
  Connections {
    target: Cpp_Misc_TimerEvents

    function onTimeout24Hz() {
      if (root.visible && root.running) {
        const count = plot.graph.seriesList.length
        for (let i = 0; i < count; ++i) {
          let ptr = plot.graph.seriesList[i]

          if (ptr.visible)
            root.model.draw(ptr, ptr.objectName)
          else
            ptr.clear()
        }
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
      checked: !root.running
      icon.color: "transparent"
      onClicked: root.running = !root.running
      icon.source: root.running?
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

      //
      // Register line series
      //
      Instantiator {
        model: root.model.count
        delegate: LineSeries {
          objectName: index
          visible: root.interpolate
          Component.onCompleted: plot.graph.addSeries(this)
        }
      }

      //
      // Register scatter series
      //
      Instantiator {
        model: root.model.count
        delegate: ScatterSeries {
          objectName: index
          visible: !root.interpolate
          Component.onCompleted: plot.graph.addSeries(this)
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
              delegate: RowLayout {
                id: _label
                spacing: 4
                Layout.fillWidth: true

                Rectangle {
                  width: 14
                  height: 14
                  color: root.model.colors[index]
                  Layout.alignment: Qt.AlignVCenter
                }

                Label {
                  elide: Qt.ElideMiddle
                  text: root.model.labels[index]
                  Layout.alignment: Qt.AlignVCenter
                  Layout.maximumWidth: 128 - 14 - 8
                  color: Cpp_ThemeManager.colors["widget_text"]
                  font: Cpp_Misc_CommonFonts.customMonoFont(0.8)
                }
              }
            }
          }
        }
      }
    }
  }
}
