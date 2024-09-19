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

import "../../Widgets" as Widgets

Widgets.Pane {
  id: root
  title: qsTr("Console")
  icon: "qrc:/rcc/icons/panes/console.svg"

  //
  // Custom properties
  //
  property alias vt100emulation: terminal.vt100emulation

  //
  // Utility functions between terminal widget & main window
  //
  function sendData() {
    terminal.sendData()
  } function clear() {
    terminal.clear()
  } function copy() {
    terminal.copy()
  } function selectAll() {
    terminal.selectAll()
  }

  //
  // Use page item to set application palette
  //
  Page {
    anchors.fill: parent
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.windowText: Cpp_ThemeManager.colors["text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.highlight: Cpp_ThemeManager.colors["switch_highlight"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    Widgets.Terminal {
      id: terminal
      widgetEnabled: true
      anchors.fill: parent
    }
  }
}
