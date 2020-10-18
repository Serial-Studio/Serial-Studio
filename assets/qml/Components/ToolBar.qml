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

import Qt.labs.settings 1.0

ToolBar {
    id: toolbar
    height: 48

    //
    // Custom properties
    //
    property alias devicesChecked: _devices.checked
    property alias consoleChecked: _console.checked
    property alias dataDisChecked: _dataDis.checked
    property alias fullscrChecked: _fullScr.checked

    //
    // Dummy string to increase width of buttons
    //
    readonly property string _btSpacer: "  "

    //
    // Custom signals
    //
    signal aboutClicked()

    //
    // Settings handler
    //
    Settings {
        category: "Toolbar"
        property alias devices: toolbar.devicesChecked
        property alias console: toolbar.consoleChecked
        property alias dataDis: toolbar.dataDisChecked
    }

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
            id: _devices
            flat: true
            checked: true
            icon.width: 24
            icon.height: 24
            checkable: true
            Layout.fillHeight: true
            icon.color: palette.text
            icon.source: "qrc:/icons/usb.svg"
            text: qsTr("Device Manager") + _btSpacer
        }

        Button {
            id: _console
            flat: true
            checked: true
            icon.width: 24
            icon.height: 24
            checkable: true
            Layout.fillHeight: true
            icon.color: palette.text
            icon.source: "qrc:/icons/code.svg"
            text: qsTr("Console") + _btSpacer
        }

        Button {
            id: _dataDis
            flat: true
            checked: true
            icon.width: 24
            icon.height: 24
            checkable: true
            Layout.fillHeight: true
            icon.color: palette.text
            icon.source: "qrc:/icons/equalizer.svg"
            text: qsTr("Data Display") + _btSpacer
        }

        Button {
            id: _fullScr
            flat: true
            checkable: true
            icon.width: 24
            icon.height: 24
            Layout.fillHeight: true
            icon.color: palette.text
            onCheckedChanged: checked ? app.showFullScreen() : app.showNormal()
            text: (checked ? qsTr("Exit Fullscreen") : qsTr("Enter Fullscreen")) + _btSpacer
            icon.source: checked ? "qrc:/icons/fullscreen-exit.svg" :
                                   "qrc:/icons/fullscreen.svg"
        }

        Button {
            flat: true
            icon.width: 24
            icon.height: 24
            text: qsTr("About")
            Layout.fillHeight: true
            icon.color: palette.text
            onClicked: toolbar.aboutClicked()
            icon.source: "qrc:/icons/info.svg"
        }

        Item {
            Layout.fillWidth: true
        }

        Button {
            id: _openCsv
            flat: true
            icon.width: 24
            icon.height: 24
            Layout.fillHeight: true
            icon.color: palette.text
            enabled: CppExport.isOpen
            opacity: enabled ? 1 : 0.5
            onClicked: CppExport.openCsv()
            icon.source: "qrc:/icons/open.svg"
            text: qsTr("Open CSV File") + _btSpacer

            Behavior on opacity {NumberAnimation{}}
        }
    }
}

