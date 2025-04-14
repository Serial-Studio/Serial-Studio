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
  property color color: Cpp_ThemeManager.colors["highlight"]
  property MultiPlotModel model: MultiPlotModel{}

  //
  // Update curves at 24 Hz
  //
  Timer {
    repeat: true
    interval: 1000 / 24
    running: root.visible
    onTriggered: {
      const count = plot.graph.seriesList.length
      for (let i = 0; i < count; ++i)
        root.model.draw(plot.graph.seriesList[i], i)
    }
  }


  RowLayout {
    spacing: 4
    anchors.margins: 8
    anchors.fill: parent

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
      xAxis.tickInterval: root.model.xTickInterval
      yAxis.tickInterval: root.model.yTickInterval

      //
      // Register curves
      //
      Instantiator {
        model: root.model.count
        delegate: LineSeries {
          Component.onCompleted: plot.graph.addSeries(this)
        }
      }
    }

    //
    // Legends widget
    //
    Item {
      Layout.fillHeight: true
      visible: Cpp_UI_Dashboard.showLegends
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
