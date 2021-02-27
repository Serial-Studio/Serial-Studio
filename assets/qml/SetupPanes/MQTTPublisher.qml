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
    implicitHeight: layout.implicitHeight + app.spacing * 2

    //
    // Aliases
    //
    property alias host: _host.text
    property alias port: _port.text
    property alias topic: _topic.text
    property alias user: _user.text
    property alias password: _password.text
    property alias dnsAddress: _addrLookup.text

    //
    // Layout
    //
    ColumnLayout {
        id: layout
        anchors.fill: parent
        anchors.margins: app.spacing

        GridLayout {
            columns: 2
            Layout.fillWidth: true
            rowSpacing: app.spacing
            columnSpacing: app.spacing

            //
            // MQTT version
            //
            Label {
                text: qsTr("Version") + ":"
            } ComboBox {
                Layout.fillWidth: true
                model: Cpp_MQTT_Publisher.mqttVersions
                currentIndex: Cpp_MQTT_Publisher.mqttVersion
                onCurrentIndexChanged: {
                    if (Cpp_MQTT_Publisher.mqttVersion !== currentIndex)
                        Cpp_MQTT_Publisher.mqttVersion = currentIndex
                }
            }

            //
            // Host
            //
            Label {
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Publisher.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
                text: qsTr("Host") + ":"
            } TextField {
                id: _host
                Layout.fillWidth: true
                text: Cpp_MQTT_Publisher.host
                placeholderText: Cpp_MQTT_Publisher.defaultHost
                onTextChanged: {
                    if (Cpp_MQTT_Publisher.host !== text)
                        Cpp_MQTT_Publisher.host = text
                }

                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Publisher.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
            }

            //
            // Port
            //
            Label {
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Publisher.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
                text: qsTr("Port") + ":"
            } TextField {
                id: _port
                Layout.fillWidth: true
                text: Cpp_MQTT_Publisher.port
                placeholderText: Cpp_MQTT_Publisher.defaultPort
                onTextChanged: {
                    if (Cpp_MQTT_Publisher.port !== text)
                        Cpp_MQTT_Publisher.port = text
                }

                validator: IntValidator {
                    bottom: 0
                    top: 65535
                }

                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Publisher.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
            }

            //
            // Topic
            //
            Label {
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Publisher.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
                text: qsTr("Topic") + ":"
            } TextField {
                id: _topic
                Layout.fillWidth: true
                text: Cpp_MQTT_Publisher.topic
                placeholderText: qsTr("MQTT topic")
                onTextChanged: {
                    if (Cpp_MQTT_Publisher.topic !== text)
                        Cpp_MQTT_Publisher.topic = text
                }

                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Publisher.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
            }

            //
            // Username
            //
            Label {
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Publisher.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
                text: qsTr("User") + ":"
            } TextField {
                id: _user
                Layout.fillWidth: true
                text: Cpp_MQTT_Publisher.username
                placeholderText: qsTr("MQTT username")
                onTextChanged: {
                    if (Cpp_MQTT_Publisher.username !== text)
                        Cpp_MQTT_Publisher.username = text
                }

                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Publisher.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
            }

            //
            // Password
            //
            Label {
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Publisher.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
                text: qsTr("Password") + ":"
            } TextField {
                id: _password
                Layout.fillWidth: true
                text: Cpp_MQTT_Publisher.password
                placeholderText: qsTr("MQTT password")
                onTextChanged: {
                    if (Cpp_MQTT_Publisher.password !== text)
                        Cpp_MQTT_Publisher.password = text
                }

                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Publisher.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
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
        // Address lookup
        //
        Label {
            text: qsTr("DNS lookup") + ": "
            opacity: enabled ? 1 : 0.5
            enabled: !Cpp_MQTT_Publisher.isConnectedToHost
            Behavior on opacity {NumberAnimation{}}
        } RowLayout {
            Layout.fillWidth: true
            spacing: app.spacing / 2

            TextField {
                id: _addrLookup
                Layout.fillWidth: true
                opacity: enabled ? 1 : 0.5
                Layout.alignment: Qt.AlignVCenter
                onAccepted: Cpp_MQTT_Publisher.lookup(text)
                placeholderText: qsTr("Enter address (e.g. google.com)")
                enabled: !Cpp_MQTT_Publisher.isConnectedToHost &&
                         !Cpp_MQTT_Publisher.lookupActive

                Behavior on opacity {NumberAnimation{}}
            }

            Button {
                icon.color: palette.text
                opacity: enabled ? 1 : 0.5
                Layout.maximumWidth: height
                Layout.alignment: Qt.AlignVCenter
                icon.source: "qrc:/icons/search.svg"
                onClicked: Cpp_MQTT_Publisher.lookup(_addrLookup.text)
                enabled: _addrLookup.text.length > 0 &&
                         !Cpp_MQTT_Publisher.isConnectedToHost &&
                         !Cpp_MQTT_Publisher.lookupActive

                Behavior on opacity {NumberAnimation{}}
            }
        }

        //
        // Spacer
        //
        Item {
            Layout.fillHeight: true
        }

        //
        // Connect/disconnect button
        //
        Button {
            icon.width: 24
            icon.height: 24
            font.bold: true
            Layout.fillWidth: true
            icon.color: palette.buttonText
            checked: Cpp_MQTT_Publisher.isConnectedToHost
            onClicked: Cpp_MQTT_Publisher.toggleConnection()
            palette.buttonText: checked ? "#d72d60" : "#2eed5c"
            text: (checked ? qsTr("Disconnect") : qsTr("Connect")) + "  "
            icon.source: checked ? "qrc:/icons/disconnect.svg" : "qrc:/icons/connect.svg"
        }

        //
        // Spacer
        //
        Item {
            Layout.fillHeight: true
        }
    }
}
