/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick

//
// Rectangular card for source, group, frame-parser, action, table, control
// script and MQTT publisher nodes.
//
Rectangle {
  id: card

  //
  // Input properties
  //
  required property var node
  required property bool active
  required property real zoom
  required property bool isSource
  required property bool isFP
  required property bool badgeVisible

  radius: 6 * card.zoom
  color: card.active
    ? Cpp_ThemeManager.colors["highlight"]
    : Cpp_ThemeManager.colors["groupbox_background"]
  border.width: 1
  border.color: card.active
    ? Cpp_ThemeManager.colors["highlight"]
    : Cpp_ThemeManager.colors["groupbox_border"]

  Row {
    anchors {
      left: parent.left
      right: parent.right
      leftMargin:  10 * card.zoom
      rightMargin:  8 * card.zoom
      verticalCenter: parent.verticalCenter
    }
    spacing: 8 * card.zoom

    Image {
      smooth: true
      width:  20 * card.zoom
      height: 20 * card.zoom
      source: card.node.icon
      sourceSize: Qt.size(20, 20)
      anchors.verticalCenter: parent.verticalCenter
      opacity: card.isFP ? 0.7 : (card.active ? 1.0 : 0.85)
    }

    Text {
      width: parent.width - 36 * card.zoom
      elide: Text.ElideRight
      anchors.verticalCenter: parent.verticalCenter
      text: card.node.label
      font.pixelSize: Math.max(8, 12 * card.zoom)
      font.bold: card.isSource
      font.italic: card.isFP
      color: card.active
        ? Cpp_ThemeManager.colors["highlighted_text"]
        : (card.isFP
            ? Qt.lighter(Cpp_ThemeManager.colors["text"], 1.3)
            : Cpp_ThemeManager.colors["text"])
    }
  }

  //
  // Badge in the corner: "[A]"/"[B]" on source cards,
  // "N regs"/"empty" on table cards.
  //
  Text {
    visible: card.badgeVisible && (card.node.badge || "") !== ""
    anchors { right: parent.right; bottom: parent.bottom; margins: 4 * card.zoom }
    text: card.node.badge || ""
    font.family: Cpp_Misc_CommonFonts.monoFont.family
    font.pixelSize: Math.max(7, 9 * card.zoom)
    opacity: 0.6
    color: card.active
      ? Cpp_ThemeManager.colors["highlighted_text"]
      : Cpp_ThemeManager.colors["text"]
  }
}
