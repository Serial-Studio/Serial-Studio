/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
import QtQuick.Controls

//
// A tab that marks itself open with the theme's tab indicator (spec 0113 R8, R12): a bar along
// its bottom edge over a soft glow. A theme without the rule draws the plain tab, as before.
//
TabButton {
  id: root

  readonly property var rule: Cpp_ThemeManager.styles["tab"] || ({})
  readonly property bool marked: root.checked && root.rule.indicator !== undefined

  Rectangle {
    z: 1
    height: 7
    opacity: 0.35
    radius: height / 2
    visible: root.marked
    color: root.marked ? root.rule.indicator : "transparent"
    anchors {
      leftMargin: 2
      rightMargin: 2
      bottomMargin: -1
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
  }

  Rectangle {
    z: 2
    height: 3
    radius: 1.5
    visible: root.marked
    color: root.marked ? root.rule.indicator : "transparent"
    anchors {
      leftMargin: 4
      rightMargin: 4
      bottomMargin: 1
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
  }
}
