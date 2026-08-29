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
// Folder card (group folders, table folders and the outputs category); the
// chevron shows the expand/collapse state.
//
Rectangle {
  id: folder

  //
  // Input properties
  //
  required property var node
  required property bool active
  required property real zoom

  radius: 6 * folder.zoom
  color: folder.active
    ? Cpp_ThemeManager.colors["highlight"]
    : Cpp_ThemeManager.colors["groupbox_background"]
  border.width: 1
  border.color: folder.active
    ? Cpp_ThemeManager.colors["highlight"]
    : Cpp_ThemeManager.colors["groupbox_border"]

  Row {
    spacing: 6 * folder.zoom
    anchors {
      left: parent.left
      right: parent.right
      leftMargin:  8 * folder.zoom
      rightMargin: 8 * folder.zoom
      verticalCenter: parent.verticalCenter
    }

    Image {
      smooth: true
      width:  9 * folder.zoom
      height: 9 * folder.zoom
      sourceSize: Qt.size(9, 9)
      rotation: folder.node.collapsed ? 270 : 0
      anchors.verticalCenter: parent.verticalCenter
      source: Cpp_Misc_IconRegistry.icon("editor", "indicator", 16)
    }

    Image {
      smooth: true
      width:  18 * folder.zoom
      height: 18 * folder.zoom
      source: folder.node.icon
      sourceSize: Qt.size(18, 18)
      opacity: folder.active ? 1.0 : 0.85
      anchors.verticalCenter: parent.verticalCenter
    }

    Text {
      width: parent.width - 45 * folder.zoom
      elide: Text.ElideRight
      text: folder.node.label
      anchors.verticalCenter: parent.verticalCenter
      font.bold: true
      font.pixelSize: Math.max(8, 12 * folder.zoom)
      color: folder.active
        ? Cpp_ThemeManager.colors["highlighted_text"]
        : Cpp_ThemeManager.colors["text"]
    }
  }
}
