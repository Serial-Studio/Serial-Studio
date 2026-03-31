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
import QtQuick.Controls

SplitView {
  id: root

  orientation: Qt.Horizontal

  //
  // Public API
  //
  property int minLeftWidth: 200
  property int minRightWidth: 200
  property string settingsKey: ""
  property Component leftPanel: null
  property Component rightPanel: null

  //
  // 1px handle matching the app theme
  //
  handle: Rectangle {
    implicitWidth: 1
    color: Cpp_ThemeManager.colors["setup_border"]

    Rectangle {
      height: 32
      color: Cpp_ThemeManager.colors["pane_caption_border"]
      anchors {
        top: parent.top
        left: parent.left
        right: parent.right
      }
    }
  }

  //
  // Persist user-chosen width
  //
  Settings {
    id: settings
    category: root.settingsKey !== "" ? root.settingsKey : "PaneSplitter"
    property alias panelWidth: leftLoader.preferredWidth
  }

  //
  // Internal: track preferred width for persistence
  //
  property int _defaultLeftWidth: root.minLeftWidth

  //
  // Left pane
  //
  Loader {
    id: leftLoader

    property int preferredWidth: root._defaultLeftWidth

    SplitView.fillHeight: true
    SplitView.minimumWidth: root.minLeftWidth
    SplitView.preferredWidth: preferredWidth

    sourceComponent: root.leftPanel

    onWidthChanged: {
      if (root.width > 0)
        preferredWidth = width
    }
  }

  //
  // Right pane
  //
  Loader {
    id: rightLoader

    SplitView.fillWidth: true
    SplitView.fillHeight: true
    SplitView.minimumWidth: root.minRightWidth

    sourceComponent: root.rightPanel
  }
}
