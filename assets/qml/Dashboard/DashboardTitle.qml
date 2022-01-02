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

import Qt.labs.settings 1.0

import "../Widgets" as Widgets

Rectangle {
    id: root
    radius: 5

    property alias consoleChecked: consoleBt.checked

    Settings {
        property alias consoleVisible: root.consoleChecked
    }

    gradient: Gradient {
        GradientStop {
            position: 0
            color: Cpp_ThemeManager.windowGradient1
        }

        GradientStop {
            position: 1
            color: Cpp_ThemeManager.windowGradient2
        }
    }

    RowLayout {
        spacing: app.spacing

        anchors {
            margins: 0
            left: parent.left
            right: parent.right
            leftMargin: app.spacing
            rightMargin: app.spacing
            verticalCenter: parent.verticalCenter
        }

        Widgets.Icon {
            Layout.alignment: Qt.AlignVCenter
            source: "qrc:/icons/arrow-right.svg"
        }

        Label {
            font.bold: true
            font.pixelSize: 16
            color: palette.brightText
            font.family: app.monoFont
            text: Cpp_UI_Dashboard.title
            Layout.alignment: Qt.AlignVCenter
        }

        Item {
            Layout.fillWidth: true
        }

        Button {
            flat: true
            id: consoleBt
            checkable: true
            font.bold: true
            text: qsTr("Console")
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/icons/code.svg"
            icon.color: Cpp_ThemeManager.menubarText
            palette.buttonText: Cpp_ThemeManager.menubarText
            palette.button: Cpp_ThemeManager.windowGradient1
            palette.window: Cpp_ThemeManager.windowGradient1
        }
    }
}
