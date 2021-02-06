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

import SerialStudio 1.0
import Qt.labs.settings 1.0

import "../Widgets" as Widgets

Control {
    id: root
    background: Rectangle {
        color: app.windowBackgroundColor
    }

    //
    // Function to send through serial port data
    //
    function sendData() {
        Cpp_IO_Console.send(send.text)
        send.clear()
    }

    //
    // Save settings
    //
    Settings {
        property alias echo: echoCheck.checked
        property alias hex: hexCheckbox.checked
        property alias timestamp: timestampCheck.checked
        property alias autoscroll: autoscrollCheck.checked
        property alias lineEnding: lineEndingCombo.currentIndex
        property alias displayMode: displayModeCombo.currentIndex
    }

    //
    // Copy shortcut
    //
    Shortcut {
        sequence: StandardKey.Copy
        onActivated: Cpp_IO_Console.copy(logView.selectedText)
    }

    //
    // Copy shortcut
    //
    Shortcut {
        sequence: StandardKey.Save
        onActivated: Cpp_IO_Console.save()
    }

    //
    // Right-click context menu
    //
    Menu {
        id: menu

        MenuItem {
            text: qsTr("Copy")
            enabled: logView.selectedText.length > 0
            onClicked: Cpp_IO_Console.copy(logView.selectedText)
        }

        MenuItem {
            text: qsTr("Clear")
            opacity: enabled ? 1 : 0.5
            onTriggered: Cpp_IO_Console.clear()
            enabled: Cpp_IO_Console.saveAvailable
        }

        MenuItem {
            opacity: enabled ? 1 : 0.5
            text: qsTr("Save as") + "..."
            onTriggered: Cpp_IO_Console.save()
            enabled: Cpp_IO_Console.saveAvailable
        }
    }

    //
    // Controls
    //
    ColumnLayout {
        anchors.fill: parent
        spacing: app.spacing
        anchors.margins: app.spacing * 1.5

        //
        // Console display
        //
        Widgets.LogView {
            id: logView
            border.width: 1
            contextMenu: menu
            font.pixelSize: 12
            Layout.fillWidth: true
            border.color: caretLineColor
            Layout.fillHeight: true
            font.family: app.monoFont
            model: Cpp_IO_Console.dataModel
            lineOffset: Cpp_IO_Console.lineOffset
            autoscroll: Cpp_IO_Console.autoscroll
            placeholderText: qsTr("No data received so far...")
        }

        //
        // Data-write controls
        //
        RowLayout {
            Layout.fillWidth: true

            TextField {
                id: send
                height: 24
                font: logView.font
                Layout.fillWidth: true
                color: logView.textColor
                opacity: enabled ? 1 : 0.5
                enabled: Cpp_IO_Manager.readWrite
                palette.base: logView.backgroundColor
                placeholderText: qsTr("Send data to device") + "..."

                background: Rectangle {
                    border.width: 1
                    color: logView.backgroundColor
                    border.color: logView.border.color
                }

                //
                // Validate hex strings
                //
                validator: RegExpValidator {
                    regExp: hexCheckbox.checked ? /^[a-fA-F0-9]+$/ : /[\s\S]*/
                }

                //
                // Send data on <enter>
                //
                Keys.onReturnPressed: root.sendData()

                //
                // Navigate command history upwards with <up>
                //
                Keys.onUpPressed: {
                    Cpp_IO_Console.historyUp()
                    send.text = Cpp_IO_Console.currentHistoryString
                }

                //
                // Navigate command history downwards with <down>
                //
                Keys.onDownPressed: {
                    Cpp_IO_Console.historyDown()
                    send.text = Cpp_IO_Console.currentHistoryString
                }
            }

            CheckBox {
                id: hexCheckbox
                text: "Hex"
                opacity: enabled ? 1 : 0.5
                enabled: Cpp_IO_Manager.readWrite
                checked: Cpp_IO_Console.dataMode === 1
                onCheckedChanged: {
                    if (checked)
                        Cpp_IO_Console.dataMode = 1
                    else
                        Cpp_IO_Console.dataMode = 0
                }
            }

            CheckBox {
                id: echoCheck
                text: qsTr("Echo")
                Layout.alignment: Qt.AlignVCenter
                checked: Cpp_IO_Console.echo
                onCheckedChanged: {
                    if (Cpp_IO_Console.echo != checked)
                        Cpp_IO_Console.echo = checked
                }
            }
        }

        //
        // Terminal output options
        //
        RowLayout {
            Layout.fillWidth: true

            CheckBox {
                id: autoscrollCheck
                text: qsTr("Autoscroll")
                Layout.alignment: Qt.AlignVCenter
                checked: Cpp_IO_Console.autoscroll
                onCheckedChanged: {
                    if (Cpp_IO_Console.autoscroll != checked)
                        Cpp_IO_Console.autoscroll = checked
                }
            }

            CheckBox {
                id: timestampCheck
                text: qsTr("Show timestamp")
                Layout.alignment: Qt.AlignVCenter
                checked: Cpp_IO_Console.showTimestamp
                onCheckedChanged: {
                    if (Cpp_IO_Console.showTimestamp != checked)
                        Cpp_IO_Console.showTimestamp = checked
                }
            }

            Item {
                Layout.fillWidth: true
            }

            ComboBox {
                id: lineEndingCombo
                Layout.alignment: Qt.AlignVCenter
                model: Cpp_IO_Console.lineEndings()
                currentIndex: Cpp_IO_Console.lineEnding
                onCurrentIndexChanged: {
                    if (currentIndex != Cpp_IO_Console.lineEnding)
                        Cpp_IO_Console.lineEnding = currentIndex
                }
            }

            ComboBox {
                id: displayModeCombo
                Layout.alignment: Qt.AlignVCenter
                model: Cpp_IO_Console.displayModes()
                currentIndex: Cpp_IO_Console.displayMode
                onCurrentIndexChanged: {
                    if (currentIndex != Cpp_IO_Console.displayMode)
                        Cpp_IO_Console.displayMode = currentIndex
                }
            }

            Button {
                height: 24
                Layout.maximumWidth: 32
                icon.color: palette.text
                opacity: enabled ? 1 : 0.5
                onClicked: Cpp_IO_Console.save()
                icon.source: "qrc:/icons/save.svg"
                enabled: Cpp_IO_Console.saveAvailable
                Behavior on opacity {NumberAnimation{}}
            }

            Button {
                height: 24
                Layout.maximumWidth: 32
                icon.color: palette.text
                opacity: enabled ? 1 : 0.5
                onClicked: Cpp_IO_Console.clear()
                icon.source: "qrc:/icons/delete.svg"
                enabled: Cpp_IO_Console.lineCount > 0
                Behavior on opacity {NumberAnimation{}}
            }
        }
    }
}
