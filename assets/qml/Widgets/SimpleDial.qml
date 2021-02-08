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
import QtQuick.Controls 2.0

Dial {
    id: control

    property string text: control.value.toFixed(1)
    property color dialColor: pressed ? palette.highlight : "#eee"

    background: Rectangle {
        border.width: 2
        radius: width / 2
        color: "transparent"
        border.color: control.dialColor
        x: control.width / 2 - width / 2
        y: control.height / 2 - height / 2
        width: Math.min(control.width, control.height)
        height: Math.min(control.width, control.height)

        Label {
            font.pixelSize: 12
            text: control.text
            color: control.dialColor
            anchors.centerIn: parent
            font.family: app.monoFont
        }
    } 

    handle: ToolButton {
        id: handleItem

        width: 42
        height: 42
        enabled: false
        icon.width: width
        icon.height: height
        icon.color: control.dialColor
        icon.source: "qrc:/icons/scroll-up.svg"
        x: control.background.x + control.background.width / 2 - width / 2
        y: control.background.y + control.background.height / 2 - height / 2

        transform: [
            Translate {
                y: -Math.min(control.background.width, control.background.height) * 0.55 + handleItem.height / 2
            },
            Rotation {
                angle: control.angle
                origin.x: handleItem.width / 2
                origin.y: handleItem.height / 2
            }
        ]
    }
}
