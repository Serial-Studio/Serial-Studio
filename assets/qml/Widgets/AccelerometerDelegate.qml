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
    borderColor: Qt.rgba(45/255, 96/255, 115/255, 1)
    backgroundColor: Qt.rgba(9 / 255, 9 / 255, 12 / 255, 1)

    //
    // Custom properties
    //
    property real max: 0
    property real min: 0
    property real accX: 0
    property real accY: 0
    property real accZ: 0
    property int groupIndex: 0
    property real meanGForce: 0
    property real gConstant: 9.80665
    property real gaugeSize: calculateGaugeSize()

    //
    // Update gauge size automatically
    //
    onWidthChanged: calculateGaugeSize()
    onHeightChanged: calculateGaugeSize()

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
        if (CppWidgets.accelerometerGroupCount() > accel.groupIndex) {
            accel.accX = CppWidgets.accelerometerX(accel.groupIndex)
            accel.accY = CppWidgets.accelerometerY(accel.groupIndex)
            accel.accZ = CppWidgets.accelerometerZ(accel.groupIndex)
            accel.title = CppWidgets.accelerometerGroupAt(accel.groupIndex).title
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
    // Redraws accelerator gauge
    //
    function calculateGaugeSize() {
        if (accel.width < accel.height)
            accel.gaugeSize = Math.max(140, accel.width - controls.implicitWidth - 12 * app.spacing)

        else
            accel.gaugeSize = Math.max(140, accel.height - controls.implicitWidth - 2 * app.spacing)
    }

    //
    // Gauge & reset button
    //
    RowLayout {
        spacing: 0
        anchors.fill: parent
        anchors.margins: app.spacing

        //
        // Spacer
        //
        Item {
            Layout.fillWidth: true
        }

        //
        // Gauge object
        //
        GaugeDelegate {
            id: gauge
            lastNumber: 8
            firstNumber: 0
            title: qsTr("G Units")
            minimumValue: accel.min
            maximumValue: accel.max
            valueLabelVisible: false
            currentValue: accel.meanGForce
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: accel.gaugeSize
            Layout.maximumWidth: accel.gaugeSize
            Layout.minimumHeight: accel.gaugeSize
            Layout.maximumHeight: accel.gaugeSize
        }

        //
        // Spacer
        //
        Item {
            Layout.fillWidth: true
        }

        //
        // Reset button & values
        //
        ColumnLayout {
            id: controls
            spacing: app.spacing
            Layout.alignment: Qt.AlignVCenter

            Label {
                font.pixelSize: 12
                color: gauge.valueColor
                font.family: app.monoFont
                text: qsTr("Maximum: %1 G").arg(accel.max.toFixed(2))
            }

            Label {
                font.pixelSize: 12
                color: gauge.valueColor
                font.family: app.monoFont
                text: qsTr("Minimum: %1 G").arg(accel.min.toFixed(2))
            }

            Item {
                height: app.spacing
            }

            Label {
                font.bold: true
                font.pixelSize: 12
                color: gauge.valueColor
                font.family: app.monoFont
                text: qsTr("Current: %1 G").arg(accel.meanGForce.toFixed(2))

                Rectangle {
                    border.width: 1
                    color: "transparent"
                    anchors.fill: parent
                    anchors.margins: -app.spacing
                    border.color: gauge.valueColor
                }
            }

            Item {
                height: app.spacing
            }

            Button {
                text: qsTr("Reset")
                onClicked: {
                    accel.max = 0
                    accel.min = 0
                }
            }
        }

        //
        // Spacer
        //
        Item {
            Layout.fillWidth: true
        }
    }
}
