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
    property alias version: _version.currentIndex
    property alias mode: _mode.currentIndex
    
    //
    // React to events from MQTT module
    //
    Connections {
        target: Cpp_MQTT_Client
        
        function onHostChanged() {
            if (_host.text != "")
                _host.text = Cpp_MQTT_Client.host
        }
        
        function onPortChanged() {
        	if (_port.text != "")
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
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Client.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
                text: qsTr("Host") + ":"
            } TextField {
                id: _host
                Layout.fillWidth: true
                placeholderText: Cpp_MQTT_Client.defaultHost
                Component.onCompleted: text = Cpp_MQTT_Client.host
                onTextChanged: {
                    if (Cpp_MQTT_Client.host !== text && text != "")
                        Cpp_MQTT_Client.host = text

                    if (text == "")
                        Cpp_MQTT_Client.host = Cpp_MQTT_Client.defaultHost
                }

                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Client.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
            }

            //
            // Port
            //
            Label {
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Client.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
                text: qsTr("Port") + ":"
            } TextField {
                id: _port
                Layout.fillWidth: true
                placeholderText: Cpp_MQTT_Client.defaultPort
                Component.onCompleted: text = Cpp_MQTT_Client.port
                onTextChanged: {
                    if (Cpp_MQTT_Client.port !== text && text != "")
                        Cpp_MQTT_Client.port = text

                    if (text == "")
                        Cpp_MQTT_Client.port = Cpp_MQTT_Client.defaultPort
                }

                validator: IntValidator {
                    bottom: 0
                    top: 65535
                }

                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Client.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
            }

            //
            // Topic
            //
            Label {
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Client.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
                text: qsTr("Topic") + ":"
            } TextField {
                id: _topic
                Layout.fillWidth: true
                text: Cpp_MQTT_Client.topic
                placeholderText: qsTr("MQTT topic")
                onTextChanged: {
                    if (Cpp_MQTT_Client.topic !== text)
                        Cpp_MQTT_Client.topic = text
                }

                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Client.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
            }

            //
            // Username
            //
            Label {
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Client.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
                text: qsTr("User") + ":"
            } TextField {
                id: _user
                Layout.fillWidth: true
                text: Cpp_MQTT_Client.username
                placeholderText: qsTr("MQTT username")
                onTextChanged: {
                    if (Cpp_MQTT_Client.username !== text)
                        Cpp_MQTT_Client.username = text
                }

                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Client.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
            }

            //
            // Password
            //
            Label {
                opacity: enabled ? 1 : 0.5
                enabled: !Cpp_MQTT_Client.isConnectedToHost
                Behavior on opacity {NumberAnimation{}}
                text: qsTr("Password") + ":"
            } RowLayout {
                Layout.fillWidth: true
                spacing: app.spacing / 2

                TextField {
                    id: _password
                    Layout.fillWidth: true
                    echoMode: TextField.Password
                    text: Cpp_MQTT_Client.password
                    placeholderText: qsTr("MQTT password")
                    onTextChanged: {
                        if (Cpp_MQTT_Client.password !== text)
                            Cpp_MQTT_Client.password = text
                    }

                    opacity: enabled ? 1 : 0.5
                    enabled: !Cpp_MQTT_Client.isConnectedToHost
                    Behavior on opacity {NumberAnimation{}}
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
        // Connect/disconnect button
        //
        Button {
            icon.width: 24
            icon.height: 24
            font.bold: true
            Layout.fillWidth: true
            icon.color: palette.buttonText
            checked: Cpp_MQTT_Client.isConnectedToHost
            onClicked: Cpp_MQTT_Client.toggleConnection()
            text: (checked ? qsTr("Disconnect") :
                             qsTr("Connect")) + "  "
            icon.source: checked ? "qrc:/icons/disconnect.svg" :
                                   "qrc:/icons/connect.svg"
            palette.buttonText: checked ? Cpp_ThemeManager.connectButtonChecked :
                                          Cpp_ThemeManager.connectButtonUnchecked
        }

        //
        // Spacer
        //
        Item {
            Layout.fillHeight: true
        }
    }
}
