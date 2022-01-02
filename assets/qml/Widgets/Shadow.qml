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

Item {
    id: root
    property real topMargin: -8
    property real leftMargin: -8
    property real rightMargin: -10
    property real bottomMargin: -10
    property alias border: borderImage.border
    property alias source: borderImage.source
    property alias shadowOpacity: borderImage.opacity

    BorderImage {
        id: borderImage

        opacity: 0.5
        smooth: true
        anchors.fill: parent
        source: "qrc:/images/shadow.png"
        verticalTileMode: BorderImage.Round
        horizontalTileMode: BorderImage.Round

        anchors {
            topMargin: root.topMargin
            leftMargin: root.leftMargin
            rightMargin: root.rightMargin
            bottomMargin: root.bottomMargin
        }

        border {
            left: 10
            top: 10
            right: 10
            bottom: 10
        }
    }
}
