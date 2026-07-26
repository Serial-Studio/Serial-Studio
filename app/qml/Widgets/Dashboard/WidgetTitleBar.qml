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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick

Item {
  id: root

  property string text: ""
  property bool active: true

  readonly property bool shown: root.active
                                && root.text.length > 0
                                && root.width >= 96

  visible: shown
  height: implicitHeight
  implicitHeight: shown ? titleLabel.implicitHeight + 8 : 0

  Text {
    id: titleLabel

    text: root.text
    elide: Text.ElideRight
    anchors.centerIn: parent
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter
    color: Cpp_ThemeManager.colors["widget_text"]
    width: Math.min(implicitWidth, root.width * 0.85)
    font: Cpp_Misc_CommonFonts.customUiFont(1.15, true)
  }
}
