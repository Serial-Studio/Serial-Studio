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

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

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
        title: loader.widgetTitle
        icon.source: loader.widgetIcon
        headerDoubleClickEnabled: true
        borderColor: Cpp_ThemeManager.widgetWindowBorder
        onHeaderDoubleClicked: {
            if (root.externalWindow !== null)
                root.externalWindow.showNormal()

            externalWindowLoader.enabled = true
        }

        WidgetLoader {
            id: loader
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

        enabled: false
        asynchronous: true

        sourceComponent: FramelessWindow.CustomWindow {
            id: dynamicWindow
            Component.onCompleted: root.externalWindow = this

            minimumWidth: 640 + shadowMargin
            minimumHeight: 480 + shadowMargin
            title: externalLoader.widgetTitle
            extraFlags: Qt.WindowStaysOnTopHint
            titlebarText: Cpp_ThemeManager.text
            titlebarColor: Cpp_ThemeManager.widgetWindowBackground
            backgroundColor: Cpp_ThemeManager.widgetWindowBackground
            borderColor: isMaximized ? backgroundColor : Cpp_ThemeManager.highlight

            Rectangle {
                clip: true
                anchors.fill: parent
                radius: externalWindow.radius
                anchors.margins: externalWindow.shadowMargin
                color: Cpp_ThemeManager.widgetWindowBackground
                anchors.topMargin: externalWindow.titlebar.height + externalWindow.shadowMargin

                Item {
                    anchors.fill: parent

                    DragHandler {
                        grabPermissions: TapHandler.CanTakeOverFromAnything
                        onActiveChanged: {
                            if (active)
                                externalWindow.startSystemMove()
                        }
                    }
                }

                Rectangle {
                    height: externalWindow.radius
                    color: Cpp_ThemeManager.widgetWindowBackground
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                    }
                }

                WidgetLoader {
                    id: externalLoader
                    anchors.fill: parent
                    isExternalWindow: true
                    widgetIndex: root.widgetIndex
                    widgetVisible: externalWindow.visible
                    anchors.margins: externalWindow.radius
                }
            }

            FramelessWindow.ResizeHandles {
                anchors.fill: parent
                window: dynamicWindow
                handleSize: dynamicWindow.handleSize
            }
        }
    }
}
