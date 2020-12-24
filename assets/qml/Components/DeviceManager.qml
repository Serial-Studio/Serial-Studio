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
import QtGraphicalEffects 1.0

import Qt.labs.settings 1.0

import "../Widgets" as Widgets

Widgets.Window {
    //
    // Window properties
    //
    implicitWidth: 256
    title: qsTr("Device Manager")
    icon.source: "qrc:/icons/usb.svg"

    //
    // Save settings
    //
    Settings {
        category: "Device Manager"
        property alias dmParity: parity.currentIndex
        property alias dmStopBits: stopBits.currentIndex
        property alias dmBaudRate: baudRate.currentIndex
        property alias dmDataBits: dataBits.currentIndex
        property alias dmFlowControl: flowControl.currentIndex
    }

    //
    // Control arrangement
    //
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: app.spacing * 1.5
        spacing: app.spacing / 2

        //
        // Comm mode selector
        //
        Label {
            text: qsTr("Communication Mode") + ":"
        } RadioButton {
            id: commAuto
            checked: true
            text: qsTr("Auto (JSON from serial device)")
        } RadioButton {
            id: commManual
            checked: false
            text: qsTr("Manual (use JSON map file)")
        }

        //
        // Map file selector button
        //
        Button {
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            enabled: commManual.checked
            text: qsTr("Select map file") + "..."
            Behavior on opacity {NumberAnimation{}}
        }

        //
        // Spacer
        //
        Item {
            height: app.spacing * 2
        }

        //
        // COM port selector
        //
        Label {
            text: qsTr("COM Port") + ":"
        } ComboBox {
            Layout.fillWidth: true
            model: CppSerialManager.portList
            currentIndex: CppSerialManager.portIndex
            onCurrentIndexChanged: {
                if (CppSerialManager.portIndex !== currentIndex)
                    CppSerialManager.portIndex = currentIndex
            }
        }

        //
        // Baud rate selector
        //
        Label {
            text: qsTr("Baud Rate") + ":"
        } ComboBox {
            id: baudRate
            Layout.fillWidth: true
            model: CppSerialManager.baudRateList
            currentIndex: CppSerialManager.baudRateIndex
            onCurrentIndexChanged: {
                if (CppSerialManager.baudRateIndex !== currentIndex)
                    CppSerialManager.baudRateIndex = currentIndex
            }
        }

        //
        // Spacer
        //
        Item {
            height: app.spacing * 2
        }

        //
        // Data bits selector
        //
        Label {
            text: qsTr("Data Bits") + ":"
        } ComboBox {
            id: dataBits
            Layout.fillWidth: true
            model: CppSerialManager.dataBitsList
            currentIndex: CppSerialManager.dataBitsIndex
            onCurrentIndexChanged: {
                if (CppSerialManager.dataBitsIndex !== currentIndex)
                    CppSerialManager.dataBitsIndex = currentIndex
            }
        }

        //
        // Parity selector
        //
        Label {
            text: qsTr("Parity") + ":"
        } ComboBox {
            id: parity
            Layout.fillWidth: true
            model: CppSerialManager.parityList
            currentIndex: CppSerialManager.parityIndex
            onCurrentIndexChanged: {
                if (CppSerialManager.parityIndex !== currentIndex)
                    CppSerialManager.parityIndex = currentIndex
            }
        }

        //
        // Stop bits selector
        //
        Label {
            text: qsTr("Stop Bits") + ":"
        } ComboBox {
            id: stopBits
            Layout.fillWidth: true
            model: CppSerialManager.stopBitsList
            currentIndex: CppSerialManager.stopBitsIndex
            onCurrentIndexChanged: {
                if (CppSerialManager.stopBitsIndex !== currentIndex)
                    CppSerialManager.stopBitsIndex = currentIndex
            }
        }

        //
        // Flow control selector
        //
        Label {
            text: qsTr("Flow Control") + ":"
        } ComboBox {
            id: flowControl
            Layout.fillWidth: true
            model: CppSerialManager.flowControlList
            currentIndex: CppSerialManager.flowControlIndex
            onCurrentIndexChanged: {
                if (CppSerialManager.flowControlIndex !== currentIndex)
                    CppSerialManager.flowControlIndex = currentIndex
            }
        }

        //
        // Spacer
        //
        Item {
            Layout.fillHeight: true
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
                    target: CppSerialManager
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
                    target: CppSerialManager
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
            Layout.fillHeight: true
        }
    }
}
