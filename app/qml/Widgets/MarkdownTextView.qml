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

import "MarkdownIcons.js" as MarkdownIcons

//
// Plain rich-text markdown viewer used when WebEngine is unavailable.
// Renders Cpp markdown through Qt's built-in MarkdownText format.
//
Rectangle {
  id: root

  property string markdown: ""
  property string placeholderText: ""

  color: "transparent"

  ScrollView {
    id: scroll

    clip: true
    anchors.margins: 8
    anchors.fill: parent

    TextEdit {
      id: edit

      readOnly: true
      selectByMouse: true
      wrapMode: TextEdit.Wrap
      width: scroll.availableWidth
      textFormat: TextEdit.MarkdownText
      font: Cpp_Misc_CommonFonts.uiFont
      color: Cpp_ThemeManager.colors["text"]
      selectionColor: Cpp_ThemeManager.colors["highlight"]
      selectedTextColor: Cpp_ThemeManager.colors["highlighted_text"]
      text: root.markdown.length > 0 ? MarkdownIcons.resolve(root.markdown, 16)
                                     : root.placeholderText

      onLinkActivated: function(link) {
        Qt.openUrlExternally(link)
      }
    }
  }
}
