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
import SerialStudio 1.0

import "../Widgets" as Widgets

Item {
    id: root
    visible: false

    property int widgetIndex: -1

    Connections {
        target: Cpp_UI_Dashboard

        function onWidgetVisibilityChanged() {
            if (loader.status == Loader.Ready)
                root.visible = Cpp_UI_Dashboard.widgetVisible(root.widgetIndex)
        }
    }

    Loader {
        id: loader
        anchors.fill: parent
        onLoaded: root.visible = Cpp_UI_Dashboard.widgetVisible(root.widgetIndex)

        sourceComponent: Widgets.Window {
            id: window
            title: widget.widgetTitle
            icon.source: widget.widgetIcon
            headerDoubleClickEnabled: true
            borderColor: Cpp_ThemeManager.widgetWindowBorder
            onHeaderDoubleClicked: widget.showExternalWindow()

            WidgetLoader {
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
    }
}
