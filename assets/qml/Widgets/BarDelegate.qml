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
    id: bar

    //
    // Window properties
    //
    spacing: -1
    implicitWidth: 260
    visible: opacity > 0
    opacity: enabled ? 1 : 0
    icon.source: "qrc:/icons/chart.svg"
    implicitHeight: implicitWidth + 96
    borderColor: Qt.rgba(81/255, 116/255, 151/255, 1)
    backgroundColor: Qt.rgba(9 / 255, 9 / 255, 12 / 255, 1)

    //
    // Colors
    //
    property color levelColor: Qt.rgba(215/255, 45/255, 96/255, 1)
    property color tankColor: Qt.rgba(230/255, 224/255, 178/255, 1)
    property color valueColor: Qt.rgba(81/255, 116/255, 151/255, 1)
    property color titleColor: Qt.rgba(142/255, 205/255, 157/255, 1)
    property color emptyColor: Qt.rgba(18 / 255, 18 / 255, 24 / 255, 1)

    //
    // Custom properties
    //
    property string units: ""
    property int datasetIndex: 0
    property real tankRadius: 20
    property real currentValue: 0
    property real minimumValue: 0
    property real maximumValue: 10
    readonly property real level: Math.min(1, currentValue / Math.max(1, (maximumValue - minimumValue)))

    //
    // Connections with widget manager
    //
    Connections {
        target: CppWidgets
        function onDataChanged() {
            bar.updateValues()
        }
    }

    //
    // Updates the internal values of the bar widget
    //
    function updateValues() {
        if (CppWidgets.barDatasetCount() > bar.datasetIndex) {
            bar.currentValue = CppWidgets.bar(bar.datasetIndex)
            bar.minimumValue = CppWidgets.barMin(bar.datasetIndex)
            bar.maximumValue = CppWidgets.barMax(bar.datasetIndex)
            bar.title = CppWidgets.barDatasetAt(bar.datasetIndex).title
            bar.units = CppWidgets.barDatasetAt(bar.datasetIndex).units
        }

        else {
            bar.title = ""
            bar.units = ""
            bar.currentValue = 0
            bar.minimumValue = 0
            bar.maximumValue = 0
        }
    }

    //
    // Animations
    //
    Behavior on currentValue {NumberAnimation{}}

    //
    // Layout
    //
    RowLayout {
        spacing: app.spacing * 4

        anchors {
            fill: parent
            margins: app.spacing * 4
            leftMargin: app.spacing * 8
            rightMargin: app.spacing * 8
        }

        //
        // Bar/Tank rectangle
        //
        Rectangle {
            id: tank
            border.width: 2
            color: bar.emptyColor
            Layout.fillWidth: true
            Layout.fillHeight: true
            border.color: bar.tankColor

            //
            // Level indicator
            //
            Rectangle {
                anchors.fill: parent
                anchors.topMargin: (1 - bar.level) * tank.height

                border.width: 2
                color: bar.levelColor
                border.color: bar.tankColor
            }
        }

        //
        // Level text
        //
        ColumnLayout {
            spacing: app.spacing
            Layout.fillHeight: true

            Label {
                color: bar.titleColor
                font.family: app.monoFont
                Layout.alignment: Qt.AlignHCenter
                text: bar.maximumValue.toFixed(2) + " " + bar.units
            }

            Rectangle {
                width: 2
                color: bar.valueColor
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                font.bold: true
                font.pixelSize: 16
                color: bar.titleColor
                font.family: app.monoFont
                Layout.alignment: Qt.AlignHCenter
                text: (bar.currentValue > bar.maximumValue ? bar.maximumValue.toFixed(2) :
                                                             bar.currentValue.toFixed(2)) + " " + bar.units

                Rectangle {
                    border.width: 1
                    anchors.fill: parent
                    color: "transparent"
                    border.color: bar.titleColor
                    anchors.margins: -app.spacing
                }
            }

            Rectangle {
                width: 2
                color: bar.valueColor
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                color: bar.titleColor
                font.family: app.monoFont
                Layout.alignment: Qt.AlignHCenter
                text: bar.minimumValue.toFixed(2) + " " + bar.units
            }
        }
    }
}
