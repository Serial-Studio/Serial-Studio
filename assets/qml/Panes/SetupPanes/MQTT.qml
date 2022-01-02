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

import "../../Windows" as Windows

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
    property alias version: _version.currentIndex
    property alias mode: _mode.currentIndex
    
    //
    // React to events from MQTT module
    //
    Connections {
        target: Cpp_MQTT_Client
        
        function onHostChanged() {
            if (_host.text.length > 0)
                _host.text = Cpp_MQTT_Client.host
        }
        
        function onPortChanged() {
            if (_port.text.length > 0)
                _port.text = Cpp_MQTT_Client.port
        }
    }

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
                id: _version
                Layout.fillWidth: true
                model: Cpp_MQTT_Client.mqttVersions
                currentIndex: Cpp_MQTT_Client.mqttVersion
                onCurrentIndexChanged: {
                    if (Cpp_MQTT_Client.mqttVersion !== currentIndex)
                        Cpp_MQTT_Client.mqttVersion = currentIndex
                }
            }

            //
            // Client mode version
            //
            Label {
                text: qsTr("Mode") + ":"
            } ComboBox {
                id: _mode
                Layout.fillWidth: true
                model: Cpp_MQTT_Client.clientModes
                currentIndex: Cpp_MQTT_Client.clientMode
                onCurrentIndexChanged: {
                    if (Cpp_MQTT_Client.clientMode !== currentIndex)
                        Cpp_MQTT_Client.clientMode = currentIndex
                }
            }

            //
            // Host
            //
            Label {
                text: qsTr("Host") + ":"
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Client.isConnectedToHost
            } TextField {
                id: _host
                Layout.fillWidth: true
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Client.isConnectedToHost
                placeholderText: Cpp_MQTT_Client.defaultHost
                Component.onCompleted: text = Cpp_MQTT_Client.host

                onTextChanged: {
                    if (Cpp_MQTT_Client.host !== text && text.length > 0)
                        Cpp_MQTT_Client.host = text

                    if (text.length === 0)
                        Cpp_MQTT_Client.host = Cpp_MQTT_Client.defaultHost
                }
            }

            //
            // Port
            //
            Label {
                text: qsTr("Port") + ":"
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Client.isConnectedToHost
            } TextField {
                id: _port
                Layout.fillWidth: true
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Client.isConnectedToHost
                placeholderText: Cpp_MQTT_Client.defaultPort
                Component.onCompleted: text = Cpp_MQTT_Client.port
                onTextChanged: {
                    if (Cpp_MQTT_Client.port !== text && text.length > 0)
                        Cpp_MQTT_Client.port = text

                    if (text.length === 0)
                        Cpp_MQTT_Client.port = Cpp_MQTT_Client.defaultPort
                }

                validator: IntValidator {
                    bottom: 0
                    top: 65535
                }
            }

            //
            // Topic
            //
            Label {
                text: qsTr("Topic") + ":"
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Client.isConnectedToHost
            } TextField {
                id: _topic
                Layout.fillWidth: true
                opacity: enabled ? 1 : 0.5
                text: Cpp_MQTT_Client.topic
                placeholderText: qsTr("MQTT topic")
                enabled: !Cpp_MQTT_Client.isConnectedToHost

                onTextChanged: {
                    if (Cpp_MQTT_Client.topic !== text)
                        Cpp_MQTT_Client.topic = text
                }
            }

            //
            // Username
            //
            Label {
                text: qsTr("User") + ":"
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Client.isConnectedToHost
            } TextField {
                id: _user
                Layout.fillWidth: true
                opacity: enabled ? 1 : 0.5
                text: Cpp_MQTT_Client.username
                placeholderText: qsTr("MQTT username")
                enabled: !Cpp_MQTT_Client.isConnectedToHost

                onTextChanged: {
                    if (Cpp_MQTT_Client.username !== text)
                        Cpp_MQTT_Client.username = text
                }
            }

            //
            // Password
            //
            Label {
                opacity: enabled ? 1 : 0.5
                text: qsTr("Password") + ":"
                enabled: !Cpp_MQTT_Client.isConnectedToHost
            } RowLayout {
                Layout.fillWidth: true
                spacing: app.spacing / 2

                TextField {
                    id: _password
                    Layout.fillWidth: true
                    opacity: enabled ? 1 : 0.5
                    echoMode: TextField.Password
                    text: Cpp_MQTT_Client.password
                    placeholderText: qsTr("MQTT password")
                    enabled: !Cpp_MQTT_Client.isConnectedToHost

                    onTextChanged: {
                        if (Cpp_MQTT_Client.password !== text)
                            Cpp_MQTT_Client.password = text
                    }
                }

                Button {
                    checkable: true
                    icon.color: palette.text
                    Layout.maximumWidth: height
                    Layout.alignment: Qt.AlignVCenter
                    icon.source: "qrc:/icons/visibility.svg"
                    onCheckedChanged: _password.echoMode = (checked ? TextField.Normal :
                                                                      TextField.Password)
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
        // Spacer
        //
        Item {
            Layout.fillHeight: true
        }

        //
        // Advanced setup & connect/disconnect button
        //
        RowLayout {
            spacing: app.spacing
            Layout.fillWidth: true

            Button {
                icon.width: 24
                icon.height: 24
                Layout.fillWidth: true
                onClicked: mqttSetup.show()
                icon.color: palette.buttonText
                text: qsTr("Advanced setup") + "  "
                icon.source: "qrc:/icons/settings.svg"
            }

            Button {
                icon.width: 24
                icon.height: 24
                font.bold: true
                Layout.fillWidth: true
                icon.color: Cpp_ThemeManager.mqttButton
                checked: Cpp_MQTT_Client.isConnectedToHost
                palette.buttonText: Cpp_ThemeManager.mqttButton
                onClicked: Cpp_MQTT_Client.toggleConnection()
                text: (checked ? qsTr("Disconnect") :
                                 qsTr("Connect to broker")) + "  "
                icon.source: checked ? "qrc:/icons/disconnect.svg" :
                                       "qrc:/icons/connect.svg"
            }
        }

        //
        // Spacer
        //
        Item {
            Layout.fillHeight: true
        }
    }

    //
    // MQTT setup dialog
    //
    Windows.MQTTConfiguration {
        id: mqttSetup
    }
}
