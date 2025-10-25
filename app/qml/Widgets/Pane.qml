/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
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
  property alias backgroundColor: _bg.color

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
    id: _bg
    color: Cpp_ThemeManager.colors["pane_background"]

    anchors {
      fill: parent
      topMargin: label.height
    }
  }
}
