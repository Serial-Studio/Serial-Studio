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

import "../JsonEditor"
import "../Widgets" as Widgets
import "../FramelessWindow" as FramelessWindow

FramelessWindow.CustomWindow {
    id: root

    //
    // Window options
    //
    minimumWidth: 910
    minimumHeight: 720
    title: qsTr("JSON Editor - %1").arg(Cpp_JSON_Editor.jsonFileName)

    //
    // Customize window border
    //
    borderWidth: 1
    borderColor: Qt.darker(Cpp_ThemeManager.toolbarGradient2, 1.5)

    //
    // Ensure that current JSON file is shown
    //
    Component.onCompleted: Cpp_JSON_Editor.openJsonFile(Cpp_JSON_Generator.jsonMapFilepath)

    //
    // Ask user to save changes before closing the window
    //
    //onClosing: (close) => close.accepted = Cpp_JSON_Editor.askSave()

    //
    // Dummy string to increase width of buttons
    //
    readonly property string _btSpacer: "  "

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
        clip: true
        anchors.fill: parent
        anchors.margins: root.shadowMargin
        palette.text: Cpp_ThemeManager.text
        palette.buttonText: Cpp_ThemeManager.text
        palette.windowText: Cpp_ThemeManager.text
        anchors.topMargin: titlebar.height + root.shadowMargin
        palette.window: Cpp_ThemeManager.dialogBackground

        background: Rectangle {
            radius: root.radius
            color: Cpp_ThemeManager.windowBackground
        }

        //
        // Shadows
        //
        Widgets.Shadow {
            anchors.fill: header
        } Widgets.Shadow {
            anchors.fill: footer
            anchors.bottomMargin: footer.height / 2
        }

        //
        // Header (project properties)
        //
        Header {
            id: header
            anchors {
                margins: 0
                top: parent.top
                left: parent.left
                right: parent.right
            }
        }

        //
        // Footer background
        //
        Footer {
            id: footer
            radius: root.radius
            onCloseWindow: root.close()
            onScrollToBottom: groupEditor.scrollToBottom()

            anchors {
                margins: 0
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
        }

        //
        // Window controls
        //
        RowLayout {
            clip: true
            anchors.fill: parent
            spacing: app.spacing
            anchors.topMargin: header.height
            anchors.bottomMargin: footer.height

            //
            // Horizontal spacer
            //
            Item {
                Layout.fillHeight: true
                Layout.minimumWidth: app.spacing
                Layout.maximumWidth: app.spacing
            }

            //
            // JSON structure tree
            //
            TreeView {
                id: jsonTree
                Layout.fillHeight: true
                Layout.minimumWidth: 240
                Layout.maximumWidth: 240
                Layout.topMargin: app.spacing * 2
                Layout.bottomMargin: app.spacing * 2
                visible: Cpp_JSON_Editor.groupCount !== 0
            }

            //
            // Group editor
            //
            GroupEditor {
                id: groupEditor
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: Cpp_JSON_Editor.groupCount !== 0
            }

            //
            // Empty project text & icon
            //
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: Cpp_JSON_Editor.groupCount === 0

                ColumnLayout {
                    spacing: app.spacing
                    anchors.centerIn: parent

                    Widgets.Icon {
                        width: 128
                        height: 128
                        color: Cpp_ThemeManager.text
                        Layout.alignment: Qt.AlignHCenter
                        source: "qrc:/icons/developer-board.svg"
                    }

                    Label {
                        font.bold: true
                        font.pixelSize: 24
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Start something awesome")
                    }

                    Label {
                        opacity: 0.8
                        font.pixelSize: 18
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Click on the \"Add group\" button to begin")
                    }
                }
            }

            //
            // Horizontal spacer
            //
            Item {
                Layout.fillHeight: true
                Layout.minimumWidth: app.spacing
                Layout.maximumWidth: app.spacing
            }
        }
    }

    //
    // Resize handler
    //
    FramelessWindow.ResizeHandles {
        window: root
        anchors.fill: parent
        handleSize: root.handleSize
    }
}
