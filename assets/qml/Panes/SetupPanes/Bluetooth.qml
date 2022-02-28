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

import "../../Widgets" as Widgets

Control {
    id: root
    implicitHeight: layout.implicitHeight + app.spacing * 2

    //
    // Signals
    //
    signal uiChanged()

    //
    // Control layout
    //
    ColumnLayout {
        id: layout
        spacing: app.spacing
        anchors.fill: parent
        anchors.margins: app.spacing

        //
        // Device selector + refresh button
        //
        RowLayout {
            spacing: app.spacing

            Label {
                id: devLabel
                opacity: enabled ? 1 : 0.5
                text: qsTr("Device") + ":"
                enabled: !Cpp_IO_Manager.connected
                Layout.minimumWidth: Math.max(devLabel.implicitWidth, servLabel.implicitWidth)
            }

            ComboBox {
                id: _deviceCombo
                Layout.fillWidth: true
                opacity: enabled ? 1 : 0.5
                model: Cpp_IO_Bluetooth.devices
                enabled: !Cpp_IO_Manager.connected
                onCurrentIndexChanged: {
                    if (currentIndex !== Cpp_IO_Bluetooth.currentDevice + 1)
                        Cpp_IO_Bluetooth.currentDevice = currentIndex

                    root.uiChanged()
                }
            }

            Button {
                width: 24
                height: 24
                icon.width: 16
                icon.height: 16
                opacity: enabled ? 1 : 0.5
                icon.color: Cpp_ThemeManager.text
                icon.source: "qrc:/icons/refresh.svg"
                enabled: !Cpp_IO_Bluetooth.isScanning
                onClicked: Cpp_IO_Bluetooth.beginScanning()
            }
        }

        //
        // Service selector
        //
        RowLayout {
            spacing: app.spacing
            enabled: Cpp_IO_Bluetooth.supported
            visible: Cpp_IO_Bluetooth.supported

            Label {
                id: servLabel
                opacity: enabled ? 1 : 0.5
                text: qsTr("Service") + ":"
                enabled: !Cpp_IO_Manager.connected
                Layout.minimumWidth: Math.max(devLabel.implicitWidth, servLabel.implicitWidth)
            }

            ComboBox {
                id: _service
                Layout.fillWidth: true
                opacity: enabled ? 1 : 0.5
                model: Cpp_IO_Bluetooth.services
                enabled: !Cpp_IO_Manager.connected
            }
        }

        //
        // RSSI
        //
        RowLayout {
            spacing: app.spacing
            visible: _deviceCombo.currentIndex >= 1

            Label {
                id: rssiLabel
                opacity: enabled ? 1 : 0.5
                text: qsTr("RSSI") + ":"
                enabled: !Cpp_IO_Manager.connected
                Layout.minimumWidth: Math.max(devLabel.implicitWidth, servLabel.implicitWidth)
            }

            Label {
                id: _rssi
                Layout.fillWidth: true
                opacity: enabled ? 1 : 0.5
                text: Cpp_IO_Bluetooth.rssi
                enabled: !Cpp_IO_Manager.connected
            }
        }

        //
        // Dinamic spacer
        //
        Item {
            height: app.spacing
            visible: !Cpp_IO_Bluetooth.supported && Cpp_IO_Bluetooth.currentDevice >= 0
        }

        //
        // Not at BLE device warning
        //
        RowLayout {
            spacing: app.spacing
            visible: !Cpp_IO_Bluetooth.supported && Cpp_IO_Bluetooth.currentDevice >= 0

            Widgets.Icon {
                width: 18
                height: 18
                color: palette.text
                source: "qrc:/icons/warning.svg"
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Label.WordWrap
                text: "<b>" + qsTr("Note:") + "</b> " + qsTr("This is not a BLE device, I/O operations " +
                                                             "are not yet supported for classic Bluetooth devices.")
            }
        }

        //
        // Not at BLE device warning
        //
        RowLayout {
            spacing: app.spacing
            visible: opacity > 0
            opacity: Cpp_IO_Bluetooth.isScanning ? 1 : 0

            Behavior on opacity {NumberAnimation{}}

            BusyIndicator {
                Layout.minimumWidth: 16
                Layout.minimumHeight: 16
                running: Cpp_IO_Bluetooth.isScanning
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("Scanning....")
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


