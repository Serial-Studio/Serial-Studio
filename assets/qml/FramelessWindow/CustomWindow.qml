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
    flags: root.customFlags | root.extraFlags

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
    property int borderWidth: 2
    readonly property int handleSize: radius > 0 ? radius + 10 + shadowMargin : 0
    readonly property int radius: ((root.visibility === Window.Maximized && maximizeEnabled) || isFullscreen) ? 0 : 10

    //
    // Visibility properties
    //
    property int extraFlags: 0
    property bool firstChange: true
    property bool isMaximized: false
    property bool isMinimized: false
    property alias isFullscreen: border.isFullscreen
    readonly property int customFlags: Qt.CustomizeWindowHint |
                                       Qt.FramelessWindowHint |
                                       Qt.NoDropShadowWindowHint

    //
    // Toggle fullscreen state
    //
    function toggleFullscreen() {
        root.isFullscreen = !root.isFullscreen
        if (root.isFullscreen)
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
    // Titlebar left/right margins for custom controls
    //
    property alias leftTitlebarMargin: border.leftMargin
    property alias rightTitlebarMargin: border.rightMargin
    property alias showMacControls: border.showMacControls

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
    property alias closeEnabled: border.closeEnabled
    property alias minimizeEnabled: border.minimizeEnabled
    property alias maximizeEnabled: border.maximizeEnabled
    property alias fullscreenEnabled: border.fullscreenEnabled

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
        radius: root.radius
        color: "transparent"
        anchors.fill: parent
        border.color: root.borderColor
        border.width: root.borderWidth
        anchors.margins: root.shadowMargin
    }
    
    //
    // Resize handler
    //
    ResizeHandles {
        window: root
        anchors.fill: parent
        handleSize: root.handleSize
    }

    //
    // Titlebar control
    //
    Titlebar {
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
        // Hard-reset window flags on macOS to fix most glitches
        if (Cpp_IsMac)
            root.flags = Qt.Window

        // Ensure that correct window flags are still used
        root.flags = root.customFlags | root.extraFlags

        // Window has been just maximized, update internal variables
        if (visibility === Window.Maximized) {
            if (!root.isMaximized)
                root.firstChange = false

            root.isMaximized = true
            root.isFullscreen = false
        }

        // Window has been just minimized, update internal variables
        else if (visibility === Window.Minimized) {
            root.isFullscreen = false
            root.isMaximized = false
        }

        // Window has been just switched to full-screen, update internal variables
        else if (visibility === Window.FullScreen) {
            if (!root.isFullscreen)
                root.firstChange = false

            root.isFullscreen = true
            root.isMaximized = false
        }

        // Window was just restored to "normal" mode, recover previous geometry
        else if (visibility !== Window.Hidden) {
            if (isMaximized || isFullscreen && firstChange) {
                root.width = root.minimumWidth
                root.height = root.minimumHeight
                root.x = (Screen.desktopAvailableWidth - root.width) / 2
                root.y = (Screen.desktopAvailableHeight - root.height) / 2
            }

            root.isMaximized = false
            root.isFullscreen = false

            // Force active focus
            root.requestActivate()
            root.requestUpdate()
        }
    }
}
