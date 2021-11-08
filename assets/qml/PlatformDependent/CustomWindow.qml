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

Window {
    id: root
    color: "transparent"
    flags: root.customFlags

    //
    // Window radius control
    //
    readonly property int windowBorder: radius + 5
    property int radius: ((root.visibility === Window.Maximized && maximizeEnabled) || fullScreen) ? 0 : 10

    //
    // Visibility properties
    //
    property bool firstChange: true
    property bool windowMaximized: false
    property alias fullScreen: border.fullScreen
    readonly property int customFlags: Qt.Dialog |
                                       Qt.FramelessWindowHint

    //
    // Toggle fullscreen state
    //
    function toggleFullscreen() {
        root.fullScreen = !root.fullScreen
        if (root.fullScreen)
            root.showFullScreen()
        else
            root.showNormal()
    }

    //
    // Alias to the titlebar
    //
    property alias titlebar: border

    //
    // Background color of the window & the titlebar
    //
    property color backgroundColor: Cpp_ThemeManager.window
    property color titlebarText: Cpp_ThemeManager.brightText
    property color titlebarColor: Cpp_ThemeManager.toolbarGradient2

    //
    // Window controls
    //
    property alias closeEnabled: border.closeEnabled
    property alias minimizeEnabled: border.minimizeEnabled
    property alias maximizeEnabled: border.maximizeEnabled
    property alias fullscreenEnabled: border.fullscreenEnabled
    property alias titlebarBorderEnabled: border.titlebarBorderEnabled

    //
    // Background color implementation
    //
    Rectangle {
        radius: root.radius
        anchors.fill: parent
        color: root.backgroundColor
    }

    //
    // Titlebar control
    //
    WindowBorder {
        id: border
        window: root
        radius: root.radius
        color: root.titlebarColor
        textColor: root.titlebarText

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
    }

    //
    // Maximize window fixes
    //
    onVisibilityChanged: {
        if (visibility === Window.Maximized) {
            if (!root.windowMaximized)
                root.firstChange = false

            root.windowMaximized = true
            root.fullScreen = false
            root.flags = root.customFlags
        }

        else if (visibility === Window.FullScreen) {
            if (!root.fullScreen)
                root.firstChange = false

            root.windowMaximized = false
            root.fullScreen = true
        }

        else if (visibility !== Window.Hidden) {
            if (windowMaximized || fullScreen && firstChange) {
                root.x = 100
                root.y = 100
                root.width = root.minimumWidth
                root.height = root.minimumHeight
            }

            root.fullScreen = false
            root.windowMaximized = false
            root.flags = root.customFlags
        }
    }

    //
    // Right resize handle
    //
    MouseArea {
        property bool dragging: false
        property point lastMousePos: Qt.point(0, 0)

        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
            topMargin: titlebar.height
        }

        hoverEnabled: true
        width: windowBorder
        cursorShape: Qt.SizeHorCursor
        onPressedChanged: dragging = pressed
        onPressed: lastMousePos = Qt.point(mouseX, mouseY)

        onMouseXChanged: {
            if (dragging) {
                var dx = mouseX + lastMousePos.x
                root.width += dx
            }
        }
    }

    //
    // Left resize handle
    //
    MouseArea {
        property bool dragging: false
        property point lastMousePos: Qt.point(0, 0)

        anchors {
            top: parent.top
            left: parent.left
            bottom: parent.bottom
            topMargin: titlebar.height
        }

        hoverEnabled: true
        width: windowBorder
        cursorShape: Qt.SizeHorCursor
        onPressedChanged: dragging = pressed
        onPressed: lastMousePos = Qt.point(mouseX, mouseY)

        onMouseXChanged: {
            if (dragging) {
                var dx = mouseX - lastMousePos.x
                var y = root.y
                var x = root.x + dx
                var height = root.height
                var width = root.width - dx

                if (x > root.x)
                    width = root.width - dx / 2

                root.setGeometry(x, y, width, height)
            }
        }
    }

    //
    // Background color implementation
    //
    Rectangle {
        z: 100
        border.width: 0
        color: "transparent"
        border.color: "#000"
        radius: root.radius
        anchors.fill: parent
    }

    //
    // Bottom resize handle
    //
    MouseArea {
        property bool dragging: false
        property point lastMousePos: Qt.point(0, 0)

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        hoverEnabled: true
        height: windowBorder
        cursorShape: Qt.SizeVerCursor
        onPressedChanged: dragging = pressed
        onPressed: lastMousePos = Qt.point(mouseX, mouseY)
        onMouseYChanged: {
            if (dragging) {
                var dy = mouseY - lastMousePos.y
                root.height += dy
            }
        }
    }

    //
    // Bottom right corner
    //
    MouseArea {
        property bool dragging: false
        property point lastMousePos: Qt.point(0, 0)

        function updateWindowPosition() {
            if (dragging) {
                var dy = mouseY - lastMousePos.y
                var dx = mouseX + lastMousePos.x

                root.width += dx
                root.height += dy
            }
        }

        anchors {
            right: parent.right
            bottom: parent.bottom
        }

        hoverEnabled: true
        width: windowBorder
        height: windowBorder
        cursorShape: Qt.SizeFDiagCursor
        onPressedChanged: dragging = pressed
        onMouseXChanged: updateWindowPosition()
        onPressed: lastMousePos = Qt.point(mouseX, mouseY)
    }

    //
    // Bottom left corner
    //
    MouseArea {
        property bool dragging: false
        property point lastMousePos: Qt.point(0, 0)

        function updateWindowPosition() {
            if (dragging) {
                var dx = mouseX - lastMousePos.x
                var dy = mouseY - lastMousePos.y

                var y = root.y
                var x = root.x + dx
                var width = root.width - dx
                var height = root.height + dy

                if (x > root.x)
                    width = root.width - dx / 2

                root.setGeometry(x, y, width, height)
            }
        }

        anchors {
            left: parent.left
            bottom: parent.bottom
        }

        hoverEnabled: true
        width: windowBorder
        height: windowBorder
        cursorShape: Qt.SizeBDiagCursor
        onPressedChanged: dragging = pressed
        onMouseXChanged: updateWindowPosition()
        onPressed: lastMousePos = Qt.point(mouseX, mouseY)
    }
}
