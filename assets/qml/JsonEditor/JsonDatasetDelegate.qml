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

import "../Widgets" as Widgets

Widgets.Window {
    id: root
    Layout.minimumHeight: height
    icon.source: "qrc:/icons/dataset.svg"
    title: qsTr("Dataset %1").arg(datasetIndex + 1)
    borderColor: Cpp_ThemeManager.datasetWindowBorder
    palette.window: Cpp_ThemeManager.datasetWindowBackground
    height: grid.implicitHeight + headerHeight + 4 * app.spacing

    property int datasetIndex

    GridLayout {
        id: grid
        columns: 2
        rowSpacing: app.spacing
        columnSpacing: app.spacing

        anchors {
            left: parent.left
            right: parent.right
            margins: 2 * app.spacing
            verticalCenter: parent.verticalCenter
        }

        Label {
            text: qsTr("Title:")
        }

        TextField {
            Layout.fillWidth: true
            placeholderText: qsTr("Sensor reading, uptime, etc...")
        }

        Label {
            text: qsTr("Units:")
        }

        TextField {
            Layout.fillWidth: true
            placeholderText: qsTr("Volts, meters, seconds, etc...")
        }

        Label {
            text: qsTr("Frame index:")
        }

        SpinBox {
            from: 0
            editable: true
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("Widget:")
        }

        ComboBox {
            id: widget
            Layout.fillWidth: true
            model: [
                qsTr("None"),
                qsTr("Bar/level"),
                qsTr("Gauge"),
                qsTr("Compass")
            ]
        }

        Label {
            text: qsTr("Min value:")
            visible: widget.currentIndex == 1 || widget.currentIndex == 2
        }

        SpinBox {
            id: min
            from: 0
            value: 0
            to: max.value
            editable: true
            Layout.fillWidth: true
            visible: widget.currentIndex == 1 || widget.currentIndex == 2
        }

        Label {
            text: qsTr("Max value:")
            visible: widget.currentIndex == 1 || widget.currentIndex == 2
        }

        SpinBox {
            id: max
            value: 100
            editable: true
            from: min.value
            Layout.fillWidth: true
            visible: widget.currentIndex == 1 || widget.currentIndex == 2
        }
    }
}
