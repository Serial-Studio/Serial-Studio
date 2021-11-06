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
import "../PlatformDependent" as PlatformDependent

Item {
    id: root

    property int widgetIndex: -1

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

            MouseArea {
                hoverEnabled: true
                anchors.fill: parent
                acceptedButtons: Qt.NoButton
                onContainsMouseChanged: loader.processMouseHover(containsMouse)
            }
        }
    }

    PlatformDependent.CustomWindow {
        id: externalWindow
        minimumWidth: 640
        minimumHeight: 480
        titlebarBorderEnabled: false
        title: externalLoader.widgetTitle
        titlebarText: Cpp_ThemeManager.text
        titlebarColor: Cpp_ThemeManager.widgetWindowBackground
        backgroundColor: Cpp_ThemeManager.widgetWindowBackground

        WidgetLoader {
            id: externalLoader
            anchors.fill: parent
            isExternalWindow: true
            anchors.margins: windowBorder
            widgetIndex: root.widgetIndex
            anchors.topMargin: titlebar.height
            widgetVisible: externalWindow.visible

            MouseArea {
                hoverEnabled: true
                anchors.fill: parent
                acceptedButtons: Qt.NoButton
                anchors.topMargin: titlebar.height
                onContainsMouseChanged: externalLoader.processMouseHover(containsMouse)
            }
        }
    }
}
