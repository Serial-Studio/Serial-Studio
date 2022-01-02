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

import QtQuick 2.5
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

MenuItem {
    id: root

    property alias sequence: _shortcut.sequence
    property bool indicatorVisible: root.icon.source.length > 0 || root.checkable

    Shortcut {
        id: _shortcut
        enabled: root.enabled
        onActivated: root.triggered()
    }

    contentItem: RowLayout {
        spacing: 0
        width: root.width
        opacity: root.enabled ? 1 : 0.5

        Item {
            width: root.indicatorVisible ? 18 : 0
        }

        Label {
            id: _titleLabel
            text: root.text
            Layout.fillWidth: true
            elide: Label.ElideRight
            verticalAlignment: Qt.AlignVCenter
            color: root.highlighted ? Cpp_ThemeManager.highlightedText : palette.text
        }

        Item {
            Layout.fillWidth: true
        }

        Label {
            id: _shortcutLabel
            opacity: 0.8
            text: _shortcut.nativeText
            verticalAlignment: Qt.AlignVCenter
            color: root.highlighted ? Cpp_ThemeManager.highlightedText : palette.text
        }
    }
}
