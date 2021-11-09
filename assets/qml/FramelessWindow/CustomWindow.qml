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
import QtGraphicalEffects 1.0

Window {
    id: root
    color: "transparent"
    flags: root.customFlags

    //
    // Custom signals
    //
    signal closed()
    signal minimized()
    signal maximized()
    signal unmaximized()

    //
    // Window radius control
    //
    readonly property int handleSize: radius + 5 + shadowMargin
    readonly property int radius: ((root.visibility === Window.Maximized && maximizeEnabled) || fullScreen) ? 0 : 10

    //
    // Visibility properties
    //
    property bool firstChange: true
    property bool windowMaximized: false
    property bool windowMinimized: false
    property alias fullScreen: border.fullScreen
    readonly property int customFlags: Qt.CustomizeWindowHint |
                                       Qt.FramelessWindowHint |
                                       Qt.NoDropShadowWindowHint

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
    // Size of the shadow object
    //
    property int shadowMargin: root.radius > 0 ? 20 : 0

    //
    // Background color of the window & the titlebar
    //
    property color borderColor: Cpp_ThemeManager.highlight
    property color backgroundColor: Cpp_ThemeManager.window
    property color titlebarText: Cpp_ThemeManager.brightText
    property color titlebarColor: Cpp_ThemeManager.toolbarGradient2

    //
    // Window controls
    //
    property alias displayIcon: border.displayIcon
    property alias closeEnabled: border.closeEnabled
    property alias minimizeEnabled: border.minimizeEnabled
    property alias maximizeEnabled: border.maximizeEnabled

    //
    // Shadow implementation
    //
    RectangularGlow {
        spread: 0.2
        anchors.fill: bg
        color: Qt.rgba(0,0,0,0.5)
        glowRadius: root.shadowMargin / 2
        cornerRadius: bg.radius + glowRadius
    } Rectangle {
        id: bg
        color: "transparent"
        radius: root.radius
        anchors.fill: parent
        anchors.margins: root.shadowMargin
    }

    //
    // Window border
    //
    Rectangle {
        z: 1000
        opacity: 0.8
        border.width: 2
        radius: root.radius
        color: "transparent"
        anchors.fill: parent
        border.color: root.borderColor
        anchors.margins: root.shadowMargin - 1
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

        onClosed: root.closed()
        onMinimized: root.minimized()
        onMaximized: root.maximized()
        onUnmaximized: root.unmaximized()

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: root.shadowMargin
        }
    }

    //
    // Maximize window fixes
    //
    onVisibilityChanged: {
        if (visibility === Window.Maximized) {
            if (!root.windowMaximized)
                root.firstChange = false

            root.fullScreen = false
            root.windowMaximized = true
        }

        else if (visibility === Window.Minimized) {
            root.fullScreen = false
            root.windowMaximized = false
        }

        else if (visibility === Window.FullScreen) {
            if (!root.fullScreen)
                root.firstChange = false

            root.fullScreen = true
            root.windowMaximized = false
        }

        else if (visibility !== Window.Hidden) {
            if (windowMaximized || fullScreen && firstChange) {
                root.width = root.minimumWidth
                root.height = root.minimumHeight
                root.x = (Screen.desktopAvailableWidth - root.width) / 2
                root.y = (Screen.desktopAvailableHeight - root.height) / 2
            }

            root.fullScreen = false
            root.windowMaximized = false
            root.flags = root.customFlags
        }
    }

    //
    // Resize handler
    //
    ResizeHandles {
        window: root
        anchors.fill: parent
        handleSize: root.handleSize
    }
}
