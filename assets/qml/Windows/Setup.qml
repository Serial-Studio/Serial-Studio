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

Control {
    id: root
    background: Rectangle {
        color: app.windowBackgroundColor
    }

    //
    // Save settings
    //
    Settings {
        category: "Setup"
        property alias dmAuto: commAuto.checked
        property alias dmManual: commManual.checked
        property alias dmParity: parity.currentIndex
        property alias dmCsvExport: csvLogging.checked
        property alias dmStopBits: stopBits.currentIndex
        property alias dmDataBits: dataBits.currentIndex
        property alias dmBaudValue: baudRate.currentIndex
        property alias dmFlowControl: flowControl.currentIndex
        property alias appLanguage: languageCombo.currentIndex
    }

    //
    // Update listbox models when translation is changed
    //
    Connections {
        target: Cpp_Misc_Translator
        function onLanguageChanged() {
            var oldParityIndex = parity.currentIndex
            var oldFlowControlIndex = flowControl.currentIndex

            parity.model = Cpp_IO_Serial.parityList
            flowControl.model = Cpp_IO_Serial.flowControlList

            parity.currentIndex = oldParityIndex
            flowControl.currentIndex = oldFlowControlIndex
        }
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
                checked: true
                palette.highlight: "#2e895c"
                text: qsTr("Create CSV file")
                Layout.alignment: Qt.AlignVCenter
                onCheckedChanged: Cpp_CSV_Export.exportEnabled = checked
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
        // A lot of comboboxes
        //
        GridLayout {
            columns: 2
            Layout.fillWidth: true
            rowSpacing: app.spacing
            columnSpacing: app.spacing

            //
            // COM port selector
            //
            Label {
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_IO_Manager.connected
                Behavior on opacity {NumberAnimation{}}
                text: qsTr("COM Port") + ":"
            } ComboBox {
                id: portSelector
                Layout.fillWidth: true
                model: Cpp_IO_Serial.portList
                currentIndex: Cpp_IO_Serial.portIndex
                onCurrentIndexChanged: {
                    if (currentIndex !== Cpp_IO_Serial.portIndex)
                        Cpp_IO_Serial.portIndex = currentIndex
                }

                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_IO_Manager.connected
                Behavior on opacity {NumberAnimation{}}
            }

            //
            // Baud rate selector
            //
            Label {
                opacity: enabled ? 1 : 0.5
                Behavior on opacity {NumberAnimation{}}
                text: qsTr("Baud Rate") + ":"
            } ComboBox {
                id: baudRate
                editable: true
                currentIndex: 3
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
                id: dataBits
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
                id: parity
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
                id: stopBits
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
                id: flowControl
                Layout.fillWidth: true
                model: Cpp_IO_Serial.flowControlList
                currentIndex: Cpp_IO_Serial.flowControlIndex
                onCurrentIndexChanged: {
                    if (Cpp_IO_Serial.flowControlIndex !== currentIndex)
                        Cpp_IO_Serial.flowControlIndex = currentIndex
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
            // Language selector
            //
            Label {
                text: qsTr("Language") + ":"
            } ComboBox {
                id: languageCombo
                Layout.fillWidth: true
                model: Cpp_Misc_Translator.availableLanguages
                onCurrentIndexChanged: Cpp_Misc_Translator.setLanguage(currentIndex)
            }
        }

        //
        // Spacer
        //
        Item {
            Layout.fillHeight: true
            Layout.minimumHeight: app.spacing * 2
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
            Layout.fillHeight: true
            Layout.minimumHeight: app.spacing * 2
        }
    }
}
