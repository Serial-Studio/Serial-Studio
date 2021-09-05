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
import QtQuick.Controls 2.12

Rectangle {
    id: root

    //
    // Custom properties
    //
    property string title: ""
    property int lastNumber: 8
    property int firstNumber: 0
    property real numberSize: 16
    property real currentValue: 0
    property real maximumValue: 0
    property real minimumValue: 0
    property real indicatorWidth: 4
    property bool maximumVisible: true
    property bool minimumVisible: true
    property bool currentVisible: true
    property bool numbersVisible: true
    property bool valueLabelVisible: true

    //
    // Redraw indicators automatically
    //
    onCurrentValueChanged: indicatorCanvas.requestPaint()

    //
    // Properties
    //
    border.width: 2
    radius: width / 2
    border.color: Cpp_ThemeManager.widgetIndicator1
    color: Cpp_ThemeManager.widgetAlternativeBackground

    //
    // Redraw numbers automatically
    //
    onWidthChanged: numbersCanvas.requestPaint()
    onHeightChanged: numbersCanvas.requestPaint()

    //
    // Label
    //
    Label {
        font.pixelSize: 14
        text: root.title
        anchors.centerIn: parent
        font.family: app.monoFont
        color: Cpp_ThemeManager.widgetForegroundSecondary
        anchors.verticalCenterOffset: 32
    }

    //
    // Value label
    //
    Label {
        font.pixelSize: 14
        anchors.centerIn: parent
        font.family: app.monoFont
        color: Cpp_ThemeManager.widgetForegroundPrimary
        visible: root.valueLabelVisible
        anchors.verticalCenterOffset: 56
        text: root.currentValue.toFixed(3)
    }

    //
    // Numbers painter
    //
    Canvas {
        opacity: 0.8
        id: numbersCanvas
        anchors.fill: parent
        Component.onCompleted: requestPaint()
        onPaint: {
            var ctx = getContext('2d')
            ctx.reset()

            if (!root.numbersVisible)
                return

            var range = lastNumber - firstNumber
            for (var i = 0; i <= range; ++i) {
                var radius = Math.min(root.width, root.height) / 2

                var startupTheta = -180
                var theta = (startupTheta + i * 360 / (range + 1)) * (Math.PI / 180)
                var dX = (radius - root.numberSize) * Math.cos(theta) + radius - root.numberSize / 2
                var dY = (radius - root.numberSize) * Math.sin(theta) + radius + root.numberSize / 2

                ctx.font = "bold " + root.numberSize + "px " + app.monoFont
                ctx.fillStyle = Cpp_ThemeManager.widgetIndicator1
                ctx.fillText(i + firstNumber, dX, dY)

                if (i === range) {
                    var x = radius
                    var y = radius
                    var spacing = 10 * Math.PI / 180.0;
                    var startAngle = theta + spacing
                    var finishAngle = Math.PI - spacing

                    ctx.lineWidth = 2
                    ctx.strokeStyle = Cpp_ThemeManager.widgetIndicator1

                    ctx.beginPath();
                    ctx.arc(x, y, radius - (root.numberSize + 3), startAngle, finishAngle)
                    ctx.stroke()
                    ctx.beginPath();
                    ctx.arc(x, y, radius - (root.numberSize - 3), startAngle, finishAngle)
                    ctx.stroke()
                }
            }
        }
    }

    //
    // Indicator painter
    //
    Canvas {
        id: indicatorCanvas
        anchors.fill: parent
        Component.onCompleted: requestPaint()
        onPaint: {
            var ctx = getContext('2d')

            function drawLineWithArrows(x0,y0,x1,y1,aWidth,aLength){
                var dx=x1-x0;
                var dy=y1-y0;
                var angle=Math.atan2(dy,dx);
                var length=Math.sqrt(dx*dx+dy*dy);

                ctx.translate(x0,y0);
                ctx.rotate(angle);
                ctx.beginPath();
                ctx.moveTo(0,0);
                ctx.lineTo(length,0);

                ctx.moveTo(length-aLength,-aWidth);
                ctx.lineTo(length,0);
                ctx.lineTo(length-aLength,aWidth);

                ctx.stroke();
                ctx.setTransform(1,0,0,1,0,0);
            }

            function drawIndicator(value, color, width, lenGain) {
                var deg = ((Math.min(value, root.lastNumber) / (root.lastNumber + 1)) * 360) - 180
                var rad = deg * (Math.PI / 180)
                var len = Math.min(root.width, root.height) * lenGain

                var x = root.width / 2
                var y = root.height / 2
                var x1 = x + Math.cos(rad) * len
                var y1 = y + Math.sin(rad) * len

                ctx.lineWidth = width
                ctx.strokeStyle = color
                drawLineWithArrows(x, y, x1, y1, width, width * 2)
            }

            ctx.reset()

            if (root.maximumVisible)
                drawIndicator(root.maximumValue,
                              Cpp_ThemeManager.widgetIndicator2,
                              root.indicatorWidth * 0.8, 0.20)

            if (root.minimumVisible)
                drawIndicator(root.minimumValue,
                              Cpp_ThemeManager.widgetIndicator3,
                              root.indicatorWidth * 0.8, 0.20)

            if (root.currentVisible)
                drawIndicator(root.currentValue,
                              Cpp_ThemeManager.widgetIndicator1,
                              root.indicatorWidth, 0.28)
        }
    }

    //
    // Central gauge
    //
    Rectangle {
        width: 24
        height: 24
        radius: width / 2
        anchors.centerIn: parent
        color: Cpp_ThemeManager.widgetControlBackground
        border.width: root.indicatorWidth - 1
        border.color: Cpp_ThemeManager.widgetIndicator1
    }
}
