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
import QtQuick.Window 2.2 as QtWindow

import "../Widgets" as Widgets

QtWindow.Window {
    id: root
    color: "transparent"
    flags: (Cpp_ThemeManager.customWindowDecorations ? root.customFlags : Qt.Window) | root.extraFlags

    //
    // Custom signals
    //
    signal closed()
    signal minimized()
    signal maximized()
    signal unmaximized()

    //
    // Connections with the theme manager for enabling/disabling frameless window
    //
    Connections {
        target: Cpp_ThemeManager

        function onCustomWindowDecorationsChanged() {
            if (Cpp_ThemeManager.customWindowDecorations)
                root.flags = root.customFlags | root.extraFlags
            else
                root.flags = Qt.Window | root.extraFlags

            var prevVisible = root.visible
            root.visible = false
            root.visible = prevVisible
        }
    }

    //
    // Window radius control
    //
    property int borderWidth: Cpp_ThemeManager.customWindowDecorations ? 2 : 0
    readonly property int handleSize: radius > 0 ? radius + shadowMargin + 10 : 0
    readonly property int radius: Cpp_ThemeManager.customWindowDecorations ?
                                      (((root.visibility === QtWindow.Window.Maximized &&
                                         maximizeEnabled) || isFullscreen) ? 0 : 10) : 0

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
    property int shadowMargin: (Cpp_IsMac | !Cpp_ThemeManager.customWindowDecorations) ?
                                   0 : (root.radius > 0 ? 20 : 0)

    //
    // Titlebar left/right margins for custom controls
    //
    property alias showMacControls: _title.showMacControls
    readonly property real leftTitlebarMargin: Cpp_ThemeManager.customWindowDecorations ? _title.leftMargin : 0
    readonly property real rightTitlebarMargin: Cpp_ThemeManager.customWindowDecorations ? _title.rightMargin : 0

    //
    // Background color of the window & the titlebar
    //
    property color borderColor: Cpp_ThemeManager.highlight
    property color backgroundColor: Cpp_ThemeManager.window
    property color titlebarText: Cpp_ThemeManager.menubarText
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
        if (root.x < 0 || root.x >= QtWindow.Screen.desktopAvailableWidth)
            root.x = 100
        if (root.y < 0 || root.y >= QtWindow.Screen.desktopAvailableHeight)
            root.y = 100

        // Startup verifications to ensure that app fits in current screen
        if (root.width > QtWindow.Screen.desktopAvailableWidth) {
            root.x = 100
            root.width = QtWindow.Screen.desktopAvailableWidth - root.x
        }

        // Startup verifications to ensure that app fits in current screen
        if (root.height > QtWindow.Screen.desktopAvailableHeight) {
            root.y = 100
            root.height = QtWindow.Screen.desktopAvailableHeight - root.y
        }
    }

    //
    // Custom shadow implementation for non-macOS systems. The shadow shall be disabled
    // when the window is maximized.
    //
    Widgets.Shadow {
        border.top: 50
        border.left: 50
        border.right: 50
        border.bottom: 50
        anchors.fill: bg
        shadowOpacity: 1.0
        visible: root.shadowMargin > 0
        topMargin: -1 * root.shadowMargin
        leftMargin: -1 * root.shadowMargin
        rightMargin: -1 * root.shadowMargin
        bottomMargin: -1 * root.shadowMargin
        source: "qrc:/images/window-shadow.png"
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
        opacity: 0.8
        z: titlebar.z + 1
        radius: root.radius
        color: "transparent"
        anchors.fill: parent
        border.color: root.borderColor
        border.width: root.borderWidth
        anchors.margins: root.shadowMargin
    }

    //
    // Global mouse area to set cursor shape while resizing
    //
    MouseArea {
        z: titlebar.z - 1
        hoverEnabled: true
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        enabled: Cpp_ThemeManager.customWindowDecorations
        cursorShape: {
            const p = Qt.point(mouseX, mouseY)
            const b = root.handleSize / 2

            if (p.x < b && p.y < b)
                return Qt.SizeFDiagCursor

            if (p.x >= width - b && p.y >= height - b)
                return Qt.SizeFDiagCursor

            if (p.x >= width - b && p.y < b)
                return Qt.SizeBDiagCursor

            if (p.x < b && p.y >= height - b)
                return Qt.SizeBDiagCursor

            if (p.x < b || p.x >= width - b)
                return Qt.SizeHorCursor

            if (p.y < b || p.y >= height - b)
                return Qt.SizeVerCursor
        }
    }

    //
    // Titlebar control
    //
    Titlebar {
        z: 2000
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
        // Ensure that correct window flags are still used
        if (Cpp_ThemeManager.customWindowDecorations) {
            // Hard-reset window flags on macOS to fix most glitches
            if (Cpp_IsMac)
                root.flags = Qt.Window

            // Apply custom flags
            root.flags = root.customFlags | root.extraFlags
        }

        // Apply basic flags
        else
            root.flags = Qt.Window | root.extraFlags

        // Window has been just maximized, update internal variables
        if (visibility === QtWindow.Window.Maximized) {
            if (!root.isMaximized)
                root.firstChange = false

            root.isMaximized = true
            root.isFullscreen = false
        }

        // Window has been just minimized, update internal variables
        else if (visibility === QtWindow.Window.Minimized) {
            root.isFullscreen = false
            root.isMaximized = false
        }

        // Window has been just switched to full-screen, update internal variables
        else if (visibility === QtWindow.Window.FullScreen) {
            if (!root.isFullscreen)
                root.firstChange = false

            root.isFullscreen = true
            root.isMaximized = false
        }

        // Window was just restored to "normal" mode, recover previous geometry
        else if (visibility !== QtWindow.Window.Hidden) {
            if (isMaximized || isFullscreen && firstChange) {
                root.width = root.minimumWidth
                root.height = root.minimumHeight
                root.x = (QtWindow.Screen.desktopAvailableWidth - root.width) / 2
                root.y = (QtWindow.Screen.desktopAvailableHeight - root.height) / 2
            }

            root.isMaximized = false
            root.isFullscreen = false

            // Force active focus
            root.requestActivate()
            root.requestUpdate()
        }
    }
}
