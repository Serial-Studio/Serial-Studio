/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls

Controls.GroupBox {
  id: root
  bottomPadding: 8
  topPadding: label.height + 16

  property string icon: ""
  property bool headerVisible: true

  property Component actionComponent

  label: Controls.ToolBar {
    z: 2000
    width: root.width
    height: visible ? 32 : 0
    visible: root.headerVisible

    background: Rectangle {
      border.width: 1
      border.color: Cpp_ThemeManager.colors["pane_caption_border"]

      Rectangle {
        gradient: Gradient {
          GradientStop {
            position: 0
            color: Cpp_ThemeManager.colors["pane_caption_bg_top"]
          }

          GradientStop {
            position: 1
            color: Cpp_ThemeManager.colors["pane_caption_bg_bottom"]
          }
        }

        anchors {
          fill: parent
          topMargin: 1
          leftMargin: 0
          rightMargin: 0
          bottomMargin: 1
        }
      }

      Rectangle {
        height: 1
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        color: Cpp_ThemeManager.colors["pane_caption_border"]
      }

      Rectangle {
        height: 1
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: Cpp_ThemeManager.colors["pane_caption_border"]
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
        text: root.title
        elide: Qt.ElideRight
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        horizontalAlignment: Qt.AlignLeft
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["pane_caption_foreground"]
      }

      Loader {
        Layout.alignment: Qt.AlignVCenter
        sourceComponent: root.actionComponent
        visible: root.actionComponent !== null
      }
    }
  }

  background: Rectangle {
    color: Cpp_ThemeManager.colors["pane_background"]

    anchors {
      fill: parent
      topMargin: label.height
    }
  }
}
