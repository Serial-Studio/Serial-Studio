/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
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
import QtQuick.Layouts
import QtQuick.Controls

ToolButton {
  id: root

  property bool toolbarButton: true

  icon.width: 32
  icon.height: 32
  icon.color: "transparent"
  display: AbstractButton.TextUnderIcon
  palette.buttonText: Cpp_ThemeManager.colors["toolbar_text"]

  Layout.minimumWidth: implicitWidth
  Layout.maximumWidth: implicitWidth
  implicitWidth: Math.max(Math.ceil(metrics.width + 32), icon.width / 32 * 72)

  opacity: enabled ? 1 : 0.5
  Behavior on opacity {NumberAnimation{}}

  TextMetrics {
    id: metrics
    text: root.text
    font: Cpp_Misc_CommonFonts.uiFont
  }

  background: Item {
    Rectangle {
      radius: 3
      border.width: 1
      anchors.fill: parent
      visible: root.toolbarButton
      color: Cpp_ThemeManager.colors["toolbar_checked_button_background"]
      border.color: Cpp_ThemeManager.colors["toolbar_checked_button_border"]
      opacity: root.checked ? Cpp_ThemeManager.colors["toolbar_checked_button_opacity"] : 0.0
    }

    ToolButton {
      checked: true
      anchors.fill: parent
      visible: root.checked && !root.toolbarButton
    }
  }
}
