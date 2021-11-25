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

import "../Widgets" as Widgets

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
    readonly property int handleSize: radius > 0 ? radius + 10 : 0
    readonly property int radius: ((root.visibility === Window.Maximized && maximizeEnabled) || isFullscreen) ? 0 : 10

    //
    // Visibility properties
    //
    property int extraFlags: 0
    property bool firstChange: true
    property bool isMaximized: false
    property bool isMinimized: false
    property alias showIcon: _title.showIcon
    property alias isFullscreen: _title.isFullscreen
    readonly property int customFlags: {
        // Setup frameless window flags
        var flags = Qt.Window |
                    Qt.CustomizeWindowHint |
                    Qt.FramelessWindowHint |
                    Qt.WindowSystemMenuHint |
                    Qt.WindowMinMaxButtonsHint

        //
        // The macOS window manager is able to generate shadows for Qt frameless
        // windows. Other operating systems have varied mileage, so we draw the
        // shadow ourselves to avoid ugly surprises.
        //
        // Also, disabling the custom shadow on macOS is a good idea so that users can
        // move the window freely over all the desktop without being blocked by the
        // menubar or dock due to the custom shadow border.
        //
        if (!Cpp_IsMac)
            flags |= Qt.NoDropShadowWindowHint

        // Return "calculated" window flags
        return flags
    }

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
    property alias titlebar: _title

    //
    // Size of the shadow object
    //
    property int shadowMargin: Cpp_IsMac ? 0 : (root.radius > 0 ? 20 : 0)

    //
    // Titlebar left/right margins for custom controls
    //
    property alias leftTitlebarMargin: _title.leftMargin
    property alias rightTitlebarMargin: _title.rightMargin
    property alias showMacControls: _title.showMacControls

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
    property alias closeEnabled: _title.closeEnabled
    property alias minimizeEnabled: _title.minimizeEnabled
    property alias maximizeEnabled: _title.maximizeEnabled

    //
    // Ensure that window size & position is valid upon showing
    //
    Component.onCompleted: {
        // Reset window size for whatever reason
        if (root.width <= 0 || root.height <= 0) {
            root.width = root.minimumWidth
            root.height = root.minimumHeight
        }

        // Startup verifications to ensure that app is displayed inside the screen
        if (root.x < 0 || root.x >= Screen.desktopAvailableWidth)
            root.x = 100
        if (root.y < 0 || root.y >= Screen.desktopAvailableHeight)
            root.y = 100

        // Startup verifications to ensure that app fits in current screen
        if (root.width > Screen.desktopAvailableWidth) {
            root.x = 100
            root.width = Screen.desktopAvailableWidth - root.x
        }

        // Startup verifications to ensure that app fits in current screen
        if (root.height > Screen.desktopAvailableHeight) {
            root.y = 100
            root.height = Screen.desktopAvailableHeight - root.y
        }
    }

    //
    // Custom shadow implementation for non-macOS systems. The shadow shall be disabled
    // when the window is maximized.
    //
    Widgets.Shadow {
        opacity: 0.2
        anchors.fill: bg
        visible: root.shadowMargin > 0
        topMargin: -1 * root.shadowMargin
        leftMargin: -1 * root.shadowMargin
        rightMargin: -1 * root.shadowMargin
        bottomMargin: -1 * root.shadowMargin
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
        id: _title
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
