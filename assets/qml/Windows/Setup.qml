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
    // Serial-open modes list
    //
    property var serialOpenModes: [qsTr("Read-only"), qsTr("Read/write")]

    //
    // Save settings
    //
    Settings {
        category: "Setup"
        property alias dmAuto: commAuto.checked
        property alias dmManual: commManual.checked
        property alias dmParity: parity.currentIndex
        property alias dmStopBits: stopBits.currentIndex
        property alias dmDataBits: dataBits.currentIndex
        property alias dmOpenMode: openMode.currentIndex
        property alias dmBaudIndex: baudRate.currentIndex
        property alias dmCustomBrEnabled: customBr.checked
        property alias dmCustomBaudRate: customBaudRateValue.value
        property alias dmFlowControl: flowControl.currentIndex
        property alias appLanguage: languageCombo.currentIndex
        property alias dmDisplayMode: displayMode.currentIndex
    }

    //
    // Update listbox models when translation is changed
    //
    Connections {
        target: CppTranslator
        function onLanguageChanged() {
            var oldParityIndex = parity.currentIndex
            var oldOpenModeIndex = openMode.currentIndex
            var oldDisplayModeIndex = displayMode.currentIndex
            var oldFlowControlIndex = flowControl.currentIndex

            root.serialOpenModes = [qsTr("Read-only"), qsTr("Read/write")]
            openMode.model = root.serialOpenModes

            parity.model = CppSerialManager.parityList
            flowControl.model = CppSerialManager.flowControlList
            displayMode.model = CppSerialManager.consoleDisplayModes

            parity.currentIndex = oldParityIndex
            openMode.currentIndex = oldOpenModeIndex
            flowControl.currentIndex = oldFlowControlIndex
            displayMode.currentIndex = oldDisplayModeIndex
        }
    }

    //
    // Update manual/auto checkboxes
    //
    Connections {
        target: CppJsonGenerator
        function onOperationModeChanged() {
            commAuto.checked = (CppJsonGenerator.operationMode == 1)
            commManual.checked = (CppJsonGenerator.operationMode == 0)
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
            text: qsTr("Communication Mode") + ":" + CppTranslator.dummy
        } RadioButton {
            id: commAuto
            checked: true
            text: qsTr("Auto (JSON from serial device)") + CppTranslator.dummy
            onCheckedChanged: {
                if (checked)
                    CppJsonGenerator.setOperationMode(1)
                else
                    CppJsonGenerator.setOperationMode(0)
            }
        } RadioButton {
            id: commManual
            checked: false
            text: qsTr("Manual (use JSON map file)") + CppTranslator.dummy
            onCheckedChanged: {
                if (checked)
                    CppJsonGenerator.setOperationMode(0)
                else
                    CppJsonGenerator.setOperationMode(1)
            }
        }

        //
        // Map file selector button
        //
        Button {
            Layout.fillWidth: true
            opacity: enabled ? 1 : 0.5
            enabled: commManual.checked
            onClicked: CppJsonGenerator.loadJsonMap()
            Behavior on opacity {NumberAnimation{}}
            text: CppTranslator.dummy +
                  (CppJsonGenerator.jsonMapFilename.length ? qsTr("Change map file (%1)").arg(CppJsonGenerator.jsonMapFilename) :
                                                             qsTr("Select map file") + "...")
        }

        //
        // Spacer
        //
        Item {
            height: app.spacing * 2
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
                text: qsTr("COM Port") + ":" + CppTranslator.dummy
            } ComboBox {
                id: portSelector
                Layout.fillWidth: true
                model: CppSerialManager.portList
                currentIndex: CppSerialManager.portIndex
                onCurrentIndexChanged: CppSerialManager.setPort(currentIndex)
            }

            //
            // Baud rate selector
            //
            Label {
                text: qsTr("Baud Rate") + ":" + CppTranslator.dummy

                enabled: !customBr.checked
                opacity: enabled ? 1 : 0.5
                Behavior on opacity {NumberAnimation{}}

            } ComboBox {
                id: baudRate
                Layout.fillWidth: true
                model: CppSerialManager.baudRateList
                currentIndex: CppSerialManager.baudRateIndex
                onCurrentIndexChanged: {
                    if (CppSerialManager.baudRateIndex !== currentIndex && enabled)
                        CppSerialManager.baudRateIndex = currentIndex
                }

                enabled: !customBr.checked
                opacity: enabled ? 1 : 0.5
                Behavior on opacity {NumberAnimation{}}
            }

            //
            // Open mode
            //
            Label {
                text: qsTr("Open mode") + ":" + CppTranslator.dummy
            } ComboBox {
                id: openMode
                Layout.fillWidth: true
                model: root.serialOpenModes
                onCurrentIndexChanged: {
                    if (currentIndex == 0)
                        CppSerialManager.setWriteEnabled(false)
                    else
                        CppSerialManager.setWriteEnabled(true)
                }
            }

            //
            // Display mode selector
            //
            Label {
                text: qsTr("Display mode") + ":" + CppTranslator.dummy
            } ComboBox {
                id: displayMode
                Layout.fillWidth: true
                model: CppSerialManager.consoleDisplayModes
                currentIndex: CppSerialManager.displayMode
                onCurrentIndexChanged: {
                    if (CppSerialManager.displayMode !== currentIndex)
                        CppSerialManager.displayMode = currentIndex
                }
            }

            //
            // Custom baud rate
            //
            CheckBox {
                id: customBr
                Layout.leftMargin: -app.spacing
                text: qsTr("Custom baud rate") + CppTranslator.dummy
            } SpinBox {
                id: customBaudRateValue

                from: 1
                stepSize: 1
                to: 10000000
                value: 9600
                editable: true
                Layout.fillWidth: true
                enabled: customBr.checked
                opacity: enabled ? 1 : 0.5
                Behavior on opacity {NumberAnimation{}}

                onValueChanged: {
                    if (enabled)
                        CppSerialManager.setBaudRate(value)
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
                text: qsTr("Data Bits") + ":" + CppTranslator.dummy
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
                text: qsTr("Parity") + ":" + CppTranslator.dummy
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
                text: qsTr("Stop Bits") + ":" + CppTranslator.dummy
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
                text: qsTr("Flow Control") + ":" + CppTranslator.dummy
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
                text: qsTr("Language") + ":" + CppTranslator.dummy
            } ComboBox {
                id: languageCombo
                Layout.fillWidth: true
                model: CppTranslator.availableLanguages
                onCurrentIndexChanged: CppTranslator.setLanguage(currentIndex)
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
            Layout.minimumHeight: app.spacing * 2
        }
    }
}
