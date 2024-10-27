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

import "../"

Item {
  id: root

  //
  // Widget data inputs
  //
  property color color: Cpp_ThemeManager.colors["highlight"]
  property PlotModel model: PlotModel{}

  //
  // Update curve at 24 Hz
  //
  Timer {
    repeat: true
    interval: 1000 / 24
    running: root.visible
    onTriggered: root.model.draw(lineSeries)
  }

  //
  // Plot widget
  //
  Plot {
    id: plot
    anchors.margins: 8
    anchors.fill: parent
    xMin: root.model.minX
    xMax: root.model.maxX
    yMin: root.model.minY
    yMax: root.model.maxY
    xLabel: qsTr("Samples")
    curveColors: [root.color]
    yLabel: root.model.yLabel
    xAxis.tickInterval: root.model.xTickInterval
    yAxis.tickInterval: root.model.yTickInterval
    Component.onCompleted: graph.addSeries(lineSeries)

    //
    // Curve element
    //
    LineSeries {
      id: lineSeries
    }
  }
}
