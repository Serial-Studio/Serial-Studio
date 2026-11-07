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
import QtQuick.Effects

//
// Shared background of every popup rising from the taskbar: menus stay square, cards take a radius
//
Item {
  id: root

  default property alias content: _panel.data

  property real radius: 0
  property bool shadowEnabled: true
  readonly property alias panel: _panel

  //
  // Standalone shader item: no layer around the panel, so HDR content behind it never clamps
  //
  RectangularShadow {
    blur: 36
    spread: 0
    offset.y: 2
    anchors.fill: _panel
    radius: _panel.radius
    color: Qt.rgba(0, 0, 0, 0.22)
    visible: root.shadowEnabled && Cpp_Misc_GraphicsBackend.effectsEnabled
  }

  Rectangle {
    id: _panel

    border.width: 1
    radius: root.radius
    anchors.fill: parent
    color: Cpp_ThemeManager.colors["start_menu_background"]
    border.color: Cpp_ThemeManager.colors["start_menu_border"]
  }
}
