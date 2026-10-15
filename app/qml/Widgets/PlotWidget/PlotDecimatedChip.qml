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
// Spec 0097 R7: shown once the plot history decimates (curve ceiling or memory rail)
//
Rectangle {
  id: root

  //
  // The PlotWidget served and whether its history still holds every sample
  //
  required property Item plot
  required property bool exact

  z: 10
  radius: 3
  visible: !root.exact
  x: root.plot.curveLayer.x + 4
  y: root.plot.curveLayer.y + 4
  width: _label.implicitWidth + 12
  height: _label.implicitHeight + 4
  color: Cpp_ThemeManager.colors["widget_base"]
  border.color: Cpp_ThemeManager.colors["widget_border"]

  Label {
    id: _label

    anchors.centerIn: parent
    text: qsTr("Decimated")
    font: Cpp_Misc_CommonFonts.monoFont
    color: Cpp_ThemeManager.colors["widget_text"]
  }

  HoverHandler {
    id: _hover
  }

  ToolTip.delay: 300
  ToolTip.visible: _hover.hovered
  ToolTip.text: qsTr("History limit reached: zooming shows min/max per cell, not every sample")
}
