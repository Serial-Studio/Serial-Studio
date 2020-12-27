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
    id: gyro

    //
    // Window properties
    //
    spacing: -1
    implicitWidth: 260
    visible: opacity > 0
    opacity: enabled ? 1 : 0
    icon.source: "qrc:/icons/chart.svg"
    implicitHeight: implicitWidth + 96
    borderColor: Qt.rgba(45/255, 96/255, 115/255, 1)
    backgroundColor: Qt.rgba(9 / 255, 9 / 255, 12 / 255, 1)

    //
    // Custom properties
    //
    property real xAngle: 0
    property real yAngle: 0
    property real zAngle: 0
    property int groupIndex: 0

    //
    // Colors
    //
    property color gyroColor: Qt.rgba(230/255, 224/255, 178/255, 1)

    //
    // Connections with widget manager
    //
    Connections {
        target: CppWidgets
        function onDataChanged() {
            gyro.updateValues()
        }
    }

    //
    // Updates the internal values of the bar widget
    //
    function updateValues() {
        if (CppWidgets.gyroGroupCount() > gyro.groupIndex) {
            gyro.xAngle = CppWidgets.gyroX(gyro.groupIndex)
            gyro.yAngle = CppWidgets.gyroY(gyro.groupIndex)
            gyro.zAngle = CppWidgets.gyroZ(gyro.groupIndex)
            gyro.title = CppWidgets.gyroGroupAt(gyro.groupIndex).title
        }

        else {
            gyro.title = ""
            gyro.xAngle = 0
            gyro.yAngle = 0
            gyro.zAngle = 0
        }
    }

    //
    // Animations
    //
    Behavior on xAngle {NumberAnimation{}}
    Behavior on yAngle {NumberAnimation{}}
    Behavior on zAngle {NumberAnimation{}}

    //
    // Artificial horizon
    //
    Rectangle {
        border.width: 2
        radius: width / 2
        anchors.centerIn: parent
        border.color: gyro.gyroColor
        color: Qt.rgba(81/255, 116/255, 151/255, 1)
        width: Math.min(gyro.width, gyro.height) * 0.7
        height: Math.min(gyro.width, gyro.height) * 0.7

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: app.spacing
        }
    }
}
