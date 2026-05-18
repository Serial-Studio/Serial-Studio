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

  LayoutMirroring.enabled: false
  LayoutMirroring.childrenInherit: true

  property alias text: textField.text
  property alias placeholderText: textField.placeholderText

  readonly property bool rtl: Cpp_Misc_Translator.rtl

  LineField {
    id: textField

    background: Item {}
    anchors.fill: parent
    anchors.leftMargin: 4
    anchors.rightMargin: 4
    font: Cpp_Misc_CommonFonts.uiFont

    leftPadding: root.rtl ? actionButton.width + 12 : 0
    rightPadding: root.rtl ? 0 : actionButton.width + 12

    IconButton {
      id: actionButton

      opacity: 0.8
      iconSize: 12
      background: Item {}
      anchors.verticalCenter: parent.verticalCenter
      anchors.left: root.rtl ? parent.left : undefined
      anchors.right: root.rtl ? undefined : parent.right
      anchors.leftMargin: root.rtl ? 4 : 0
      anchors.rightMargin: root.rtl ? 0 : 4
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
