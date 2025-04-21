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
    onTriggered: {
      root.model.draw(upperSeries)
      lowerSeries.clear()
      lowerSeries.append(root.model.minX, root.model.minY)
      lowerSeries.append(root.model.maxX, root.model.minY)
    }
  }

  //
  // Plot widget
  //
  PlotWidget {
    id: plot
    anchors.margins: 8
    anchors.fill: parent
    xMin: root.model.minX
    xMax: root.model.maxX
    yMin: root.model.minY
    yMax: root.model.maxY
    curveColors: [root.color]
    yLabel: root.model.yLabel
    xLabel: root.model.xLabel
    xAxis.tickInterval: root.model.xTickInterval
    yAxis.tickInterval: root.model.yTickInterval
    showCrosshairs: Cpp_UI_Dashboard.showCrosshairs

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
      visible: Cpp_UI_Dashboard.showAreaUnderPlots
      color: Qt.rgba(root.color.r, root.color.g, root.color.b, 0.2)
    }
  }
}
