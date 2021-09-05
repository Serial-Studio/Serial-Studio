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
    // Window properties
    //
    spacing: -1
    gradient: true
    implicitWidth: 260
    visible: opacity > 0
    opacity: enabled ? 1 : 0
    implicitHeight: implicitWidth + 96
    icon.source: "qrc:/icons/compass.svg"
    backgroundColor: Cpp_ThemeManager.widgetBackground

    //
    // Custom properties
    //
    property int angle: 0
    property int datasetIndex: 0
    property string direction: "0 N"
    property int widgetSize: Math.min(root.height, root.width) * 0.6

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
    // Updates the internal values of the compass widget
    //
    function updateValues() {
        if (Cpp_UI_WidgetProvider.compassDatasetCount() > root.datasetIndex) {
            root.angle = Cpp_UI_WidgetProvider.compass(root.datasetIndex)
            root.title = Cpp_UI_WidgetProvider.compassDatasetAt(root.datasetIndex).title

            if (root.angle <= 22)
                root.direction = root.angle + "° N"
            else if (root.angle > 22 && root.angle <= 67)
                root.direction = root.angle + "° NE"
            else if (root.angle > 67 && root.angle <= 112)
                root.direction = root.angle + "° E"
            else if (root.angle > 112 && root.angle <= 157)
                root.direction = root.angle + "° SE"
            else if (root.angle > 157 && root.angle <= 203)
                root.direction = root.angle + "° S"
            else if (root.angle > 203 && root.angle <= 247)
                root.direction = root.angle + "° SW"
            else if (root.angle > 247 && root.angle <= 292)
                root.direction = root.angle + "° W"
            else if (root.angle > 292 && root.angle <= 337)
                root.direction = root.angle + "° NW"
            else
                root.direction = root.angle + "° N"
        }

        else {
            root.angle = 0
            root.title = ""
            root.direction = "0° N"
        }
    }

    //
    // Compass image
    //
    ColumnLayout {
        anchors.centerIn: parent
        spacing: app.spacing * 2

        Item {
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: root.widgetSize
            Layout.maximumWidth: root.widgetSize
            Layout.minimumHeight: root.widgetSize
            Layout.maximumHeight: root.widgetSize

            Image {
                anchors.fill: parent
                rotation: 360 - root.angle
                source: "qrc:/instruments/Compass.svg"
                sourceSize: Qt.size(root.widgetSize, root.widgetSize)
            }

            Image {
                anchors.fill: parent
                source: "qrc:/instruments/CompassCase.svg"
                sourceSize: Qt.size(root.widgetSize, root.widgetSize)
            }
        }

        Label {
            font.bold: true
            font.pixelSize: 24
            text: root.direction
            font.family: app.monoFont
            color: Cpp_ThemeManager.widgettextPrimary
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
