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
import QtQuick.Controls

ToolButton {
  id: root

  width: 24
  height: 24
  icon.width: 18
  icon.height: 18
  icon.color: "transparent"

  ToolTip {
    delay: 700
    parent: root
    text: root.ToolTip.text
    x: _mouseArea.mouseX + 10
    y: _mouseArea.mouseY + 10
    visible: _mouseArea.containsMouse && root.ToolTip.text !== ""
  }

  MouseArea {
    id: _mouseArea

    hoverEnabled: true
    anchors.fill: parent
    acceptedButtons: Qt.NoButton
  }
}
