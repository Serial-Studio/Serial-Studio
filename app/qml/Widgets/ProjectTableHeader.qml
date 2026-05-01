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

// Project table header: gradient row of bold labels driven by `columns`.
Rectangle {
  id: header

  property var columns: []
  property int rowHeight: 30

  implicitHeight: rowHeight

  gradient: Gradient {
    GradientStop { position: 0; color: Cpp_ThemeManager.colors["table_bg_header_top"] }
    GradientStop { position: 1; color: Cpp_ThemeManager.colors["table_bg_header_bottom"] }
  }

  //
  // 1-px bottom border separates the header from the first row
  //
  Rectangle {
    height: 1
    width: parent.width
    anchors.bottom: parent.bottom
    color: Cpp_ThemeManager.colors["table_border_header"]
  }

  //
  // Labels and 1-px separators interleaved to match the row delegate geometry.
  //
  RowLayout {
    spacing: 0
    anchors.fill: parent

    Repeater {
      model: header.columns.length * 2 - 1

      delegate: Loader {
        readonly property int columnIndex: Math.floor(index / 2)
        readonly property var columnData: header.columns[columnIndex]
        readonly property bool isSeparator: (index % 2) === 1

        Layout.fillHeight: true
        Layout.fillWidth: !isSeparator && columnData && columnData.width < 0
        Layout.preferredWidth: isSeparator
                                 ? 1
                                 : (columnData && columnData.width >= 0 ? columnData.width : 0)

        sourceComponent: isSeparator ? separatorComponent : labelComponent

        Component {
          id: labelComponent

          Label {
            leftPadding: 8
            rightPadding: 8
            verticalAlignment: Qt.AlignVCenter
            text: columnData ? (columnData.title || "") : ""
            elide: Text.ElideRight
            font: Cpp_Misc_CommonFonts.boldUiFont
            color: Cpp_ThemeManager.colors["table_fg_header"]
          }
        }

        Component {
          id: separatorComponent

          Rectangle {
            color: Cpp_ThemeManager.colors["table_separator"]
          }
        }
      }
    }
  }
}
