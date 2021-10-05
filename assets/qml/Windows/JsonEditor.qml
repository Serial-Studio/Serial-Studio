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

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import Qt.labs.settings

import "../JsonEditor"
import "../Widgets" as Widgets

ApplicationWindow {
    id: root

    //
    // Window options
    //
    minimumWidth: 910
    minimumHeight: 720
    title: qsTr("JSON Editor - %1").arg(Cpp_JSON_Editor.jsonFileName)

    //
    // Ensure that current JSON file is shown
    //
    onVisibleChanged: {
        if (visible)
            Cpp_JSON_Editor.openJsonFile(Cpp_JSON_Generator.jsonMapFilepath)
    }

    //
    // Ask user to save changes before closing the window
    //
    onClosing: (close) => close.accepted = Cpp_JSON_Editor.askSave()

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
    // Connections with JSON editor model
    //
    Connections {
        target: Cpp_JSON_Editor

        function onGroupCountChanged() {
            view.model = 0
            view.model = Cpp_JSON_Editor.groupCount
        }

        function onGroupOrderChanged() {
            view.model = 0
            view.model = Cpp_JSON_Editor.groupCount
        }
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
        // Header (project properties)
        //
        Rectangle {
            id: headerBg
            border.width: 1
            border.color: Cpp_ThemeManager.toolbarGradient1
            height: header.implicitHeight + 4 * app.spacing

            gradient: Gradient {
                GradientStop { position: 0; color: Cpp_ThemeManager.toolbarGradient1 }
                GradientStop { position: 1; color: Cpp_ThemeManager.toolbarGradient2 }
            }

            GridLayout {
                id: header
                columns: 2
                rowSpacing: app.spacing
                columnSpacing: app.spacing * 2

                anchors {
                    left: parent.left
                    right: parent.right
                    margins: app.spacing * 2
                    verticalCenter: parent.verticalCenter
                }

                //
                // Project title
                //
                RowLayout {
                    spacing: app.spacing
                    Layout.fillWidth: true

                    Widgets.Icon {
                        color: Cpp_ThemeManager.brightText
                        source: "qrc:/icons/registration.svg"
                    }

                    TextField {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 320
                        text: Cpp_JSON_Editor.title
                        onTextChanged: Cpp_JSON_Editor.setTitle(text)
                        placeholderText: qsTr("Project title (required)")
                    }
                }

                //
                // Separator character
                //
                RowLayout {
                    spacing: app.spacing
                    Layout.fillWidth: true

                    Widgets.Icon {
                        color: Cpp_ThemeManager.brightText
                        source: "qrc:/icons/separator.svg"
                    }

                    TextField {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 420
                        text: Cpp_JSON_Editor.separator
                        onTextChanged: Cpp_JSON_Editor.setSeparator(text)
                        placeholderText: qsTr("Data separator (default is ',')")
                    }
                }

                //
                // Start sequence
                //
                RowLayout {
                    spacing: app.spacing
                    Layout.fillWidth: true

                    Widgets.Icon {
                        color: Cpp_ThemeManager.brightText
                        source: "qrc:/icons/start-sequence.svg"
                    }

                    TextField {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 256
                        text: Cpp_JSON_Editor.frameStartSequence
                        onTextChanged: Cpp_JSON_Editor.setFrameStartSequence(text)
                        placeholderText: qsTr("Frame start sequence (default is '%1')").arg(Cpp_IO_Manager.startSequence)
                    }
                }

                //
                // End sequence
                //
                RowLayout {
                    spacing: app.spacing
                    Layout.fillWidth: true

                    Widgets.Icon {
                        color: Cpp_ThemeManager.brightText
                        source: "qrc:/icons/end-sequence.svg"
                    }

                    TextField {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 256
                        text: Cpp_JSON_Editor.frameEndSequence
                        onTextChanged: Cpp_JSON_Editor.setFrameEndSequence(text)
                        placeholderText: qsTr("Frame end sequence (default is '%1')").arg(Cpp_IO_Manager.finishSequence)
                    }
                }
            }

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
        Rectangle {
            id: footerBg
            border.width: 1
            border.color: Cpp_ThemeManager.toolbarGradient1
            height: footer.implicitHeight + 4 * app.spacing

            gradient: Gradient {
                GradientStop { position: 0; color: Cpp_ThemeManager.toolbarGradient1 }
                GradientStop { position: 1; color: Cpp_ThemeManager.toolbarGradient2 }
            }

            anchors {
                margins: 0
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            //
            // Dialog buttons
            //
            RowLayout {
                id: footer
                spacing: app.spacing

                anchors {
                    left: parent.left
                    right: parent.right
                    margins: app.spacing * 2
                    verticalCenter: parent.verticalCenter
                }

                Button {
                    icon.width: 24
                    icon.height: 24
                    onClicked: root.close()
                    text: qsTr("Close") + _btSpacer
                    icon.source: "qrc:/icons/close.svg"
                    icon.color: Cpp_ThemeManager.brightText
                    palette.buttonText: Cpp_ThemeManager.brightText
                    palette.button: Cpp_ThemeManager.toolbarGradient1
                    palette.window: Cpp_ThemeManager.toolbarGradient1
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    id: addGrp
                    icon.width: 24
                    icon.height: 24
                    highlighted: true
                    Layout.fillWidth: true
                    text: qsTr("Add group")
                    icon.source: "qrc:/icons/add.svg"
                    icon.color: Cpp_ThemeManager.brightText
                    palette.buttonText: Cpp_ThemeManager.brightText
                    palette.button: Cpp_ThemeManager.toolbarGradient1
                    palette.window: Cpp_ThemeManager.toolbarGradient1
                    onClicked: {
                        Cpp_JSON_Editor.addGroup()
                        scroll.position = 1
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    icon.width: 24
                    icon.height: 24
                    icon.source: "qrc:/icons/open.svg"
                    icon.color: Cpp_ThemeManager.brightText
                    onClicked: Cpp_JSON_Editor.openJsonFile()
                    palette.buttonText: Cpp_ThemeManager.brightText
                    palette.button: Cpp_ThemeManager.toolbarGradient1
                    palette.window: Cpp_ThemeManager.toolbarGradient1
                    text: qsTr("Open existing project...") + _btSpacer
                }

                Button {
                    icon.width: 24
                    icon.height: 24
                    icon.source: "qrc:/icons/new.svg"
                    icon.color: Cpp_ThemeManager.brightText
                    onClicked: Cpp_JSON_Editor.newJsonFile()
                    text: qsTr("Create new project") + _btSpacer
                    palette.buttonText: Cpp_ThemeManager.brightText
                    palette.button: Cpp_ThemeManager.toolbarGradient1
                    palette.window: Cpp_ThemeManager.toolbarGradient1
                }

                Button {
                    icon.width: 24
                    icon.height: 24
                    opacity: enabled ? 1: 0.5
                    enabled: Cpp_JSON_Editor.modified
                    icon.source: "qrc:/icons/apply.svg"
                    icon.color: Cpp_ThemeManager.brightText
                    palette.buttonText: Cpp_ThemeManager.brightText
                    palette.button: Cpp_ThemeManager.toolbarGradient1
                    palette.window: Cpp_ThemeManager.toolbarGradient1
                    text: (Cpp_JSON_Editor.jsonFilePath.length > 0 ? qsTr("Apply") : qsTr("Save")) + _btSpacer

                    onClicked: {
                        if (Cpp_JSON_Editor.saveJsonFile())
                            root.close()
                    }

                    Behavior on opacity {NumberAnimation{}}
                }
            }
        }

        //
        // Window controls
        //
        ColumnLayout {
            id: column
            spacing: 0
            clip: true
            anchors.fill: parent
            anchors.margins: app.spacing * 2
            anchors.topMargin: headerBg.height
            anchors.bottomMargin: footerBg.height

            //
            // Spacer
            //
            Item {
                height: 2 * app.spacing
            }

            //
            // List view
            //
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 320
                Layout.topMargin: -app.spacing
                Layout.bottomMargin: -app.spacing
                Layout.leftMargin: -2 * app.spacing
                Layout.rightMargin: -2 * app.spacing

                ListView {
                    id: view
                    anchors.fill: parent
                    model: Cpp_JSON_Editor.groupCount
                    anchors.bottomMargin: app.spacing

                    ScrollBar.vertical: ScrollBar {
                        id: scroll
                        policy: ScrollBar.AsNeeded
                    }

                    delegate: Item {
                        x: (parent.width - width) / 2
                        height: group.height + app.spacing
                        width: parent.width - 4 * app.spacing

                        //
                        // Group window
                        //
                        JsonGroupDelegate {
                            id: group
                            group: index
                            anchors {
                                left: parent.left
                                right: parent.right
                                bottom: parent.bottom
                            }
                        }


                        //
                        // Window shadow
                        //
                        Widgets.Shadow {
                            source: group
                            anchors.fill: group
                        }
                    }
                }

                ColumnLayout {
                    spacing: app.spacing
                    anchors.centerIn: parent
                    visible: Cpp_JSON_Editor.groupCount === 0

                    Button {
                        flat: true
                        enabled: false
                        icon.width: 128
                        icon.height: 128
                        Layout.alignment: Qt.AlignHCenter
                        icon.color: Cpp_ThemeManager.text
                        icon.source: "qrc:/icons/developer-board.svg"
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
                        text: qsTr("Click on the \"%1\" button to begin").arg(addGrp.text)
                    }
                }
            }

            //
            // Spacer
            //
            Item {
                height: 2 * app.spacing
            }
        }

        //
        // Shadows
        //
        Widgets.Shadow {
            source: headerBg
            horizontalOffset: 0
            anchors.fill: headerBg
        } Widgets.Shadow {
            source: footerBg
            verticalOffset: -3
            horizontalOffset: 0
            anchors.fill: footerBg
        }
    }
}
