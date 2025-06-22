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
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
  id: root
  spacing: 8
  Layout.maximumWidth: parent ? parent.width : implicitWidth

  property alias text: label.text
  property alias bulletText: bullet.text
  property alias bulletColor: bullet.color

  Label {
    id: bullet
    text: "✔"
    color: "#27AE60"
    font: Cpp_Misc_CommonFonts.customUiFont(1.33, true)
  }

  Label {
    id: label
    Layout.fillWidth: true
    wrapMode: Label.WordWrap
  }
}
