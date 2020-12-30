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
    id: root

    //
    // Window properties
    //
    spacing: -1
    gradient: true
    implicitWidth: 260
    visible: opacity > 0
    opacity: enabled ? 1 : 0
    icon.source: "qrc:/icons/chart.svg"
    implicitHeight: implicitWidth + 96
    backgroundColor: Qt.rgba(9 / 255, 9 / 255, 12 / 255, 1)

    //
    // Set the level color randomly from an array
    //
    property var colorPalette: [
        "#f94144",
        "#f3722c",
        "#f8961e",
        "#f9844a",
        "#f9c74f",
        "#90be6d",
        "#43aa8b",
        "#4d908e",
        "#577590",
        "#277da1"
    ]

    //
    // Colors
    //
    property color tankColor: Qt.rgba(230/255, 224/255, 178/255, 1)
    property color valueColor: Qt.rgba(81/255, 116/255, 151/255, 1)
    property color titleColor: Qt.rgba(142/255, 205/255, 157/255, 1)
    property color emptyColor: Qt.rgba(18 / 255, 18 / 255, 24 / 255, 1)
    property color levelColor:  {
        if (colorPalette.length > index)
            return colorPalette[index]

        return colorPalette[colorPalette.length % index]
    }

    //
    // Custom properties
    //
    property real level: 0
    property string units: ""
    property int datasetIndex: 0
    property real tankRadius: 20
    property real currentValue: 0
    property real minimumValue: 0
    property real maximumValue: 10

    //
    // Connections with widget manager
    //
    Connections {
        target: CppWidgets
        function onDataChanged() {
            root.updateValues()
        }
    }

    //
    // Updates the internal values of the bar widget
    //
    function updateValues() {
        if (CppWidgets.barDatasetCount() > root.datasetIndex) {
            root.minimumValue = CppWidgets.barMin(root.datasetIndex)
            root.maximumValue = CppWidgets.barMax(root.datasetIndex)
            root.currentValue = CppWidgets.bar(root.datasetIndex)
            root.title = CppWidgets.barDatasetAt(root.datasetIndex).title
            root.units = CppWidgets.barDatasetAt(root.datasetIndex).units

            if (root.maximumValue > root.minimumValue)  {
                var range = root.maximumValue - root.minimumValue
                var relat = root.currentValue - root.minimumValue
                root.level = relat / range
            }

            else
                root.level = 0

            if (root.level > 1)
                root.level = 1
        }

        else {
            root.title = ""
            root.units = ""
            root.currentValue = 0
            root.minimumValue = 0
            root.maximumValue = 0
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
            color: root.emptyColor
            Layout.fillHeight: true
            border.color: root.tankColor
            Layout.minimumWidth: root.width * 0.17
            Layout.maximumWidth: root.width * 0.17

            //
            // Level indicator
            //
            Rectangle {
                anchors.fill: parent
                anchors.topMargin: (1 - root.level) * tank.height

                border.width: 2
                color: root.levelColor
                border.color: root.tankColor
            }
        }

        //
        // Level text
        //
        ColumnLayout {
            spacing: app.spacing
            Layout.fillHeight: true

            Label {
                color: root.titleColor
                font.family: app.monoFont
                Layout.alignment: Qt.AlignHCenter
                text: root.maximumValue.toFixed(2) + " " + root.units
            }

            Rectangle {
                width: 2
                color: root.valueColor
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                font.bold: true
                font.pixelSize: 16
                color: root.titleColor
                font.family: app.monoFont
                Layout.alignment: Qt.AlignHCenter
                text: (root.currentValue > root.maximumValue ? root.maximumValue.toFixed(2) :
                                                               root.currentValue.toFixed(2)) + " " + root.units

                Rectangle {
                    border.width: 1
                    anchors.fill: parent
                    color: "transparent"
                    border.color: root.titleColor
                    anchors.margins: -app.spacing
                }
            }

            Rectangle {
                width: 2
                color: root.valueColor
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                color: root.titleColor
                font.family: app.monoFont
                Layout.alignment: Qt.AlignHCenter
                text: root.minimumValue.toFixed(2) + " " + root.units
            }
        }
    }
}
