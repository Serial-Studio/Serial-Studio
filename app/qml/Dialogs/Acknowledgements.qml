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
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

Window {
  id: root

  //
  // Window options
  //
  width: minimumWidth
  height: minimumHeight
  title: qsTr("Acknowledgements")
  x: (Screen.desktopAvailableWidth - width) / 2
  y: (Screen.desktopAvailableHeight - height) / 2
  minimumWidth: column.implicitWidth + 32
  maximumWidth: column.implicitWidth + 32
  minimumHeight: column.implicitHeight + 32
  maximumHeight: column.implicitHeight + 32
  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.WindowTitleHint |
        Qt.WindowStaysOnTopHint |
        Qt.WindowCloseButtonHint
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

    //
    // Window controls
    //
    ColumnLayout {
      id: column
      spacing: 8
      anchors.centerIn: parent

      ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumWidth: 640
        Layout.maximumWidth: 640
        Layout.minimumHeight: 480
        Layout.maximumHeight: 480

        TextArea {
          readOnly: true
          textFormat: TextArea.MarkdownText
          text: Cpp_Misc_Translator.acknowledgementsText
          wrapMode: TextArea.WrapAtWordBoundaryOrAnywhere

          background: TextField {
            enabled: false
          }
        }
      }

      Button {
        text: qsTr("Close")
        onClicked: root.close()
        Layout.alignment: Qt.AlignRight
      }
    }
  }
}
