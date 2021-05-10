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
    // Access to properties
    //
    property alias endSequence: _endSequence.text
    property alias language: _langCombo.currentIndex
    property alias startSequence: _startSequence.text
    property alias tcpPlugins: _tcpPlugins.checked

    //
    // Layout
    //
    ColumnLayout {
        id: layout
        anchors.fill: parent
        anchors.margins: app.spacing

        //
        // Controls
        //
        GridLayout {
            columns: 2
            Layout.fillWidth: true
            rowSpacing: app.spacing
            columnSpacing: app.spacing

            //
            // Language selector
            //
            Label {
                text: qsTr("Language") + ":"
            } ComboBox {
                id: _langCombo
                Layout.fillWidth: true
                model: Cpp_Misc_Translator.availableLanguages
                onCurrentIndexChanged: Cpp_Misc_Translator.setLanguage(currentIndex)
            }

            //
            // Start sequence
            //
            Label {
                text: qsTr("Start sequence") + ": "
            } TextField {
                id: _startSequence
                Layout.fillWidth: true
                placeholderText: "/*"
                text: "/*"
                onTextChanged: {
                    if (text !== Cpp_IO_Manager.startSequence)
                        Cpp_IO_Manager.startSequence = text
                }
            }

            //
            // End sequence
            //
            Label {
                text: qsTr("End sequence") + ": "
            } TextField {
                id: _endSequence
                Layout.fillWidth: true
                placeholderText: "*/"
                text: "*/"
                onTextChanged: {
                    if (text !== Cpp_IO_Manager.finishSequence)
                        Cpp_IO_Manager.finishSequence = text
                }
            }

            //
            // Plugins enabled
            //
            Label {
                text: qsTr("Plugin system") + ": "
            } Switch {
                id: _tcpPlugins
                Layout.leftMargin: -app.spacing
                Layout.alignment: Qt.AlignLeft
                checked: Cpp_Plugins_Bridge.enabled
                onCheckedChanged: {
                    if (checked !== Cpp_Plugins_Bridge.enabled)
                        Cpp_Plugins_Bridge.enabled = checked
                }
            }
        }

        //
        // Plugins label
        //
        Label {
            opacity: 0.8
            font.pixelSize: 12
            Layout.fillWidth: true
            color: palette.highlightedText
            wrapMode: Label.WrapAtWordBoundaryOrAnywhere
            text: qsTr("Applications/plugins can interact with %1 by " +
                       "establishing a TCP connection on port 7777.").arg(Cpp_AppName)
        }

        //
        // Vertical spacer
        //
        Item {
            Layout.fillHeight: true
        }
    }
}
