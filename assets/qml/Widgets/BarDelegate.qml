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
import "../Config/Colors.js" as Colors

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
    backgroundColor: Colors.WidgetBackground

    //
    // Select level color depending on widget index
    //
    property color levelColor:  {
        if (Colors.BarColors.length > index)
            return olors.BarColors[index]

        return Colors.BarColors[Colors.BarColors.length % index]
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
    // Animations
    //
    Behavior on currentValue {NumberAnimation{duration: 200}}

    //
    // Connections with widget manager
    //
    Connections {
        target: Cpp_UI_WidgetProvider

        //*! Optimize this function
        function onDataChanged() {
            root.updateValues()
        }
    }

    //
    // Updates the internal values of the bar widget
    //
    //*! Optimize this function
    //   About 6% of the UI thread is spent here
    function updateValues() {
        if (Cpp_UI_WidgetProvider.barDatasetCount() > root.datasetIndex) {
            root.minimumValue = Cpp_UI_WidgetProvider.barMin(root.datasetIndex)
            root.maximumValue = Cpp_UI_WidgetProvider.barMax(root.datasetIndex)
            root.currentValue = Cpp_UI_WidgetProvider.bar(root.datasetIndex)
            root.title = Cpp_UI_WidgetProvider.barDatasetAt(root.datasetIndex).title
            root.units = Cpp_UI_WidgetProvider.barDatasetAt(root.datasetIndex).units

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
            root.level = 0
            root.title = ""
            root.units = ""
            root.currentValue = 0
            root.minimumValue = 0
            root.maximumValue = 0
        }
    }

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
            Layout.fillHeight: true
            color: Colors.BarTankBackground
            Layout.minimumWidth: root.width * 0.17
            Layout.maximumWidth: root.width * 0.17
            border.color: Colors.WidgetIndicatorColor

            //
            // Level indicator
            //
            Rectangle {
                anchors.fill: parent
                anchors.topMargin: (1 - root.level) * tank.height

                border.width: 2
                color: root.levelColor
                border.color: Colors.WidgetIndicatorColor
            }
        }

        //
        // Level text
        //
        ColumnLayout {
            spacing: app.spacing
            Layout.fillHeight: true

            Label {
                font.family: app.monoFont
                color: Colors.WidgetValueColor
                Layout.alignment: Qt.AlignHCenter
                text: root.maximumValue.toFixed(2) + " " + root.units
            }

            Rectangle {
                width: 2
                Layout.fillHeight: true
                color: Colors.WidgetTitleColor
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                font.bold: true
                font.pixelSize: 16
                font.family: app.monoFont
                color: Colors.WidgetValueColor
                Layout.alignment: Qt.AlignHCenter
                text: (root.currentValue > root.maximumValue ? root.maximumValue.toFixed(2) :
                                                               root.currentValue.toFixed(2)) + " " + root.units

                Rectangle {
                    border.width: 1
                    anchors.fill: parent
                    color: "transparent"
                    anchors.margins: -app.spacing
                    border.color: Colors.WidgetValueColor
                }
            }

            Rectangle {
                width: 2
                Layout.fillHeight: true
                color: Colors.WidgetTitleColor
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                font.family: app.monoFont
                color: Colors.WidgetValueColor
                Layout.alignment: Qt.AlignHCenter
                text: root.minimumValue.toFixed(2) + " " + root.units
            }
        }
    }
}
