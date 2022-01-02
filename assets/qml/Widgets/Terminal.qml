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

import Qt.labs.settings 1.0
import SerialStudio 1.0 as SerialStudio

Item {
    id: root
    property bool isExternalWindow: false
    property alias widgetEnabled: textEdit.widgetEnabled
    property alias vt100emulation: textEdit.vt100emulation

    //
    // Save settings
    //
    Settings {
        property alias hex: hexCheckbox.checked
        property alias echo: echoCheckbox.checked
        property alias timestamp: timestampCheck.checked
        property alias autoscroll: autoscrollCheck.checked
        property alias vt100Enabled: textEdit.vt100emulation
        property alias lineEnding: lineEndingCombo.currentIndex
        property alias displayMode: displayModeCombo.currentIndex
    }

    //
    // Function to send through serial port data
    //
    function sendData() {
        Cpp_IO_Console.send(send.text)
        send.clear()
    }

    //
    // Clears console output
    //
    function clear() {
        Cpp_IO_Console.clear()
        textEdit.clear()
    }

    //
    // Copy function
    //
    function copy() {
        textEdit.copy()
    }

    //
    // Select all text
    //
    function selectAll() {
        textEdit.selectAll()
    }

    //
    // Right-click context menu
    //
    Menu {
        id: contextMenu

        MenuItem {
            id: copyMenu
            text: qsTr("Copy")
            opacity: enabled ? 1 : 0.5
            onClicked: textEdit.copy()
            enabled: textEdit.copyAvailable
        }

        MenuItem {
            text: qsTr("Select all")
            enabled: !textEdit.empty
            opacity: enabled ? 1 : 0.5
            onTriggered: textEdit.selectAll()
        }

        MenuItem {
            text: qsTr("Clear")
            opacity: enabled ? 1 : 0.5
            onTriggered: root.clear()
            enabled: Cpp_IO_Console.saveAvailable
        }

        MenuSeparator {}

        MenuItem {
            opacity: enabled ? 1 : 0.5
            text: qsTr("Print")
            enabled: Cpp_IO_Console.saveAvailable
            onTriggered: Cpp_IO_Console.print(app.monoFont)
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
        SerialStudio.Terminal {
            id: textEdit
            focus: true
            readOnly: true
            font.pixelSize: 12
            vt100emulation: true
            centerOnScroll: false
            undoRedoEnabled: false
            Layout.fillWidth: true
            Layout.fillHeight: true
            maximumBlockCount: 12000
            font.family: app.monoFont
            autoscroll: Cpp_IO_Console.autoscroll
            renderTarget: PaintedItem.FramebufferObject
            wordWrapMode: Text.WrapAtWordBoundaryOrAnywhere
            placeholderText: qsTr("No data received so far") + "..."

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                cursorShape: Qt.IBeamCursor
                propagateComposedEvents: true
                acceptedButtons: Qt.RightButton
                anchors.rightMargin: textEdit.scrollbarWidth

                onClicked: {
                    if (mouse.button === Qt.RightButton) {
                        contextMenu.popup()
                        mouse.accepted = true
                    }
                }
            }
        }

        //
        // Data-write controls
        //
        RowLayout {
            Layout.fillWidth: true

            TextField {
                id: send
                height: 24
                font: textEdit.font
                Layout.fillWidth: true
                enabled: Cpp_IO_Manager.readWrite
                palette.text: Cpp_ThemeManager.consoleText
                palette.base: Cpp_ThemeManager.consoleBase
                placeholderText: qsTr("Send data to device") + "..."
                Component.onCompleted: {
                    if (Cpp_Qt6)
                        placeholderTextColor = Cpp_ThemeManager.consolePlaceholderText
                }

                //
                // Validate hex strings
                //
                //validator: RegExpValidator {
                //    regExp: hexCheckbox.checked ? /^(?:([a-f0-9]{2})\s*)+$/i : /[\s\S]*/
                //}

                //
                // Send data on <enter>
                //
                Keys.onReturnPressed: root.sendData()

                //
                // Add space automatically in hex view
                //
                onTextChanged: {
                    if (hexCheckbox.checked)
                        send.text = Cpp_IO_Console.formatUserHex(send.text)
                }

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
                text: "HEX"
                id: hexCheckbox
                opacity: enabled ? 1 : 0.5
                enabled: Cpp_IO_Manager.readWrite
                checked: Cpp_IO_Console.dataMode === 1
                onCheckedChanged: Cpp_IO_Console.dataMode = checked ? 1 : 0
            }

            CheckBox {
                visible: false
                text: qsTr("Echo")
                id: echoCheckbox
                opacity: enabled ? 1 : 0.5
                checked: Cpp_IO_Console.echo
                enabled: Cpp_IO_Manager.readWrite
                onCheckedChanged: {
                    if (Cpp_IO_Console.echo !== checked)
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
                    if (Cpp_IO_Console.autoscroll !== checked)
                        Cpp_IO_Console.autoscroll = checked
                }
            }

            CheckBox {
                id: timestampCheck
                text: qsTr("Show timestamp")
                Layout.alignment: Qt.AlignVCenter
                checked: Cpp_IO_Console.showTimestamp
                onCheckedChanged: {
                    if (Cpp_IO_Console.showTimestamp !== checked)
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
                    if (currentIndex !== Cpp_IO_Console.lineEnding)
                        Cpp_IO_Console.lineEnding = currentIndex
                }
            }

            ComboBox {
                id: displayModeCombo
                Layout.alignment: Qt.AlignVCenter
                model: Cpp_IO_Console.displayModes()
                currentIndex: Cpp_IO_Console.displayMode
                onCurrentIndexChanged: {
                    if (currentIndex !== Cpp_IO_Console.displayMode)
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
            }

            Button {
                height: 24
                Layout.maximumWidth: 32
                icon.color: palette.text
                opacity: enabled ? 1 : 0.5
                onClicked: root.clear()
                icon.source: "qrc:/icons/delete.svg"
                enabled: Cpp_IO_Console.saveAvailable
            }
        }
    }
}
