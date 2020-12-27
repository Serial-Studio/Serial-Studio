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
    implicitWidth: 260
    visible: opacity > 0
    opacity: enabled ? 1 : 0
    icon.source: "qrc:/icons/tab.svg"
    implicitHeight: implicitWidth + 96
    borderColor: Qt.rgba(81/255, 116/255, 151/255, 1)
    backgroundColor: Qt.rgba(9 / 255, 9 / 255, 12 / 255, 1)

    //
    // Custom properties
    //
    property real max: 0
    property real min: 0
    property int index: 0
    property real accX: 0
    property real accY: 0
    property real accZ: 0
    property real meanGForce: 0
    property real gConstant: 9.80665

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
    // Gauge & reset button
    //
    ColumnLayout {
        spacing: 0
        anchors.fill: parent
        anchors.margins: app.spacing * 2

        //
        // Spacer
        //
        Item {
            Layout.fillHeight: true
            Layout.minimumHeight: app.spacing * 2
        }

        //
        // Gauge object
        //
        GaugeDelegate {
            firstNumber: 0
            lastNumber: 8
            title: qsTr("G Units")
            minimumValue: accel.min
            maximumValue: accel.max
            currentValue: accel.meanGForce

            Layout.fillWidth: true
            Layout.minimumHeight: width
            Layout.maximumHeight: width
            Layout.alignment: Qt.AlignHCenter
        }

        //
        // Spacer
        //
        Item {
            Layout.fillHeight: true
            Layout.minimumHeight: app.spacing * 2
        }

        //
        // Reset button
        //
        Button {
            text: qsTr("Reset")
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                accel.max = 0
                accel.min = 0
            }
        }

        //
        // Spacer
        //
        Item {
            Layout.fillHeight: true
            Layout.minimumHeight: app.spacing * 2
        }
    }
}
