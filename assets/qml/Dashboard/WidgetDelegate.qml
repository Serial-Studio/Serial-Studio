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

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import "../Widgets" as Widgets

Item {
    id: root

    property int widgetIndex: -1

    Widgets.Shadow {
        source: window
        anchors.fill: window
    }

    Widgets.Window {
        id: window
        anchors.fill: parent
        title: loader.widgetTitle
        icon.source: loader.widgetIcon
        borderColor: Cpp_ThemeManager.widgetWindowBorder
        onHeaderDoubleClicked: externalWindow.visible = true

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

    Window {
        id: externalWindow
        minimumWidth: 640
        minimumHeight: 480
        title: externalLoader.widgetTitle
        palette.base: Cpp_ThemeManager.widgetWindowBackground
        palette.window: Cpp_ThemeManager.widgetWindowBackground
        flags: Qt.Dialog | Qt.WindowCloseButtonHint | Qt.WindowTitleHint

        WidgetLoader {
            id: externalLoader
            anchors.fill: parent
            isExternalWindow: true
            widgetIndex: root.widgetIndex
            widgetVisible: externalWindow.visible
        }
    }
}
