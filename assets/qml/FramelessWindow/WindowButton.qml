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
import "../Widgets" as Widgets

Widgets.Icon {
    id: root

    signal clicked()
    property string name
    property color textColor
    property color pressedColor: Cpp_ThemeManager.highlight

    width: 24
    height: 24
    color: root.textColor
    source: "qrc:/window-border/" + name + ".svg"

    Behavior on color { ColorAnimation{} }

    MouseArea {
        hoverEnabled: true
        anchors.fill: parent
        preventStealing: true
        onClicked: root.clicked()
        acceptedButtons: Qt.LeftButton
        onContainsMouseChanged: root.color = containsMouse ? root.pressedColor : root.textColor
    }
}
