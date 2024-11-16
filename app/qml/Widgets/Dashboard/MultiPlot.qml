/*
 * Copyright (c) 2024 Alex Spataru <https://github.com/alex-spataru>
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
    Plot {
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
                  font: Cpp_Misc_CommonFonts.customMonoFont(0.8)
                  Layout.maximumWidth: 128 - 14 - 8
                }
              }
            }
          }
        }
      }
    }
  }
}
