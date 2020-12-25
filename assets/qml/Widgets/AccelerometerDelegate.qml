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
import QtQuick.Layouts 1.0

import Group 1.0
import Dataset 1.0

RowLayout {
    id: accel
    spacing: app.spacing

    //
    // Custom properties
    //
    property int index: 0
    property real accX: 0
    property real accY: 0
    property real accZ: 0
    property string title: ""
    property real meanGForce: 0
    property real gConstant: 9.80665

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
    }

    //
    // Accelerometer circle
    //
    Rectangle {
        width: height
        color: "#444"
        border.width: 2
        radius: width / 2
        border.color: "#bebebe"
        Layout.fillHeight: true
    }

    //
    // Indicator controls
    //
    ColumnLayout {
        spacing: app.spacing
        Layout.fillHeight: true

        Item {
            Layout.fillHeight: true
        }

        LED {
            onColor: "#f00"
            text: qsTr("Cont. value")
        }

        LED {
            onColor: "#0f0"
            text: qsTr("Max. value")
        }

        LED {
            onColor: "#00f"
            text: qsTr("Min. value")
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
