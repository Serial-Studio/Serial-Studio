/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Control {
    id: root

    //
    // Start BLE scanning 2 seconds after the control is created
    //
    Component.onCompleted: timer.start()
    Timer {
        id: timer
        interval: 2000
        onTriggered: Cpp_IO_Bluetooth_LE.startDiscovery()
    }

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
            visible: opacity > 0
            opacity: Cpp_IO_Bluetooth_LE.operatingSystemSupported && Cpp_IO_Bluetooth_LE.deviceCount > 0 ? 1 : 0

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
                enabled: !Cpp_IO_Manager.connected
                model: Cpp_IO_Bluetooth_LE.deviceNames
                palette.base: Cpp_ThemeManager.setupPanelBackground
                onCurrentIndexChanged: {
                    if (currentIndex !== Cpp_IO_Bluetooth_LE.currentDevice)
                        Cpp_IO_Bluetooth_LE.selectDevice(currentIndex)
                }
            }

            Button {
                width: 24
                height: 24
                icon.width: 16
                icon.height: 16
                opacity: enabled ? 1 : 0.5
                icon.color: Cpp_ThemeManager.text
                enabled: !Cpp_IO_Manager.connected
                icon.source: "qrc:/icons/refresh.svg"
                onClicked: Cpp_IO_Bluetooth_LE.startDiscovery()
                palette.base: Cpp_ThemeManager.setupPanelBackground
            }
        }

        //
        // Service selector
        //
        RowLayout {
            spacing: app.spacing
            visible: opacity > 0
            opacity: Cpp_IO_Bluetooth_LE.operatingSystemSupported && serviceNames.count > 1 ? 1 : 0

            Label {
                id: servLabel
                text: qsTr("Service") + ":"
                Layout.minimumWidth: Math.max(devLabel.implicitWidth, servLabel.implicitWidth)
            }

            ComboBox {
                id: serviceNames
                Layout.fillWidth: true
                model: Cpp_IO_Bluetooth_LE.serviceNames
                palette.base: Cpp_ThemeManager.setupPanelBackground
                onCurrentIndexChanged: Cpp_IO_Bluetooth_LE.selectService(currentIndex)
            }
        }

        //
        // Scanning indicator
        //
        RowLayout {
            spacing: app.spacing
            visible: opacity > 0
            opacity: Cpp_IO_Bluetooth_LE.operatingSystemSupported && Cpp_IO_Bluetooth_LE.deviceCount < 1 ? 1 : 0

            BusyIndicator {
                running: parent.visible
                Layout.minimumWidth: 16
                Layout.minimumHeight: 16
                Layout.alignment: Qt.AlignVCenter
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("Scanning....")
                Layout.alignment: Qt.AlignVCenter
            }
        }

        //
        // OS not supported indicator
        //
        RowLayout {
            spacing: app.spacing
            visible: opacity > 0
            opacity: !Cpp_IO_Bluetooth_LE.operatingSystemSupported

            ToolButton {
                flat: true
                enabled: false
                icon.width: 32
                icon.height: 32
                Layout.alignment: Qt.AlignVCenter
                icon.source: "qrc:/icons/heart-broken.svg"
                icon.color: Cpp_ThemeManager.connectButtonChecked
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Label.WordWrap
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("Sorry, this version of %1 is not supported yet. " +
                           "We'll update Serial Studio to work with this operating " +
                           "system as soon as Qt officially supports it.").arg(Cpp_OSName);
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


