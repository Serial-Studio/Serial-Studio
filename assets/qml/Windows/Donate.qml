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
    // Window options
    //
    title: qsTr("Donate")
    width: minimumWidth
    height: minimumHeight
    minimizeEnabled: false
    maximizeEnabled: false
    titlebarText: Cpp_ThemeManager.text
    x: (Screen.desktopAvailableWidth - width) / 2
    y: (Screen.desktopAvailableHeight - height) / 2
    titlebarColor: Cpp_ThemeManager.dialogBackground
    backgroundColor: Cpp_ThemeManager.dialogBackground
    minimumWidth: column.implicitWidth + 4 * app.spacing + 2 * root.shadowMargin
    maximumWidth: column.implicitWidth + 4 * app.spacing + 2 * root.shadowMargin
    extraFlags: Qt.WindowStaysOnTopHint | Qt.Dialog | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
    minimumHeight: column.implicitHeight + 4 * app.spacing + titlebar.height + 2 * root.shadowMargin
    maximumHeight: column.implicitHeight + 4 * app.spacing + titlebar.height + 2 * root.shadowMargin

    //
    // Custom properties
    //
    property alias doNotShowAgain: doNotShowAgainCheck.checked

    //
    // Enables the "do not show again" checkbox and sets
    // the text of the close button to "later".
    //
    // This is used when the window is shown automatically
    // every now and then to the user.
    //
    function showAutomatically() {
        doNotShowAgainCheck.visible = true
        closeBt.text = qsTr("Later")
        showNormal()
    }

    //
    // Disables the "do not show again" checkbox and sets
    // the text of the close button to "close".
    //
    // This is used when the user opens this dialog from
    // the "about" window.
    //
    function show() {
        doNotShowAgainCheck.visible = false
        closeBt.text = qsTr("Close")
        showNormal()
    }

    //
    // Save settings
    //
    Settings {
        property alias disableDonationsWindow: root.doNotShowAgain
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
        // Window drag handler
        //
        Item {
            anchors.fill: parent

            MouseArea {
                anchors.fill: parent
                onPressedChanged: {
                    if (pressed)
                        root.startSystemMove()
                }
            }
        }

        //
        // Window controls
        //
        ColumnLayout {
            id: column
            anchors.centerIn: parent
            spacing: app.spacing * 2

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: app.spacing * 2

                Image {
                    sourceSize: Qt.size(120, 120)
                    Layout.alignment: Qt.AlignVCenter
                    source: "qrc:/images/donate-qr.svg"

                    Rectangle {
                        border.width: 2
                        color: "transparent"
                        anchors.fill: parent
                        border.color: "#000"
                    }
                }

                ColumnLayout {
                    spacing: 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Label {
                        id: title
                        Layout.fillWidth: true
                        text: "<h3>" + qsTr("Support the development of %1!").arg(Cpp_AppName) + "</h3>"
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.maximumWidth: title.implicitWidth
                        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                        text: qsTr("Serial Studio is free & open-source software supported by volunteers. " +
                                   "Consider donating to support development efforts :)")
                    }

                    Item {
                        height: app.spacing * 2
                    }

                    Label {
                        opacity: 0.8
                        font.pixelSize: 12
                        Layout.fillWidth: true
                        Layout.maximumWidth: title.implicitWidth
                        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
                        color: Cpp_ThemeManager.highlightedTextAlternative
                        text: qsTr("You can also support this project by sharing it, reporting bugs and proposing new features!")
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: app.spacing

                CheckBox {
                    id: doNotShowAgainCheck
                    Layout.leftMargin: -app.spacing
                    Layout.alignment: Qt.AlignVCenter
                    text: qsTr("Don't annoy me again!")
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    id: closeBt
                    onClicked: root.close()
                    Layout.alignment: Qt.AlignVCenter
                }

                Button {
                    highlighted: true
                    text: qsTr("Donate")
                    Keys.onEnterPressed: clicked()
                    Keys.onReturnPressed: clicked()
                    Layout.alignment: Qt.AlignVCenter
                    Component.onCompleted: forceActiveFocus()
                    onClicked: Qt.openUrlExternally("https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE")
                }
            }
        }
    }
}
