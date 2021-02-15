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
    // Access to properties
    //
    property alias port: _portText.text
    property alias address: _ipText.text
    property alias socketType: _typeCombo.currentIndex

    //
    // Layout
    //
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: app.spacing / 2

        GridLayout {
            columns: 2
            Layout.fillWidth: true
            rowSpacing: app.spacing
            columnSpacing: app.spacing

            //
            // Socket type
            //
            Label {
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_IO_Manager.connected
                Behavior on opacity {NumberAnimation{}}
                text: qsTr("Socket type") + ":"
            } ComboBox {
                id: _typeCombo
                Layout.fillWidth: true
                model: Cpp_IO_Network.socketTypes
                currentIndex: Cpp_IO_Network.socketTypeIndex
                onCurrentIndexChanged: {
                    if (currentIndex !== Cpp_IO_Network.socketTypeIndex)
                        Cpp_IO_Network.socketTypeIndex = currentIndex
                }

                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_IO_Manager.connected
                Behavior on opacity {NumberAnimation{}}
            }

            //
            // Address
            //
            Label {
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_IO_Manager.connected
                Behavior on opacity {NumberAnimation{}}
                text: qsTr("IP Address") + ":"
            } TextField {
                id: _ipText
                Layout.fillWidth: true
                placeholderText: Cpp_IO_Network.defaultHost
                text: Cpp_IO_Network.host
                onTextChanged: {
                    if (Cpp_IO_Network.host !== text)
                        Cpp_IO_Network.host = text
                }

                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_IO_Manager.connected
                Behavior on opacity {NumberAnimation{}}
            }

            //
            // Port
            //
            Label {
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_IO_Manager.connected
                Behavior on opacity {NumberAnimation{}}
                text: qsTr("Port") + ":"
            } TextField {
                id: _portText
                Layout.fillWidth: true
                text: Cpp_IO_Network.port
                placeholderText: Cpp_IO_Network.defaultPort
                onTextChanged: {
                    if (Cpp_IO_Network.port !== text)
                        Cpp_IO_Network.port = text
                }

                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_IO_Manager.connected
                Behavior on opacity {NumberAnimation{}}
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
