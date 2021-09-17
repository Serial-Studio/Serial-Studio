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
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import Qt.labs.settings 1.0

Window {
    id: root

    //
    // Window options
    //
    title: qsTr("JSON Editor")
    minimumWidth: column.implicitWidth + 4 * app.spacing
    minimumHeight: column.implicitHeight + 4 * app.spacing

    //
    // Save window size
    //
    Settings {
        category: "JSONEditor"
        property alias windowX: root.x
        property alias windowY: root.y
        property alias windowWidth: root.width
        property alias windowHeight: root.height
    }

    //
    // Use page item to set application palette
    //
    Page {
        anchors.margins: 0
        anchors.fill: parent
        palette.text: Cpp_ThemeManager.text
        palette.buttonText: Cpp_ThemeManager.text
        palette.windowText: Cpp_ThemeManager.text
        palette.window: Cpp_ThemeManager.dialogBackground

        //
        // Window controls
        //
        ColumnLayout {
            id: column
            spacing: 0
            anchors.fill: parent
            anchors.margins: app.spacing * 2

            //
            // Project title
            //
            RowLayout {
                spacing: app.spacing
                Layout.fillWidth: true

                ToolButton {
                    flat: true
                    enabled: false
                    icon.width: 24
                    icon.height: 24
                    icon.color: Cpp_ThemeManager.text
                    icon.source: "qrc:/icons/registration.svg"
                }

                TextField {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 320
                    placeholderText: qsTr("Project title (required)")
                }
            }

            //
            // Separator character
            //
            RowLayout {
                spacing: app.spacing
                Layout.fillWidth: true

                ToolButton {
                    flat: true
                    enabled: false
                    icon.width: 24
                    icon.height: 24
                    icon.color: Cpp_ThemeManager.text
                    icon.source: "qrc:/icons/separator.svg"
                }

                TextField {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 420
                    placeholderText: qsTr("Data separator (default is ',')")
                }
            }

            //
            // Start sequence
            //
            RowLayout {
                spacing: app.spacing
                Layout.fillWidth: true

                ToolButton {
                    flat: true
                    enabled: false
                    icon.width: 24
                    icon.height: 24
                    icon.color: Cpp_ThemeManager.text
                    icon.source: "qrc:/icons/start-sequence.svg"
                }

                TextField {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 256
                    placeholderText: qsTr("Frame start sequence (default is '%1')").arg(Cpp_IO_Manager.startSequence)
                }
            }

            //
            // End sequence
            //
            RowLayout {
                spacing: app.spacing
                Layout.fillWidth: true

                ToolButton {
                    flat: true
                    enabled: false
                    icon.width: 24
                    icon.height: 24
                    icon.color: Cpp_ThemeManager.text
                    icon.source: "qrc:/icons/end-sequence.svg"
                }

                TextField {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 256
                    placeholderText: qsTr("Frame end sequence (default is '%1')").arg(Cpp_IO_Manager.finishSequence)
                }
            }

            //
            // Spacer
            //
            Item {
                height: app.spacing
            }

            //
            // List view
            //
            TextField {
                clip: true
                readOnly: true
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 320
                horizontalAlignment: Text.AlignHCenter
                wrapMode: TextField.WrapAtWordBoundaryOrAnywhere
                placeholderText: view.model === 0 ? qsTr("Click on the \"Add group\" button to begin.") : ""

                ListView {
                    id: view
                    model: 0
                    anchors.fill: parent
                    anchors.bottomMargin: app.spacing

                    ScrollBar.vertical: ScrollBar {
                        id: scroll
                        policy: ScrollBar.AsNeeded
                    }

                    delegate: Item {
                        x: (parent.width - width) / 2
                        height: group.height + app.spacing
                        width: parent.width - 2 * app.spacing

                        JsonGroupDelegate {
                            id: group
                            groupIndex: index
                            totalGroupCount: view.model
                            anchors {
                                left: parent.left
                                right: parent.right
                                bottom: parent.bottom
                            }
                        }
                    }
                }
            }

            //
            // Spacer
            //
            Item {
                height: app.spacing
            }

            //
            // Add group button
            //
            Button {
                icon.width: 24
                icon.height: 24
                Layout.fillWidth: true
                text: qsTr("Add group")
                icon.source: "qrc:/icons/add.svg"
                icon.color: Cpp_ThemeManager.text
                onClicked: {
                    view.model = view.model + 1
                    scroll.position = 1
                }
            }

            //
            // Spacer
            //
            Item {
                height: app.spacing * 3
            }

            //
            // Dialog buttons
            //
            RowLayout {
                spacing: app.spacing
                Layout.fillWidth: true

                Button {
                    text: qsTr("Close")
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTr("Open")
                }

                Button {
                    text: qsTr("Save")
                }
            }
        }
    }
}
