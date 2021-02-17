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

import Qt.labs.settings 1.0
import QtGraphicalEffects 1.0

import "../Widgets" as Widgets
import "../SetupPanes" as SetupPanes

Control {
    id: root
    background: Rectangle {
        color: app.windowBackgroundColor
    }

    //
    // Save settings
    //
    Settings {
        //
        // Misc settings
        //
        property alias auto: commAuto.checked
        property alias manual: commManual.checked
        property alias tabIndex: tab.currentIndex
        property alias csvExport: csvLogging.checked

        //
        // Serial settings
        //
        property alias parity: serial.parity
        property alias baudRate: serial.baudRate
        property alias dataBits: serial.dataBits
        property alias stopBits: serial.stopBits
        property alias flowControl: serial.flowControl

        //
        // Network settings
        //
        property alias port: network.port
        property alias address: network.address
        property alias socketType: network.socketType
        property alias addressLookup: network.addressLookup

        //
        // App settings
        //
        property alias language: settings.language
        property alias endSequence: settings.endSequence
        property alias startSequence: settings.startSequence
    }

    //
    // Update manual/auto checkboxes
    //
    Connections {
        target: Cpp_JSON_Generator
        function onOperationModeChanged() {
            commAuto.checked = (Cpp_JSON_Generator.operationMode == 1)
            commManual.checked = (Cpp_JSON_Generator.operationMode == 0)
        }
    }

    //
    // Control arrangement
    //
    ColumnLayout {
        id: column
        anchors.fill: parent
        spacing: app.spacing / 2
        anchors.margins: app.spacing * 1.5

        //
        // Comm mode selector
        //
        Label {
            font.bold: true
            text: qsTr("Communication Mode") + ":"
        } RadioButton {
            id: commAuto
            checked: true
            text: qsTr("Auto (JSON from serial device)")
            onCheckedChanged: {
                if (checked)
                    Cpp_JSON_Generator.setOperationMode(1)
                else
                    Cpp_JSON_Generator.setOperationMode(0)
            }
        } RadioButton {
            id: commManual
            checked: false
            text: qsTr("Manual (use JSON map file)")
            onCheckedChanged: {
                if (checked)
                    Cpp_JSON_Generator.setOperationMode(0)
                else
                    Cpp_JSON_Generator.setOperationMode(1)
            }
        }

        //
        // Map file selector button
        //
        Button {
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            enabled: commManual.checked
            onClicked: Cpp_JSON_Generator.loadJsonMap()
            Behavior on opacity {NumberAnimation{}}
            text: (Cpp_JSON_Generator.jsonMapFilename.length ? qsTr("Change map file (%1)").arg(Cpp_JSON_Generator.jsonMapFilename) :
                                                               qsTr("Select map file") + "...")
        }

        //
        // Spacer
        //
        Item {
            height: app.spacing / 2
        }

        //
        // Enable/disable CSV logging
        //
        RowLayout {
            Layout.fillWidth: true

            Switch {
                id: csvLogging
                palette.highlight: "#2e895c"
                text: qsTr("Create CSV file")
                Layout.alignment: Qt.AlignVCenter
                checked: Cpp_CSV_Export.exportEnabled
                onCheckedChanged:  {
                    if (Cpp_CSV_Export.exportEnabled !== checked)
                        Cpp_CSV_Export.exportEnabled = checked
                }
            }

            Item {
                Layout.fillWidth: true
            }

            RoundButton {
                icon.width: 24
                icon.height: 24
                icon.color: "#fff"
                icon.source: "qrc:/icons/help.svg"
                Layout.alignment: Qt.AlignVCenter
                onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/wiki")
            }
        }

        //
        // Spacer
        //
        Item {
            height: app.spacing / 2
        }

        //
        // Tab bar
        //
        TabBar {
            height: 24
            id: tab
            Layout.fillWidth: true
            onCurrentIndexChanged: {
                if (currentIndex < 2 && currentIndex !== Cpp_IO_Manager.dataSource)
                    Cpp_IO_Manager.dataSource = currentIndex
            }

            TabButton {
                text: qsTr("Serial")
                height: tab.height + 3
                width: implicitWidth + 2 * app.spacing
            }

            TabButton {
                text: qsTr("Network")
                height: tab.height + 3
                width: implicitWidth + 2 * app.spacing
            }

            TabButton {
                text: qsTr("Settings")
                height: tab.height + 3
                width: implicitWidth + 2 * app.spacing
            }
        }

        //
        // Tab bar contents
        //
        StackLayout {
            id: stack
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: false
            currentIndex: tab.currentIndex
            Layout.topMargin: -parent.spacing - 1
            Layout.minimumHeight: implicitHeight
            Layout.maximumHeight: implicitHeight
            Layout.preferredHeight: implicitHeight
            onCurrentIndexChanged: getImplicitHeight()
            Component.onCompleted: getImplicitHeight()

            function getImplicitHeight() {
                stack.implicitHeight = 0

                switch (currentIndex) {
                case 0:
                    stack.implicitHeight = serial.implicitHeight
                    break
                case 1:
                    stack.implicitHeight = network.implicitHeight
                    break
                case 2:
                    stack.implicitHeight = settings.implicitHeight
                }
            }

            SetupPanes.Serial {
                id: serial
                background: TextField {
                    enabled: false
                    palette.base: "#16232a"
                }
            }

            SetupPanes.Network {
                id: network
                background: TextField {
                    enabled: false
                    palette.base: "#16232a"
                }
            }

            SetupPanes.Settings {
                id: settings
                background: TextField {
                    enabled: false
                    palette.base: "#16232a"
                }
            }
        }

        //
        // Spacer
        //
        Item {
            Layout.fillHeight: true
            Layout.minimumHeight: app.spacing
        }

        //
        // RX/TX LEDs
        //
        RowLayout {
            spacing: app.spacing
            Layout.alignment: Qt.AlignHCenter

            Widgets.LED {
                id: _rx
                enabled: false
                Layout.alignment: Qt.AlignVCenter

                Connections {
                    target: Cpp_IO_Manager
                    function onRx() {
                        _rx.flash()
                    }
                }
            }

            Label {
                text: "RX"
                font.bold: true
                font.pixelSize: 18
                font.family: app.monoFont
                Layout.alignment: Qt.AlignVCenter
                color: _rx.enabled ? palette.highlight : _rx.offColor
            }

            Image {
                width: sourceSize.width
                height: sourceSize.height
                sourceSize: Qt.size(32, 32)
                source: "qrc:/icons/ethernet.svg"
                Layout.alignment: Qt.AlignVCenter

                ColorOverlay {
                    source: parent
                    anchors.fill: parent
                    color: _rx.offColor
                }
            }

            Label {
                text: "TX"
                font.bold: true
                font.pixelSize: 18
                font.family: app.monoFont
                Layout.alignment: Qt.AlignVCenter
                color: _tx.enabled ? palette.highlight : _tx.offColor
            }

            Widgets.LED {
                id: _tx
                enabled: false
                layoutDirection: Qt.RightToLeft
                Layout.alignment: Qt.AlignVCenter

                Connections {
                    target: Cpp_IO_Manager
                    function onTx() {
                        _tx.flash()
                    }
                }
            }
        }

        //
        // Spacer
        //
        Item {
            Layout.minimumHeight: app.spacing * 4
        }
    }
}
