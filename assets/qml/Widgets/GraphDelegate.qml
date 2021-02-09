/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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

import QtQuick 2.12
import QtCharts 2.3

import SerialStudio 1.0

Window {
    id: root

    property int graphId: -1
    property real maximumValue: -Infinity
    property real minimumValue: +Infinity

    spacing: -1
    showIcon: false
    visible: opacity > 0
    opacity: enabled ? 1 : 0
    borderColor: root.headerVisible ? "#517497" : "transparent"

    title: Cpp_UI_GraphProvider.getDataset(graphId).title +
           " (" + Cpp_UI_GraphProvider.getDataset(graphId).units + ")"

    Connections {
        target: Cpp_UI_GraphProvider
        function onDataUpdated() {
            // Cancel if window is not enabled
            if (!root.enabled)
                return

            // Update maximum value (if required)
            maximumValue = Cpp_UI_GraphProvider.maximumValue(graphId)
            minimumValue = Cpp_UI_GraphProvider.minimumValue(graphId)

            // Get central value
            var medianValue = Math.max(1, (maximumValue + minimumValue)) / 2
            if (maximumValue == minimumValue)
                medianValue = maximumValue

            // Center graph verticaly
            var mostDiff = Math.max(Math.abs(minimumValue), Math.abs(maximumValue))
            var min = medianValue * (1 - 0.5) - Math.abs(medianValue - mostDiff)
            var max = medianValue * (1 + 0.5) + Math.abs(medianValue - mostDiff)
            if (minimumValue < 0)
                min = max * -1

            // Fix issues when min & max are equal
            if (min === max) {
                max = Math.abs(max)
                min = max * -1
            }

            // Fix issues on min = max = (0,0)
            if (min === 0 && max === 0) {
                max = 1
                min = -1
            }

            // Update axes only if needed
            if (positionAxis.min !== min)
                positionAxis.min = min
            if (positionAxis.max !== max)
                positionAxis.max = max

            // Draw graph
            Cpp_UI_GraphProvider.updateGraph(series, graphId)
        }
    }

    ChartView {
        antialiasing: true
        anchors.fill: parent
        legend.visible: false
        backgroundRoundness: 0
        enabled: root.enabled
        visible: root.enabled
        backgroundColor: root.backgroundColor

        margins {
            top: 0
            bottom: 0
            left: 0
            right: 0
        }

        ValueAxis {
            id: timeAxis
            min: 0
            labelFormat: " "
            lineVisible: false
            labelsVisible: false
            gridLineColor: "#517497"
            tickType: ValueAxis.TicksFixed
            labelsFont.family: app.monoFont
            max: Cpp_UI_GraphProvider.displayedPoints
        }

        ValueAxis {
            id: positionAxis
            min: 0
            max: 1
            lineVisible: false
            labelsColor: "#517497"
            gridLineColor: "#517497"
            tickType: ValueAxis.TicksFixed
            labelsFont.family: app.monoFont
        }

        LineSeries {
            id: series
            width: 2
            axisX: timeAxis
            useOpenGL: true
            color: "#e6e0b2"
            capStyle: Qt.RoundCap
            axisYRight: positionAxis
        }
    }
}
