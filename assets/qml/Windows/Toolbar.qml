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

Page {
    id: root

    //
    // Dummy string to increase width of buttons
    //
    readonly property string _btSpacer: "  "

    //
    // Custom signals
    //
    signal dataClicked()
    signal aboutClicked()
    signal devicesClicked()
    signal consoleClicked()
    signal widgetsClicked()

    //
    // Aliases to button check status
    //
    property alias dataChecked: dataBt.checked
    property alias aboutChecked: aboutBt.checked
    property alias consoleChecked: consoleBt.checked
    property alias widgetsChecked: widgetsBt.checked
    property alias devicesChecked: devicesBt.checked

    //
    // Background gradient
    //
    Rectangle {
        border.width: 1
        color: Qt.rgba(33/255, 55/255, 63/255, 1)
        border.color: Qt.darker(color)

        anchors {
            fill: parent
            topMargin: -border.width
            leftMargin: -border.width * 10
            rightMargin: -border.width * 10
        }
    }

    //
    // Toolbar icons
    //
    RowLayout {
        spacing: app.spacing
        anchors.fill: parent
        anchors.margins: app.spacing

        Button {
            id: devicesBt

            flat: true
            icon.width: 24
            icon.height: 24
            Layout.fillHeight: true
            icon.color: palette.text
            onClicked: root.devicesClicked()
            icon.source: "qrc:/icons/usb.svg"
            text: qsTr("Devices") + _btSpacer
        }

        Button {
            id: consoleBt

            flat: true
            icon.width: 24
            icon.height: 24
            Layout.fillHeight: true
            icon.color: palette.text
            onClicked: root.consoleClicked()
            icon.source: "qrc:/icons/code.svg"
            text: qsTr("Console") + _btSpacer
            enabled: dataBt.enabled || widgetsBt.enabled
        }

        Button {
            id: dataBt

            flat: true
            icon.width: 24
            icon.height: 24
            Layout.fillHeight: true
            icon.color: palette.text
            onClicked: root.dataClicked()
            enabled: CppQmlBridge.groupCount > 0
            icon.source: "qrc:/icons/equalizer.svg"
            text: qsTr("Data Display") + _btSpacer

            opacity: enabled ? 1 : 0.5
            Behavior on opacity {NumberAnimation{}}
        }

        Button {
            id: widgetsBt

            flat: true
            icon.width: 24
            icon.height: 24
            Layout.fillHeight: true
            icon.color: palette.text
            onClicked: root.widgetsClicked()
            icon.source: "qrc:/icons/chart.svg"
            text: qsTr("Widgets") + _btSpacer
            enabled: CppWidgets.totalWidgetCount > 0

            opacity: enabled ? 1 : 0.5
            Behavior on opacity {NumberAnimation{}}
        }

        Button {
            id: aboutBt

            flat: true
            icon.width: 24
            icon.height: 24
            text: qsTr("About")
            Layout.fillHeight: true
            icon.color: palette.text
            onClicked: root.aboutClicked()
            icon.source: "qrc:/icons/info.svg"
        }

        Item {
            Layout.fillWidth: true
        }

        Button {
            flat: true
            icon.width: 24
            icon.height: 24
            Layout.fillHeight: true
            icon.color: palette.text
            opacity: enabled ? 1 : 0.5
            onClicked: CppExport.openCsv()
            icon.source: "qrc:/icons/update.svg"
            text: qsTr("Open past CSV") + _btSpacer

            Behavior on opacity {NumberAnimation{}}
        }

        Button {
            flat: true
            icon.width: 24
            icon.height: 24
            Layout.fillHeight: true
            icon.color: palette.text
            enabled: CppExport.isOpen
            opacity: enabled ? 1 : 0.5
            icon.source: "qrc:/icons/open.svg"
            onClicked: CppExport.openCurrentCsv()
            text: qsTr("Open current CSV") + _btSpacer

            Behavior on opacity {NumberAnimation{}}
        }
    }
}
