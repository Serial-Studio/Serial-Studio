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

import "../Widgets" as Widgets

Control {
    id: root
    background: Rectangle {
        color: app.windowBackgroundColor
    }

    //
    // Console text color
    //
    property alias text: _console.text
    readonly property color consoleColor: "#8ecd9d"
    
    //
    // Function to send through serial port data
    //
    function sendData(data) {
        CppSerialManager.sendData(data)
        root.text += "\n\n"
        root.text += data
        root.text += "\n\n"
        _tf.clear()
    }

    //
    // Connections with serial manager
    //
    Connections {
        target: CppSerialManager
        function onRx(rxData) {
            // Add data to console buffer
            root.text += rxData

            // Ensure that console line count stays in check
            if (_console.height > _scrollView.height)
                root.text = root.text.substring(rxData.length)

            // Scroll to bottom
            if (_scrollView.contentHeight > _scrollView.height)
                _console.cursorPosition = _console.length - 1
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
        // Console display (HACK: we use the scrollview to know if there is text that is
        // no longer visible in the console buffer)
        //
        TextField {
            id: _cont
            readOnly: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            palette.base: "#121218"

            ScrollView {
                id: _scrollView
                anchors.fill: parent
                contentWidth: parent.width
                ScrollBar.vertical.policy: ScrollBar.AlwaysOff
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                TextArea {
                    id: _console
                    readOnly: true
                    font.pixelSize: 12
                    cursorVisible: true
                    color: root.consoleColor
                    wrapMode: TextEdit.Wrap
                    font.family: app.monoFont
                    width: _scrollView.contentWidth
                    placeholderText: qsTr("No data received so far...") + CppTranslator.dummy
                    Component.onCompleted: CppSerialManager.configureTextDocument(textDocument)
                }
            }
        }

        //
        // Data-write controls
        //
        RowLayout {
            Layout.fillWidth: true

            TextField {
                id: _tf
                height: 24
                font.pixelSize: 12
                Layout.fillWidth: true
                palette.base: "#121218"
                color: root.consoleColor
                font.family: app.monoFont
                opacity: enabled ? 1 : 0.5
                enabled: CppSerialManager.readWrite
                Keys.onReturnPressed: root.sendData(_tf.text)
                placeholderText: qsTr("Send data to device") + "..." + CppTranslator.dummy

                Behavior on opacity {NumberAnimation{}}
            }

            Button {
                height: 24
                icon.color: palette.text
                opacity: enabled ? 1 : 0.5
                icon.source: "qrc:/icons/send.svg"
                onClicked: root.sendData(_tf.text)
                enabled: CppSerialManager.readWrite
                Behavior on opacity {NumberAnimation{}}
            }

            Button {
                height: 24
                icon.color: palette.text
                opacity: enabled ? 1 : 0.5
                onClicked: _console.clear()
                enabled: _console.length > 0
                icon.source: "qrc:/icons/delete.svg"
                Behavior on opacity {NumberAnimation{}}
            }
        }
    }
}
