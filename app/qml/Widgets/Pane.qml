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
  clip: true
  bottomPadding: 8
  topPadding: label.height + 16

  property string icon: ""
  property string buttonIcon: ""
  property bool hardBorder: false
  property bool headerVisible: true

  signal actionButtonClicked()

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
      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter

      Image {
        source: root.icon
        sourceSize: Qt.size(16, 16)
        Layout.alignment: Qt.AlignVCenter
      }

      Controls.Label {
        id: _title
        text: root.title
        elide: Qt.ElideRight
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        horizontalAlignment: Qt.AlignLeft
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: root.hardBorder ?
                 Cpp_ThemeManager.colors["pane_hard_caption_foreground"] :
                 Cpp_ThemeManager.colors["pane_caption_foreground"]
      }

      Controls.ToolButton {
        flat: true
        icon.width: 18
        icon.height: 18
        background: Item {}
        icon.color: _title.color
        visible: icon.source !== ""
        enabled: icon.source !== ""
        icon.source: root.buttonIcon
        onClicked: root.actionButtonClicked()
        opacity: mouseArea.containsMouse ? 1 : 0.5

        MouseArea {
          id: mouseArea
          hoverEnabled: true
          anchors.fill: parent
          onClicked: root.actionButtonClicked()
        }
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
