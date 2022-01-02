/*
 * Copyright (c) 2021 Alex Spataru <https://github.com/alex-spataru>
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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import SerialStudio 1.0

import "../Widgets" as Widgets
import "../FramelessWindow" as FramelessWindow

Item {
    id: root

    property int widgetIndex: -1
    property FramelessWindow.CustomWindow externalWindow: null

    Widgets.Window {
        id: window
        anchors.fill: parent
        title: widget.widgetTitle
        icon.source: widget.widgetIcon
        headerDoubleClickEnabled: true
        borderColor: Cpp_ThemeManager.widgetWindowBorder
        onHeaderDoubleClicked: {
            if (root.externalWindow !== null)
                root.externalWindow.showNormal()
            else
                externalWindowLoader.active = true
        }

        DashboardWidget {
            id: widget
            widgetIndex: root.widgetIndex
            anchors {
                fill: parent
                leftMargin: window.borderWidth
                rightMargin: window.borderWidth
                bottomMargin: window.borderWidth
            }
        }
    }

    Loader {
        id: externalWindowLoader

        active: false
        asynchronous: true

        sourceComponent: FramelessWindow.CustomWindow {
            id: _window
            minimumWidth: 640 + shadowMargin
            minimumHeight: 480 + shadowMargin
            title: externalWidget.widgetTitle
            titlebarText: Cpp_ThemeManager.text
            titlebarColor: Cpp_ThemeManager.widgetWindowBackground
            backgroundColor: Cpp_ThemeManager.widgetWindowBackground
            borderColor: isMaximized ? backgroundColor : Cpp_ThemeManager.highlight
            extraFlags: Qt.WindowStaysOnTopHint | Qt.Dialog | Qt.WindowCloseButtonHint | Qt.WindowTitleHint

            Timer {
                id: timer
                interval: 200
                onTriggered: _window.showNormal()
            }

            Component.onCompleted: {
                root.externalWindow = this
                timer.start()
            }

            Rectangle {
                clip: true
                anchors.fill: parent
                radius: _window.radius
                anchors.margins: _window.shadowMargin
                color: Cpp_ThemeManager.widgetWindowBackground
                anchors.topMargin: _window.titlebar.height + _window.shadowMargin

                Rectangle {
                    height: _window.radius
                    color: Cpp_ThemeManager.widgetWindowBackground
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                    }
                }

                DashboardWidget {
                    id: externalWidget
                    anchors.fill: parent
                    isExternalWindow: true
                    widgetIndex: root.widgetIndex
                    widgetVisible: _window.visible
                    anchors.margins: _window.radius
                }
            }

            FramelessWindow.ResizeHandles {
                window: _window
                anchors.fill: parent
                handleSize: _window.handleSize
            }
        }
    }
}
