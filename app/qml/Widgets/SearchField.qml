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

Rectangle {
  id: root

  radius: 2
  border.width: 1
  implicitHeight: textField.implicitHeight + 8
  color: Cpp_ThemeManager.colors["groupbox_background"]
  border.color: textField.activeFocus ? Cpp_ThemeManager.colors["highlight"]
                                      : Cpp_ThemeManager.colors["groupbox_border"]

  property alias text: textField.text
  property alias placeholderText: textField.placeholderText

  TextField {
    id: textField

    background: Item {}
    anchors.fill: parent
    anchors.leftMargin: 4
    font: Cpp_Misc_CommonFonts.uiFont
    rightPadding: actionButton.width + 16

    Button {
      id: actionButton

      opacity: 0.8
      icon.width: 12
      icon.height: 12
      background: Item {}
      anchors.rightMargin: 4
      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter
      icon.color: Cpp_ThemeManager.colors["button_text"]
      icon.source: textField.text.length > 0
                     ? "qrc:/icons/buttons/close.svg"
                     : "qrc:/icons/buttons/search.svg"

      onClicked: {
        if (textField.text.length > 0)
          textField.clear()
      }

      HoverHandler {
        enabled: textField.text.length > 0
        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
      }
    }
  }
}
