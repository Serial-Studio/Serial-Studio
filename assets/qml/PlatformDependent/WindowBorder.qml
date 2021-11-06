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
    // Window controls
    //
    property Window window
    property bool fullScreen: false
    property bool closeEnabled: true
    property bool minimizeEnabled: true
    property bool maximizeEnabled: true
    property bool fullscreenEnabled: true
    property bool titlebarBorderEnabled: true
    property color textColor: palette.text

    //
    // Toggle maximized
    //
    function toggleMaximized() {
        if (window.visibility === Window.Maximized)
            window.showNormal()
        else
            window.showMaximized()
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
    height: 32

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
    // Bottom titlebar border
    //
    Rectangle {
        height: 1
        visible: root.titlebarBorderEnabled
        color: Qt.darker(parent.color, 1.5)

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
    // macOS layout
    //
    Item {
        //visible: Cpp_IsMac
        //enabled: Cpp_IsMac
        anchors.fill: parent

        RowLayout {
            spacing: 0
            anchors.fill: parent

            Item {
                width: 4
            }

            WindowButton {
                name: "close"
                onClicked: window.close()
                enabled: root.closeEnabled
                visible: root.closeEnabled
                Layout.alignment: Qt.AlignVCenter
            }

            WindowButton {
                name: "minimize"
                onClicked: window.showMinimized()
                Layout.alignment: Qt.AlignVCenter
                enabled: root.minimizeEnabled && !root.fullScreen
                visible: root.minimizeEnabled && !root.fullScreen
            }

            WindowButton {
                name: "maximize"
                onClicked: root.toggleMaximized()
                Layout.alignment: Qt.AlignVCenter
                enabled: root.maximizeEnabled && !root.fullScreen
                visible: root.maximizeEnabled && !root.fullScreen
            }

            Item {
                Layout.fillWidth: true
            }

            Widgets.Icon {
                width: 18
                height: 18
                color: "#fff"
                visible: root.fullscreenEnabled
                enabled: root.fullscreenEnabled
                Layout.alignment: Qt.AlignVCenter
                source: root.fullScreen ? "qrc:/icons/restore.svg" : "qrc:/icons/expand.svg"

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.toggleFullscreen()
                }
            }


            Item {
                width: 8
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
}
