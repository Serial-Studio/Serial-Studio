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

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
  id: root
  spacing: 0

  //
  // Public API
  //
  property int minLeftWidth: 200
  property int minRightWidth: 200
  property string settingsKey: ""
  property Component leftPanel: null
  property Component rightPanel: null

  //
  // Internals
  //
  property int userWidth: 0
  readonly property int effectiveLeftWidth: {
    const w = userWidth > 0 ? Math.max(minLeftWidth, userWidth) : minLeftWidth
    if (root.width <= 0)
      return w
    return Math.min(w, root.width - minRightWidth - 1)
  }

  //
  // Persist user-chosen width
  //
  Settings {
    category: root.settingsKey !== "" ? root.settingsKey : "PaneSplitter"
    property alias panelWidth: root.userWidth
  }

  //
  // Left pane
  //
  Loader {
    id: leftLoader
    Layout.fillHeight: true
    Layout.preferredWidth: root.effectiveLeftWidth
    Layout.maximumWidth: Math.max(root.minLeftWidth, root.width - root.minRightWidth - 1)
    sourceComponent: root.leftPanel
  }

  //
  // Draggable separator
  //
  Rectangle {
    z: 2
    implicitWidth: 1
    Layout.fillHeight: true
    color: Cpp_ThemeManager.colors["setup_border"]

    Rectangle {
      width: 1
      height: 32
      anchors.top: parent.top
      anchors.left: parent.left
      color: Cpp_ThemeManager.colors["pane_caption_border"]
    }

    MouseArea {
      width: 8
      height: parent.height
      anchors.left: parent.left
      anchors.leftMargin: -4
      cursorShape: Qt.SizeHorCursor

      property int _startX: 0
      property int _startWidth: 0

      onPressed: (mouse) => {
        _startX = mouse.x
        _startWidth = leftLoader.width
      }

      onPositionChanged: (mouse) => {
        if (!pressed)
          return

        const maxW = root.width - root.minRightWidth - 1
        const newW = Math.max(root.minLeftWidth,
                              Math.min(_startWidth + (mouse.x - _startX), maxW))
        root.userWidth = newW
      }
    }
  }

  //
  // Right pane
  //
  Loader {
    id: rightLoader
    Layout.fillWidth: true
    Layout.fillHeight: true
    sourceComponent: root.rightPanel
  }
}
