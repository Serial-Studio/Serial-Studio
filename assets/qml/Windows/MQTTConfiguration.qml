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
import QtQuick.Window 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import Qt.labs.settings 1.0

import "../FramelessWindow" as FramelessWindow

FramelessWindow.CustomWindow {
    id: root

    //
    // Custom properties
    //
    readonly property int year: new Date().getFullYear()

    //
    // Window options
    //
    width: minimumWidth
    height: minimumHeight
    title: qsTr("MQTT Configuration")
    x: (Screen.desktopAvailableWidth - width) / 2
    y: (Screen.desktopAvailableHeight - height) / 2
    minimumWidth: column.implicitWidth + 4 * app.spacing + 2 * root.shadowMargin
    maximumWidth: column.implicitWidth + 4 * app.spacing + 2 * root.shadowMargin
    extraFlags: Qt.WindowStaysOnTopHint | Qt.Dialog | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
    minimumHeight: column.implicitHeight + 4 * app.spacing + titlebar.height + 2 * root.shadowMargin
    maximumHeight: column.implicitHeight + 4 * app.spacing + titlebar.height + 2 * root.shadowMargin

    //
    // Titlebar options
    //
    minimizeEnabled: false
    maximizeEnabled: false
    titlebarText: Cpp_ThemeManager.text
    titlebarColor: Cpp_ThemeManager.dialogBackground
    backgroundColor: Cpp_ThemeManager.dialogBackground

    //
    // Save settings
    //
    Settings {
        property alias version: _version.currentIndex
        property alias mode: _mode.currentIndex
        property alias host: _host.text
        property alias port: _port.text
        property alias qos: _qos.currentIndex
        property alias keepAlive: _keepAlive.text
        property alias topic: _topic.text
        property alias retain: _retain.checked
        property alias user: _user.text
        property alias password: _password.text
        property alias ssl: _ssl.checked
        property alias certificate: _certificateMode.currentIndex
        property alias protocol: _protocols.currentIndex
        property alias caFilePath: _caFile.text
    }

    //
    // Use page item to set application palette
    //
    Page {
        anchors {
            fill: parent
            margins: root.shadowMargin
            topMargin: titlebar.height + root.shadowMargin
        }

        palette.text: Cpp_ThemeManager.text
        palette.buttonText: Cpp_ThemeManager.text
        palette.windowText: Cpp_ThemeManager.text
        palette.window: Cpp_ThemeManager.dialogBackground
        background: Rectangle {
            radius: root.radius
            color: root.backgroundColor

            Rectangle {
                height: root.radius
                color: root.backgroundColor

                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }
            }
        }

        //
        // Window controls
        //
        ColumnLayout {
            id: column
            spacing: app.spacing
            anchors.centerIn: parent

            GridLayout {
                columns: 2
                rowSpacing: 0
                Layout.fillWidth: true
                columnSpacing: app.spacing

                //
                // Version & mode titles
                //
                Label {
                    text: qsTr("Version") + ":"
                } Label {
                    text: qsTr("Mode") + ":"
                }

                //
                // MQTT version
                //
                ComboBox {
                    id: _version
                    Layout.fillWidth: true
                    Layout.minimumWidth: 256
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
                ComboBox {
                    id: _mode
                    Layout.fillWidth: true
                    Layout.minimumWidth: 256
                    model: Cpp_MQTT_Client.clientModes
                    currentIndex: Cpp_MQTT_Client.clientMode
                    onCurrentIndexChanged: {
                        if (Cpp_MQTT_Client.clientMode !== currentIndex)
                            Cpp_MQTT_Client.clientMode = currentIndex
                    }
                }

                //
                // Spacers
                //
                Item {
                    height: app.spacing
                } Item {
                    height: app.spacing
                }

                //
                // QOS Level & Keep Alive
                //
                Label {
                    opacity: enabled ? 1 : 0.5
                    text: qsTr("QOS level") + ":"
                    enabled: !Cpp_MQTT_Client.isConnectedToHost
                } Label {
                    opacity: enabled ? 1 : 0.5
                    text: qsTr("Keep alive (s)") + ":"
                    enabled: !Cpp_MQTT_Client.isConnectedToHost
                }

                //
                // QOS
                //
                ComboBox {
                    id: _qos
                    Layout.fillWidth: true
                    opacity: enabled ? 1 : 0.5
                    model: Cpp_MQTT_Client.qosLevels
                    currentIndex: Cpp_MQTT_Client.qos
                    enabled: !Cpp_MQTT_Client.isConnectedToHost

                    onCurrentIndexChanged: {
                        if (Cpp_MQTT_Client.qos !== currentIndex)
                            Cpp_MQTT_Client.qos = currentIndex
                    }
                }

                //
                // Keep alive
                //
                TextField {
                    id: _keepAlive
                    Layout.fillWidth: true
                    opacity: enabled ? 1 : 0.5
                    placeholderText: Cpp_MQTT_Client.keepAlive
                    enabled: !Cpp_MQTT_Client.isConnectedToHost
                    Component.onCompleted: text = Cpp_MQTT_Client.keepAlive

                    onTextChanged: {
                        if (Cpp_MQTT_Client.keepAlive !== text && text.length > 0)
                            Cpp_MQTT_Client.keepAlive = text

                        if (text.length === 0)
                            Cpp_MQTT_Client.keepAlive = 1
                    }

                    validator: IntValidator {
                        bottom: 1
                        top: 65535
                    }
                }

                //
                // Spacers
                //
                Item {
                    height: app.spacing
                } Item {
                    height: app.spacing
                }

                //
                // Host & port titles
                //
                Label {
                    text: qsTr("Host") + ":"
                    opacity: enabled ? 1 : 0.5
                    enabled: !Cpp_MQTT_Client.isConnectedToHost
                } Label {
                    text: qsTr("Port") + ":"
                    opacity: enabled ? 1 : 0.5
                    enabled: !Cpp_MQTT_Client.isConnectedToHost
                }

                //
                // Host
                //
                TextField {
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
                TextField {
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
                // Spacers
                //
                Item {
                    height: app.spacing
                } Item {
                    height: app.spacing
                }

                //
                // Topic & retain labels
                //
                Label {
                    text: qsTr("Topic") + ":"
                    opacity: enabled ? 1 : 0.5
                    enabled: !Cpp_MQTT_Client.isConnectedToHost
                } Label {
                    text: qsTr("Retain") + ":"
                    opacity: enabled ? 1 : 0.5
                    enabled: !Cpp_MQTT_Client.isConnectedToHost
                }

                //
                // Topic
                //
                TextField {
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
                // Retain checkbox
                //
                CheckBox {
                    id: _retain
                    Layout.fillWidth: true
                    opacity: enabled ? 1 : 0.5
                    text: qsTr("Add retain flag")
                    Layout.leftMargin: -app.spacing
                    checked: Cpp_MQTT_Client.retain
                    enabled: !Cpp_MQTT_Client.isConnectedToHost

                    onCheckedChanged: {
                        if (Cpp_MQTT_Client.retain != checked)
                            Cpp_MQTT_Client.retain = checked
                    }
                }

                //
                // Spacers
                //
                Item {
                    height: app.spacing
                } Item {
                    height: app.spacing
                }

                //
                // Username & password titles
                //
                Label {
                    text: qsTr("User") + ":"
                    opacity: enabled ? 1 : 0.5
                    enabled: !Cpp_MQTT_Client.isConnectedToHost
                } Label {
                    text: qsTr("Password") + ":"
                    opacity: enabled ? 1 : 0.5
                    enabled: !Cpp_MQTT_Client.isConnectedToHost
                }

                //
                // Username
                //
                TextField {
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
                RowLayout {
                    Layout.fillWidth: true
                    spacing: app.spacing

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

                // Spacers
                //
                Item {
                    height: app.spacing
                } Item {
                    height: app.spacing
                }

                //
                // SSL & certificate titles
                //
                Label {
                    text: qsTr("Enable SSL/TLS:")
                    opacity: enabled ? 1 : 0.5
                    enabled: !Cpp_MQTT_Client.isConnectedToHost
                } Label {
                    text: qsTr("Certificate:")
                    opacity: enabled ? 1 : 0.5
                    enabled: !Cpp_MQTT_Client.isConnectedToHost && _ssl.checked
                }

                //
                // SSL/TLS switch
                //
                Switch {
                    id: _ssl
                    opacity: enabled ? 1 : 0.5
                    Layout.leftMargin: -app.spacing
                    checked: Cpp_MQTT_Client.sslEnabled
                    enabled: !Cpp_MQTT_Client.isConnectedToHost

                    onCheckedChanged: {
                        if (Cpp_MQTT_Client.sslEnabled !== checked)
                            Cpp_MQTT_Client.sslEnabled = checked
                    }
                }

                //
                // Certificate selection
                //
                RowLayout {
                    spacing: app.spacing
                    Layout.fillWidth: true

                    ComboBox {
                        id: _certificateMode
                        Layout.fillWidth: true
                        opacity: enabled ? 1 : 0.5
                        enabled: !Cpp_MQTT_Client.isConnectedToHost && _ssl.checked
                        model: [
                            qsTr("Use system database"),
                            qsTr("Custom CA file")
                        ]

                        onCurrentIndexChanged: {
                            if (currentIndex == 0)
                                Cpp_MQTT_Client.loadCaFile("")
                        }
                    }

                    Button {
                        icon.color: palette.text
                        opacity: enabled ? 1 : 0.5
                        Layout.maximumWidth: height
                        Layout.alignment: Qt.AlignVCenter
                        icon.source: "qrc:/icons/open.svg"
                        onClicked: Cpp_MQTT_Client.loadCaFile()
                        enabled: _certificateMode.currentIndex == 1  && _ssl.checked
                    }
                }

                //
                // Spacers
                //
                Item {
                    height: app.spacing
                } Item {
                    height: app.spacing
                }

                //
                // SSL/TLS protocol selection title
                //
                Item {
                    Layout.fillWidth: true
                } Label {
                    text: qsTr("Protocol:")
                    opacity: enabled ? 1 : 0.5
                    enabled: !Cpp_MQTT_Client.isConnectedToHost && _ssl.checked
                }

                //
                // SSL/TLS protocol selection
                //
                Item {
                    Layout.fillWidth: true
                } ComboBox {
                    id: _protocols
                    Layout.fillWidth: true
                    opacity: enabled ? 1 : 0.5
                    model: Cpp_MQTT_Client.sslProtocols
                    enabled: !Cpp_MQTT_Client.isConnectedToHost && _ssl.checked

                    onCurrentIndexChanged: {
                        if (currentIndex !== Cpp_MQTT_Client.sslProtocol)
                            Cpp_MQTT_Client.sslProtocol = currentIndex
                    }
                }
            }

            //
            // Spacer
            //
            Item {
                height: app.spacing
                visible: Cpp_MQTT_Client.caFilePath.length > 0
            }

            //
            // CA file display
            //
            RowLayout {
                spacing: app.spacing
                Layout.fillWidth: true
                visible: Cpp_MQTT_Client.caFilePath.length > 0

                Label {
                    font.bold: true
                    text: qsTr("CA file:")
                    Layout.alignment: Qt.AlignVCenter
                }

                Label {
                    id: _caFile
                    text: Cpp_MQTT_Client.caFilePath
                    onTextChanged: {
                        if (text !== Cpp_MQTT_Client.caFilePath)
                            Cpp_MQTT_Client.loadCaFile(text)
                    }
                }
            }

            //
            // Spacer
            //
            Item {
                height: app.spacing * 2
                visible: Cpp_MQTT_Client.caFilePath.length <= 0
            }

            //
            // Buttons
            //
            RowLayout {
                spacing: app.spacing
                Layout.fillWidth: true

                Button {
                    icon.width: 24
                    icon.height: 24
                    font.bold: true
                    Layout.fillWidth: true
                    icon.color: Cpp_ThemeManager.mqttButton
                    checked: Cpp_MQTT_Client.isConnectedToHost
                    onClicked: Cpp_MQTT_Client.toggleConnection()
                    palette.buttonText: Cpp_ThemeManager.mqttButton
                    text: (checked ? qsTr("Disconnect") : qsTr("Connect")) + "  "
                    icon.source: checked ? "qrc:/icons/disconnect.svg" :
                                           "qrc:/icons/connect.svg"
                }

                Button {
                    icon.width: 24
                    icon.height: 24
                    Layout.fillWidth: true
                    onClicked: root.close()
                    text: qsTr("Apply") + "  "
                    icon.color: palette.buttonText
                    icon.source: "qrc:/icons/apply.svg"
                }
            }
        }
    }
}
