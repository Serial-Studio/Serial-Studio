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
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import SerialStudio 1.0

import "."

Window {
    id: root

    //
    // Properties
    //
    spacing: -1
    gradient: true
    implicitWidth: 260
    visible: opacity > 0
    opacity: enabled ? 1 : 0
    icon.source: "qrc:/icons/tab.svg"
    implicitHeight: implicitWidth + 96
    backgroundColor: Cpp_ThemeManager.widgetBackground

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
    property bool firstReading: true
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
        target: Cpp_UI_WidgetProvider
        function onDataChanged() {
            root.calculateMeanGForce()
        }
    }

    //
    // Calculates the mean g force for all three axes using the pythagorean theorem
    //
    function calculateMeanGForce() {
        // Update values
        if (Cpp_UI_WidgetProvider.accelerometerGroupCount() > root.groupIndex) {
            root.accX  = Cpp_UI_WidgetProvider.accelerometerX(root.groupIndex)
            root.accY  = Cpp_UI_WidgetProvider.accelerometerY(root.groupIndex)
            root.accZ  = Cpp_UI_WidgetProvider.accelerometerZ(root.groupIndex)
            root.title = Cpp_UI_WidgetProvider.accelerometerGroupAt(root.groupIndex).title
        }

        // Invalid widget index, reset everything
        else {
            root.accX  = 0
            root.accY  = 0
            root.accZ  = 0
            root.title = 0
            root.firstReading = true
        }

        // Obtain g-forces for each axis
        var gX = root.accX / root.gConstant
        var gY = root.accY / root.gConstant
        var gZ = root.accZ / root.gConstant

        // Calculate mean force by squared normalization
        root.meanGForce = Math.sqrt(Math.pow(gX, 2) + Math.pow(gY, 2) + Math.pow(gZ, 2))

        // Update min/max values
        if (!firstReading) {
            if (root.meanGForce > root.max)
                root.max = root.meanGForce
            else if (root.meanGForce < root.min)
                root.min = root.meanGForce
        }

        // This is first reading, set min/max to current value
        else {
            // Update min/max and ensure that this is called only once
            if (root.meanGForce != 0) {
                root.max = root.meanGForce
                root.min = root.meanGForce
                root.firstReading = false
            }

            // Just to be sure re-calculate gauge size on first reading
            calculateGaugeSize()
        }
    }

    //
    // Redraws accelerator gauge
    //
    function calculateGaugeSize() {
        if (root.width < root.height)
            root.gaugeSize = Math.max(120, root.width - controls.implicitWidth - 12 * app.spacing) * 0.8

        else
            root.gaugeSize = Math.max(120, root.height - controls.implicitWidth - 4 * app.spacing) * 0.8
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
        AccelerometerGaugeDelegate {
            id: gauge
            lastNumber: 8
            firstNumber: 0
            minimumValue: root.min
            maximumValue: root.max
            valueLabelVisible: false
            currentValue: root.meanGForce
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: root.gaugeSize
            Layout.maximumWidth: root.gaugeSize
            Layout.minimumHeight: root.gaugeSize
            Layout.maximumHeight: root.gaugeSize
            title: qsTr("G Units")
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
                font.family: app.monoFont
                color: Cpp_ThemeManager.widgettextPrimary
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("%1 G MAX").arg(root.max.toFixed(2))
            }

            Label {
                font.pixelSize: 12
                font.family: app.monoFont
                color: Cpp_ThemeManager.widgettextPrimary
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("%1 G MIN").arg(root.min.toFixed(2))
            }

            Item {
                height: app.spacing
            }

            Label {
                font.bold: true
                font.pixelSize: 12
                font.family: app.monoFont
                color: Cpp_ThemeManager.widgettextPrimary
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("%1 G ACT").arg(root.meanGForce.toFixed(2))

                Rectangle {
                    border.width: 1
                    color: "transparent"
                    anchors.fill: parent
                    anchors.margins: -app.spacing
                    border.color: Cpp_ThemeManager.widgettextPrimary
                }
            }

            Item {
                height: app.spacing
            }

            Button {
                text: qsTr("Reset")
                Layout.alignment: Qt.AlignHCenter

                onClicked: {
                    root.max = 0
                    root.min = 0
                    root.firstReading = true
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
