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
import QtQuick.Controls as Controls

Controls.GroupBox {
  id: root
  bottomPadding: 8
  topPadding: label.height + 16

  property string icon: ""
  property bool hardBorder: false
  property bool headerVisible: true

  label: Controls.ToolBar {
    z: 2000
    width: root.width
    height: visible ? 32 : 0
    visible: root.headerVisible

    background: Rectangle {
      border.width: 1
      border.color: root.hardBorder ?
                      Cpp_ThemeManager.colors["pane_hard_caption_border"] :
                      Cpp_ThemeManager.colors["groupbox_hard_border"]

      Rectangle {
        gradient: Gradient {
          GradientStop {
            position: 0
            color: root.hardBorder ?
                     Cpp_ThemeManager.colors["pane_hard_caption_bg_top"] :
                     Cpp_ThemeManager.colors["pane_caption_bg_top"]
          }

          GradientStop {
            position: 1
            color: root.hardBorder ?
                     Cpp_ThemeManager.colors["pane_hard_caption_bg_bottom"] :
                     Cpp_ThemeManager.colors["pane_caption_bg_bottom"]
          }
        }

        anchors {
          fill: parent
          topMargin: 1
          bottomMargin: 1
          leftMargin: root.hardBorder ? 1 : 0
          rightMargin: root.hardBorder ? 1 : 0
        }
      }

      Rectangle {
        height: 1
        visible: !root.hardBorder
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        color: root.hardBorder ?
                 Cpp_ThemeManager.colors["pane_hard_caption_border"] :
                 Cpp_ThemeManager.colors["pane_caption_border"]
      }

      Rectangle {
        height: 1
        visible: !root.hardBorder
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: root.hardBorder ?
                 Cpp_ThemeManager.colors["pane_hard_caption_border"] :
                 Cpp_ThemeManager.colors["pane_caption_border"]
      }
    }

    RowLayout {
      spacing: 4
      anchors.leftMargin: 0
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter

      Image {
        source: root.icon
        visible: !root.hardBorder
        sourceSize: Qt.size(16, 16)
        Layout.alignment: Qt.AlignVCenter
      }

      Controls.ToolButton {
        flat: true
        enabled: false
        icon.width: 24
        icon.height: 24
        background: Item {}
        Layout.leftMargin: -8
        icon.source: root.icon
        Layout.minimumWidth: 32
        Layout.maximumWidth: 32
        visible: root.hardBorder
        Layout.alignment: Qt.AlignVCenter
        icon.color: root.hardBorder ?
                      Cpp_ThemeManager.colors["pane_hard_caption_foreground"] :
                      Cpp_ThemeManager.colors["pane_caption_foreground"]
      }

      Controls.Label {
        id: _title
        text: root.title
        Layout.alignment: Qt.AlignVCenter
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.leftMargin: root.hardBorder ? -6 : undefined
        color: root.hardBorder ?
                 Cpp_ThemeManager.colors["pane_hard_caption_foreground"] :
                 Cpp_ThemeManager.colors["pane_caption_foreground"]
      }

      Item {
        Layout.fillWidth: true
      }
    }
  }

  background: Rectangle {
    border.width: root.hardBorder ? 1 : 0
    border.color: Cpp_ThemeManager.colors["groupbox_hard_border"]
    color: root.hardBorder ? Cpp_ThemeManager.colors["groupbox_background"] :
                             Cpp_ThemeManager.colors["pane_background"]

    anchors {
      fill: parent
      topMargin: root.hardBorder ? label.height - 1 : label.height
    }
  }
}
