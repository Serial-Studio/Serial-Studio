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
// Rounded pill for dataset and output-widget nodes.
//
Rectangle {
  id: pill

  //
  // Input properties
  //
  required property var node
  required property bool active
  required property real zoom

  radius: height / 2
  color: pill.active
    ? Cpp_ThemeManager.colors["highlight"]
    : Cpp_ThemeManager.colors["groupbox_background"]
  border.width: 1
  border.color: pill.active
    ? Cpp_ThemeManager.colors["highlight"]
    : Cpp_ThemeManager.colors["groupbox_border"]

  Row {
    spacing: 4 * pill.zoom
    anchors.centerIn: parent
    width: parent.width - 12

    Image {
      visible: (pill.node.icon || "") !== ""
      width: 12 * pill.zoom
      height: 12 * pill.zoom
      anchors.verticalCenter: parent.verticalCenter
      source: pill.node.icon || ""
      sourceSize: Qt.size(12, 12)
      smooth: true
    }

    Text {
      width: parent.width - (pill.node.icon ? 16 * pill.zoom : 0)
      elide: Text.ElideRight
      anchors.verticalCenter: parent.verticalCenter
      text: pill.node.label
      font.pixelSize: Math.max(8, 11 * pill.zoom)
      color: pill.active
        ? Cpp_ThemeManager.colors["highlighted_text"]
        : Cpp_ThemeManager.colors["text"]
    }
  }
}
