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

//
// Standard row container used by every project-editor table view.
//
// Wraps the user's row content in a Rectangle that paints `table_cell_bg` and
// a 1-px bottom border. The actual cell layout is the user's responsibility —
// they place their cells inside this component and bind widths to the same
// values declared in the matching ProjectTableHeader's `columns` array.
//
// Usage as a ListView delegate:
//
//   ListView {
//     model: items
//     delegate: Widgets.ProjectTableRow {
//       RowLayout {
//         spacing: 0
//         anchors.fill: parent
//         Label { Layout.preferredWidth: 80;  text: modelData.id }
//         Rectangle { width: 1; Layout.fillHeight: true; color: separatorColor }
//         Label { Layout.preferredWidth: 200; text: modelData.group }
//       }
//     }
//   }
//
// The row exposes `separatorColor` and `textColor` for delegates that draw
// their own vertical separators or styled labels.
//
Rectangle {
  id: row

  //
  // Theme passthroughs — saves callers the long `Cpp_ThemeManager.colors[…]`
  // lookups when wiring the cells.
  //
  readonly property color separatorColor: Cpp_ThemeManager.colors["table_separator"]
  readonly property color textColor: Cpp_ThemeManager.colors["table_text"]
  readonly property color highlightColor: Cpp_ThemeManager.colors["highlight"]
  readonly property color headerTopColor: Cpp_ThemeManager.colors["table_bg_header_top"]
  readonly property color headerBottomColor: Cpp_ThemeManager.colors["table_bg_header_bottom"]
  readonly property color headerTextColor: Cpp_ThemeManager.colors["table_fg_header"]

  property int rowHeight: 30

  //
  // Default sizing — picks up the parent ListView's width and the standard
  // 30-px row height.
  //
  width: ListView.view ? ListView.view.width : 0
  implicitHeight: rowHeight
  color: Cpp_ThemeManager.colors["table_cell_bg"]

  //
  // 1-px bottom border separating this row from the next.
  //
  Rectangle {
    height: 1
    width: parent.width
    anchors.bottom: parent.bottom
    color: row.separatorColor
  }
}
