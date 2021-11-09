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
import QtQuick.Window 2.12
import QtQuick.Controls 2.12

import "../Widgets" as Widgets

Rectangle {   
    id: root

    //
    // Custom signals
    //
    signal closed()
    signal minimized()
    signal maximized()
    signal unmaximized()

    //
    // Window controls
    //
    property Window window
    property bool displayIcon: true
    property bool fullScreen: false
    property bool closeEnabled: true
    property bool minimizeEnabled: true
    property bool maximizeEnabled: true
    property color textColor: palette.text
    readonly property bool showMacControls: Cpp_IsMac

    //
    // Toggle maximized
    //
    function toggleMaximized() {
        if (window.visibility === Window.Maximized) {
            window.showNormal()
            root.unmaximized()
        }

        else {
            window.showMaximized()
            root.maximized()
        }
    }

    //
    // Toggle fullscreen state
    //
    function toggleFullscreen() {
        root.fullScreen = !root.fullScreen
        if (window.fullScreen)
            window.showFullScreen()
        else
            window.showNormal()
    }

    //
    // Height calculation
    //
    height: !showMacControls ? 38 : 32

    //
    // Radius compensator rectangle
    //
    Rectangle {
        color: parent.color
        height: parent.radius
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
    }

    //
    // Window drag handler
    //
    Item {
        anchors.fill: parent

        DragHandler {
            grabPermissions: TapHandler.CanTakeOverFromAnything
            onActiveChanged: {
                if (active)
                    window.startSystemMove()
            }
        }
    }

    //
    // Window maximize by double click
    //
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onDoubleClicked: {
            if (root.maximizeEnabled)
                root.toggleMaximized()
        }
    }

    //
    // macOS layout
    //
    Item {
        anchors.fill: parent
        visible: showMacControls
        enabled: showMacControls

        RowLayout {
            spacing: 0
            anchors.fill: parent

            Item {
                width: 4
            }

            WindowButtonMacOS {
                name: "close"
                enabled: root.closeEnabled
                visible: root.closeEnabled
                Layout.alignment: Qt.AlignVCenter
                onClicked: {
                    window.close()
                    root.closed()
                }

            }

            WindowButtonMacOS {
                name: "minimize"
                Layout.alignment: Qt.AlignVCenter
                enabled: root.minimizeEnabled && !root.fullScreen
                visible: root.minimizeEnabled && !root.fullScreen
                onClicked: {
                    window.showMinimized()
                    root.minimized()
                }
            }

            WindowButtonMacOS {
                name: "maximize"
                onClicked: root.toggleMaximized()
                Layout.alignment: Qt.AlignVCenter
                enabled: root.maximizeEnabled && !root.fullScreen
                visible: root.maximizeEnabled && !root.fullScreen
            }

            Item {
                Layout.fillWidth: true
            }

            WindowButton {
                width: 18
                height: 18
                textColor: root.textColor
                visible: root.fullscreenEnabled
                enabled: root.fullscreenEnabled
                Layout.alignment: Qt.AlignVCenter
                onClicked: root.toggleFullscreen()
                highlightColor: Cpp_ThemeManager.highlight
                name: root.fullScreen ? "restore" : "fullscreen"
            }

            Item {
                width: 8
            }
        }
    }

    //
    // Windows & Linux layout
    //
    Item {
        anchors.fill: parent
        visible: !showMacControls
        enabled: !showMacControls

        RowLayout {
            spacing: 0
            anchors.fill: parent

            Item {
                width: 8
            }

            Image {
                width: 24
                height: 24
                opacity: 0.9
                visible: root.displayIcon
                enabled: root.displayIcon
                sourceSize: Qt.size(24, 24)
                Layout.alignment: Qt.AlignVCenter
                source: "qrc:/images/icon-window.svg"
            }

            Item {
                Layout.fillWidth: true
            }

            WindowButton {
                name: "minimize"
                textColor: root.textColor
                Layout.alignment: Qt.AlignVCenter
                highlightColor: Cpp_ThemeManager.highlight
                enabled: root.minimizeEnabled && !root.fullScreen
                visible: root.minimizeEnabled && !root.fullScreen
                onClicked: {
                    window.showMinimized()
                    root.minimized()
                }
            }

            WindowButton {
                textColor: root.textColor
                onClicked: root.toggleMaximized()
                Layout.alignment: Qt.AlignVCenter
                highlightColor: Cpp_ThemeManager.highlight
                enabled: root.maximizeEnabled && !root.fullScreen
                visible: root.maximizeEnabled && !root.fullScreen
                name: window.visibility === Window.Maximized ? "unmaximize" : "maximize"
            }

            WindowButton {
                name: "close"
                highlightColor: "#f00"
                textColor: root.textColor
                enabled: root.closeEnabled
                visible: root.closeEnabled
                Layout.alignment: Qt.AlignVCenter
                onClicked: {
                    window.close()
                    root.closed()
                }

            }

            Item {
                width: 8
            }
        }
    }

    Label {
        font.bold: true
        font.pixelSize: 14
        text: window.title
        color: root.textColor
        anchors.centerIn: parent
    }
}
