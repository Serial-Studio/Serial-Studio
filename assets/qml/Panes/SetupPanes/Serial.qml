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

import QtQuick 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

Control {
    id: root
    implicitHeight: layout.implicitHeight + app.spacing * 2

    //
    // Access to properties
    //
    property alias port: _portCombo.currentIndex
    property alias baudRate: _baudCombo.currentIndex
    property alias dataBits: _dataCombo.currentIndex
    property alias parity: _parityCombo.currentIndex
    property alias flowControl: _flowCombo.currentIndex
    property alias stopBits: _stopBitsCombo.currentIndex
    property alias autoReconnect: _autoreconnect.checked

    //
    // Update listbox models when translation is changed
    //
    Connections {
        target: Cpp_Misc_Translator
        function onLanguageChanged() {
            var oldParityIndex = _parityCombo.currentIndex
            var oldFlowControlIndex = _flowCombo.currentIndex

            _parityCombo.model = Cpp_IO_Serial.parityList
            _flowCombo.model = Cpp_IO_Serial.flowControlList

            _parityCombo.currentIndex = oldParityIndex
            _flowCombo.currentIndex = oldFlowControlIndex
        }
    }

    //
    // Control layout
    //
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: app.spacing

        //
        // Controls
        //
        GridLayout {
            id: layout
            columns: 2
            Layout.fillWidth: true
            rowSpacing: app.spacing
            columnSpacing: app.spacing

            //
            // COM port selector
            //
            Label {
                opacity: enabled ? 1 : 0.5
                text: qsTr("COM Port") + ":"
                enabled: !Cpp_IO_Manager.connected
            } ComboBox {
                id: _portCombo
                Layout.fillWidth: true
                opacity: enabled ? 1 : 0.5
                model: Cpp_IO_Serial.portList
                enabled: !Cpp_IO_Manager.connected
                currentIndex: Cpp_IO_Serial.portIndex
                onCurrentIndexChanged: {
                    if (currentIndex !== Cpp_IO_Serial.portIndex)
                        Cpp_IO_Serial.portIndex = currentIndex
                }
            }

            //
            // Baud rate selector
            //
            Label {
                opacity: enabled ? 1 : 0.5
                text: qsTr("Baud Rate") + ":"
            } ComboBox {
                id: _baudCombo
                editable: true
                currentIndex: 4
                Layout.fillWidth: true
                model: Cpp_IO_Serial.baudRateList

                validator: IntValidator {
                    bottom: 1
                }

                onAccepted: {
                    if (find(editText) === -1)
                        Cpp_IO_Serial.appendBaudRate(editText)
                }

                onCurrentTextChanged: {
                    var value = currentText
                    Cpp_IO_Serial.baudRate = value
                }
            }

            //
            // Auto-reconnect
            //
            Label {
                text: qsTr("Auto-reconnect") + ":"
            } CheckBox {
                id: _autoreconnect
                Layout.alignment: Qt.AlignLeft
                Layout.leftMargin: -app.spacing
                checked: Cpp_IO_Serial.autoReconnect
                onCheckedChanged: {
                    if (Cpp_IO_Serial.autoReconnect !== checked)
                        Cpp_IO_Serial.autoReconnect = checked
                }
            }

            //
            // Spacer
            //
            Item {
                Layout.minimumHeight: app.spacing / 2
                Layout.maximumHeight: app.spacing / 2
            } Item {
                Layout.minimumHeight: app.spacing / 2
                Layout.maximumHeight: app.spacing / 2
            }

            //
            // Data bits selector
            //
            Label {
                text: qsTr("Data Bits") + ":"
            } ComboBox {
                id: _dataCombo
                Layout.fillWidth: true
                model: Cpp_IO_Serial.dataBitsList
                currentIndex: Cpp_IO_Serial.dataBitsIndex
                onCurrentIndexChanged: {
                    if (Cpp_IO_Serial.dataBitsIndex !== currentIndex)
                        Cpp_IO_Serial.dataBitsIndex = currentIndex
                }
            }

            //
            // Parity selector
            //
            Label {
                text: qsTr("Parity") + ":"
            } ComboBox {
                id: _parityCombo
                Layout.fillWidth: true
                model: Cpp_IO_Serial.parityList
                currentIndex: Cpp_IO_Serial.parityIndex
                onCurrentIndexChanged: {
                    if (Cpp_IO_Serial.parityIndex !== currentIndex)
                        Cpp_IO_Serial.parityIndex = currentIndex
                }
            }

            //
            // Stop bits selector
            //
            Label {
                text: qsTr("Stop Bits") + ":"
            } ComboBox {
                id: _stopBitsCombo
                Layout.fillWidth: true
                model: Cpp_IO_Serial.stopBitsList
                currentIndex: Cpp_IO_Serial.stopBitsIndex
                onCurrentIndexChanged: {
                    if (Cpp_IO_Serial.stopBitsIndex !== currentIndex)
                        Cpp_IO_Serial.stopBitsIndex = currentIndex
                }
            }

            //
            // Flow control selector
            //
            Label {
                text: qsTr("Flow Control") + ":"
            } ComboBox {
                id: _flowCombo
                Layout.fillWidth: true
                model: Cpp_IO_Serial.flowControlList
                currentIndex: Cpp_IO_Serial.flowControlIndex
                onCurrentIndexChanged: {
                    if (Cpp_IO_Serial.flowControlIndex !== currentIndex)
                        Cpp_IO_Serial.flowControlIndex = currentIndex
                }
            }
        }

        //
        // Vertical spacer
        //
        Item {
            Layout.fillHeight: true
        }
    }
}


