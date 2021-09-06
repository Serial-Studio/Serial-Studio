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

Control {
    id: root

    //
    // Dummy string to increase width of buttons
    //
    readonly property string _btSpacer: "  "

    //
    // Custom signals
    //
    signal dataClicked()
    signal setupClicked()
    signal consoleClicked()
    signal widgetsClicked()

    //
    // Aliases to button check status
    //
    property alias dataChecked: dataBt.checked
    property alias setupChecked: setupBt.checked
    property alias consoleChecked: consoleBt.checked
    property alias widgetsChecked: widgetsBt.checked

    //
    // Background gradient
    //
    Rectangle {
        border.width: 1
        border.color: palette.midlight

        Rectangle {
            height: parent.border.width
            visible: Qt.platform.os === "osx"
            color: Cpp_ThemeManager.toolbarGradient2

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
        }

        gradient: Gradient {
            GradientStop { position: 0; color: Cpp_ThemeManager.toolbarGradient1 }
            GradientStop { position: 1; color: Cpp_ThemeManager.toolbarGradient2 }
        }

        anchors {
            fill: parent
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
            id: setupBt

            flat: true
            icon.width: 24
            icon.height: 24
            Layout.fillHeight: true
            onClicked: root.setupClicked()
            text: qsTr("Setup") + _btSpacer
            icon.source: "qrc:/icons/settings.svg"
            icon.color: Cpp_ThemeManager.brightText
            palette.buttonText: Cpp_ThemeManager.brightText
            palette.button: Cpp_ThemeManager.toolbarGradient1
            palette.window: Cpp_ThemeManager.toolbarGradient1
        }

        Button {
            id: consoleBt

            flat: true
            icon.width: 24
            icon.height: 24
            Layout.fillHeight: true
            onClicked: root.consoleClicked()
            icon.source: "qrc:/icons/code.svg"
            text: qsTr("Console") + _btSpacer
            enabled: dataBt.enabled || widgetsBt.enabled
            icon.color: Cpp_ThemeManager.brightText
            palette.buttonText: Cpp_ThemeManager.brightText
            palette.button: Cpp_ThemeManager.toolbarGradient1
            palette.window: Cpp_ThemeManager.toolbarGradient1
        }

        Button {
            id: dataBt

            flat: true
            icon.width: 24
            icon.height: 24
            Layout.fillHeight: true
            onClicked: root.dataClicked()
            enabled: app.dashboardAvailable
            text: qsTr("Dashboard") + _btSpacer
            icon.source: "qrc:/icons/equalizer.svg"
            icon.color: Cpp_ThemeManager.brightText
            palette.buttonText: Cpp_ThemeManager.brightText
            palette.button: Cpp_ThemeManager.toolbarGradient1
            palette.window: Cpp_ThemeManager.toolbarGradient1

            opacity: enabled ? 1 : 0.5
            Behavior on opacity {NumberAnimation{}}
        }

        Button {
            id: widgetsBt

            flat: true
            icon.width: 24
            icon.height: 24
            Layout.fillHeight: true
            enabled: app.widgetsAvailable
            onClicked: root.widgetsClicked()
            text: qsTr("Widgets") + _btSpacer
            icon.source: "qrc:/icons/chart.svg"
            icon.color: Cpp_ThemeManager.brightText
            palette.buttonText: Cpp_ThemeManager.brightText
            palette.button: Cpp_ThemeManager.toolbarGradient1
            palette.window: Cpp_ThemeManager.toolbarGradient1

            opacity: enabled ? 1 : 0.5
            Behavior on opacity {NumberAnimation{}}
        }

        Item {
            Layout.fillWidth: true
        }

        Button {
            flat: true
            icon.width: 24
            icon.height: 24
            Layout.fillHeight: true
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_CSV_Player.isOpen
            icon.source: "qrc:/icons/open.svg"
            text: qsTr("Open CSV") + _btSpacer
            icon.color: Cpp_ThemeManager.brightText
            palette.buttonText: Cpp_ThemeManager.brightText
            palette.button: Cpp_ThemeManager.toolbarGradient1
            palette.window: Cpp_ThemeManager.toolbarGradient1
            

            onClicked: {
                if (Cpp_CSV_Export.isOpen)
                    Cpp_CSV_Export.openCurrentCsv()
                else
                    Cpp_CSV_Player.openFile()
            }

            Behavior on opacity {NumberAnimation{}}
        }

        Button {
            id: connectBt

            //
            // Button properties
            //
            flat: true
            icon.width: 24
            icon.height: 24
            font.bold: true
            Layout.fillHeight: true
            icon.color: palette.buttonText

            //
            // Connection-dependent
            //
            checked: Cpp_IO_Manager.connected
            text: (checked ? qsTr("Disconnect") :
                             qsTr("Connect")) + _btSpacer
            icon.source: checked ? "qrc:/icons/disconnect.svg" :
                                   "qrc:/icons/connect.svg"
            palette.button: Cpp_ThemeManager.toolbarGradient1
            palette.window: Cpp_ThemeManager.toolbarGradient1
            palette.buttonText: checked ? Cpp_ThemeManager.connectButtonChecked :
                                          Cpp_ThemeManager.connectButtonUnchecked

            //
            // Only enable button if it can be clicked
            //
            opacity: enabled ? 1 : 0.5
            Behavior on opacity {NumberAnimation{}}
            enabled: Cpp_IO_Manager.configurationOk

            //
            // Connect/disconnect device when button is clicked
            //
            onClicked: Cpp_IO_Manager.toggleConnection()
        }
    }
}
