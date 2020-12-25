/*
 * Copyright (c) 2020 Alex Spataru <https://github.com/alex-spataru>
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
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import Group 1.0
import Dataset 1.0

import "."

Window {
    id: accel

    //
    // Properties
    //
    spacing: -1
    showIcon: false
    implicitWidth: 240
    visible: opacity > 0
    opacity: enabled ? 1 : 0
    implicitHeight: implicitWidth + 32
    Behavior on opacity {NumberAnimation{}}
    borderColor: Qt.rgba(81/255, 116/255, 151/255, 1)

    //
    // Gauge object
    //
    Rectangle {
        id: gauge
        border.width: 2
        radius: width / 2
        anchors.fill: parent
        anchors.margins: app.spacing * 2
        color: Qt.rgba(18 / 255, 18 / 255, 24 / 255, 1)
        border.color: Qt.rgba(230/255, 224/255, 178/255, 1)
    }

    //
    // Custom properties
    //
    property int index: 0
    property real accX: 0
    property real accY: 0
    property real accZ: 0
    property real meanGForce: 0
    property real gConstant: 9.80665
    readonly property real indicatorWidth: 4
    property real max: Number.MIN_SAFE_INTEGER
    property real min: Number.MAX_SAFE_INTEGER
    property color indicatorColor: Qt.rgba(230/255, 224/255, 178/255, 1)

    //
    // Redraw numbers & indicator
    //
    onWidthChanged: numbersCanvas.requestPaint()
    onHeightChanged: numbersCanvas.requestPaint()
    onMeanGForceChanged: indicatorCanvas.requestPaint()

    //
    // Animations
    //
    Behavior on meanGForce {NumberAnimation{}}

    //
    // Connections with widget manager
    //
    Connections {
        target: CppWidgets
        function onDataChanged() {
            accel.calculateMeanGForce()
        }
    }

    //
    // Calculates the mean g force for all three axes using the pythagorean theorem
    //
    function calculateMeanGForce() {
        if (CppWidgets.accelerometerGroupCount() > accel.index) {
            accel.accX = CppWidgets.accelerometerX(accel.index)
            accel.accY = CppWidgets.accelerometerY(accel.index)
            accel.accZ = CppWidgets.accelerometerZ(accel.index)
            accel.title = CppWidgets.accelerometerGroupAt(accel.index).title
        }

        else {
            accel.accX = 0
            accel.accY = 0
            accel.accZ = 0
            accel.title = 0
        }

        var gX = accel.accX / accel.gConstant
        var gY = accel.accY / accel.gConstant
        var gZ = accel.accZ / accel.gConstant

        accel.meanGForce = Math.sqrt(Math.pow(gX, 2) + Math.pow(gY, 2) + Math.pow(gZ, 2))

        if (accel.meanGForce > accel.max)
            accel.max = accel.meanGForce
        else if (accel.meanGForce < accel.min)
            accel.min = accel.meanGForce
    }

    //
    // Units label
    //
    Label {
        text: qsTr("G Units")
        anchors.centerIn: parent
        font.family: app.monoFont
        anchors.verticalCenterOffset: -32
        color: Qt.rgba(81/255, 116/255, 151/255, 1)
    }

    //
    // Numbers painter
    //
    Canvas {
        opacity: 0.8
        id: numbersCanvas
        anchors.fill: parent
        anchors.margins: app.spacing * 2
        Component.onCompleted: requestPaint()
        onPaint: {
            var ctx = getContext('2d')

            for (var i = 0; i <= 7; ++i) {
                var d = -60
                var dd = (d + i * 45) * (Math.PI / 180)
                var tx = (gauge.width * 0.4) * Math.cos(dd) + gauge.width / 2 - 9/2
                var ty = (gauge.height * 0.4) * Math.sin(dd) + gauge.height / 2 + 9/2

                ctx.font = "bold 18px " + app.monoFont
                ctx.fillStyle = accel.indicatorColor
                ctx.fillText(i, tx, ty)
            }
        }
    }

    //
    // Indicator painter
    //
    Canvas {
        id: indicatorCanvas
        anchors.fill: parent
        anchors.margins: app.spacing * 2
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
                var deg = (Math.min(value, 7.5) / 8) * 360
                var rad = deg * (Math.PI / 180)
                var len = Math.min(gauge.width, gauge.height) * lenGain

                var x = gauge.width / 2
                var y = gauge.height / 2
                var x1 = x + Math.cos(rad) * len
                var y1 = y + Math.sin(rad) * len

                ctx.lineWidth = width
                ctx.strokeStyle = color
                drawLineWithArrows(x, y, x1, y1, width, width * 2)
            }

            ctx.reset()
            drawIndicator(accel.max, Qt.rgba(215/255, 45/255, 96/255, 1), accel.indicatorWidth * 0.8, 0.20)
            drawIndicator(accel.min, Qt.rgba(45/255, 96/255, 115/255, 1), accel.indicatorWidth * 0.8, 0.20)
            drawIndicator(accel.meanGForce, accel.indicatorColor, accel.indicatorWidth, 0.28)
        }
    }

    //
    // Central gauge
    //
    Rectangle {
        width: 24
        height: 24
        color: "#111"
        radius: width / 2
        anchors.centerIn: parent
        border.color: accel.indicatorColor
        border.width: accel.indicatorWidth - 1
    }
}
