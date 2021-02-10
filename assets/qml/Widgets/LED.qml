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
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

RowLayout {
    id: root
    spacing: app.spacing

    //
    // Custom properties
    //
    property alias label: _label
    property alias indicator: _dot
    property alias text: _label.text
    property alias font: _label.font
    property color onColor: "#d72d60"
    property color offColor: "#2d6073"
    property alias flashDuration: _timer.interval

    //
    //*! Optimize this function
    // Turns on the LED for a short period of time
    //
    function flash() {
        root.enabled = true
        if (_timer.running)
            _timer.restart()
        else
            _timer.start()
    }

    //
    // Used to allow the flash behavior
    //
    Timer {
        id: _timer
        interval: 50
        onTriggered: root.enabled = false
    }

    //
    // LED indicator
    //
    Rectangle {
        id: _dot
        width: 18
        height: 18
        radius: width / 2
        Layout.alignment: Qt.AlignVCenter
        color: root.enabled ? root.onColor : root.offColor
    }

    //
    // LED text
    //
    Label {
        id: _label
        font.family: app.monoFont
        color: palette.highlightedText
        Layout.alignment: Qt.AlignVCenter
    }
}
