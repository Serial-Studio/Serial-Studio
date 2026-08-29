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
import QtQuick.Controls

//
// Small block drawn between a group card and a dataset pill when the dataset
// carries a value transform.
//
Rectangle {
  id: block

  //
  // Input properties
  //
  required property bool active
  required property real zoom
  required property bool showTooltip

  radius: 4 * block.zoom
  color: block.active
    ? Cpp_ThemeManager.colors["highlight"]
    : Cpp_ThemeManager.colors["groupbox_background"]
  border.width: 1
  border.color: block.active
    ? Cpp_ThemeManager.colors["highlight"]
    : Cpp_ThemeManager.colors["groupbox_border"]

  Image {
    smooth: true
    width: 14 * block.zoom
    height: 14 * block.zoom
    anchors.centerIn: parent
    sourceSize: Qt.size(14, 14)
    opacity: block.active ? 1.0 : 0.85
    source: Cpp_Misc_IconRegistry.icon("editor", "transform", 16)
  }

  ToolTip.delay: 400
  ToolTip.visible: block.showTooltip
  ToolTip.text: qsTr("Open the transform code editor for this dataset.")
}
