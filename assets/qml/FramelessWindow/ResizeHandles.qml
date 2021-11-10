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

Item {
    id: root

    //
    // Pointer to window to control
    //
    property Window window
    property int handleSize

    //
    // Disable handles if window size is fixed or window is maximized
    //
    enabled: ((window.minimumWidth !== window.maximumWidth) ||
              (window.minimumHeight !== window.maximumHeight)) &&
             (window.visibility !== Window.Maximized)

    //
    // Global mouse area to fix cursor shape while resizing
    //
    MouseArea {
        z: 1000
        hoverEnabled: true
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        cursorShape: {
            const p = Qt.point(mouseX, mouseY)
            const b = handleSize / 2

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
    // Handle for resizing the window
    // Note: this does not work on macOS, see the following article for more information
    //       https://www.qt.io/blog/custom-window-decorations
    //
    DragHandler {
        id: resizeHandler
        target: null
        enabled: !Cpp_IsMac
        grabPermissions: TapHandler.TakeOverForbidden
        onActiveChanged: {
            if (active) {
                const p = resizeHandler.centroid.position
                const b = root.handleSize
                let e = 0;

                if (p.x < b)
                    e |= Qt.LeftEdge

                if (p.x >= width - b)
                    e |= Qt.RightEdge

                if (p.y < b)
                    e |= Qt.TopEdge

                if (p.y >= height - b)
                    e |= Qt.BottomEdge

                window.startSystemResize(e)
            }
        }
    }

    //
    // Right resize handle for macOS
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

        width: handleSize
        enabled: Cpp_IsMac
        onPressedChanged: dragging = pressed
        onPressed: lastMousePos = Qt.point(mouseX, mouseY)

        onMouseXChanged: {
            if (dragging) {
                var dx = mouseX + lastMousePos.x
                var width = window.width + dx
                if (width < window.minimumWidth)
                    width = window.minimumWidth
                else if (width > window.maximumWidth)
                    width = window.maximumWidth

                window.setGeometry(window.x, window.y, width, window.height)
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

        width: handleSize
        enabled: Cpp_IsMac
        onPressedChanged: dragging = pressed
        onPressed: lastMousePos = Qt.point(mouseX, mouseY)

        onMouseXChanged: {
            if (dragging) {
                var dx = mouseX - lastMousePos.x
                var y = window.y
                var x = window.x + dx
                var height = window.height
                var width = window.width - dx

                if (x > window.x) {
                    width = window.width - dx / 2
                    if (width < window.minimumWidth) {
                        width = window.minimumWidth
                        x = window.x
                    }

                    else if (width > window.maximumWidth) {
                        width = window.maximumWidth
                        x = window.x
                    }
                }

                window.setGeometry(x, y, width, height)
            }
        }
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

        height: handleSize
        enabled: Cpp_IsMac
        onPressedChanged: dragging = pressed
        onPressed: lastMousePos = Qt.point(mouseX, mouseY)
        onMouseYChanged: {
            if (dragging) {
                var dy = mouseY - lastMousePos.y
                var height = window.height + dy
                if (height < window.minimumHeight)
                    height = window.minimumHeight
                else if (height > window.maximumHeight)
                    height = window.maximumHeight

                window.setGeometry(window.x, window.y, window.width, height)
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
                var width = window.width + dx
                var height = window.height + dy

                if (width < window.minimumWidth)
                    width = window.minimumWidth
                else if (width > window.maximumWidth)
                    width = window.maximumWidth

                if (height < window.minimumHeight)
                    height = window.minimumHeight
                else if (height > window.maximumHeight)
                    height = window.maximumHeight

                window.setGeometry(window.x, window.y, width, height)
            }
        }

        anchors {
            right: parent.right
            bottom: parent.bottom
        }

        width: handleSize
        height: handleSize
        enabled: Cpp_IsMac
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

                var y = window.y
                var x = window.x + dx
                var width = window.width - dx
                var height = window.height + dy

                if (x > window.x)
                    width = window.width - dx / 2

                if (width < window.minimumWidth) {
                    width = window.minimumWidth
                    x = window.x
                }

                else if (width > window.maximumWidth) {
                    width = window.maximumWidth
                    x = window.x
                }

                if (height < minimumHeight)
                    height = minimumHeight
                else if (height > maximumHeight)
                    height = maximumHeight

                window.setGeometry(x, y, width, height)
            }
        }

        anchors {
            left: parent.left
            bottom: parent.bottom
        }

        width: handleSize
        height: handleSize
        enabled: Cpp_IsMac
        onPressedChanged: dragging = pressed
        onMouseXChanged: updateWindowPosition()
        onPressed: lastMousePos = Qt.point(mouseX, mouseY)
    }
}
