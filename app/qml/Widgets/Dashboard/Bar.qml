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

import SerialStudio

import "../"

Item {
  id: root

  //
  // Widget data inputs
  //
  required property color color
  required property BarModel model
  required property var windowRoot

  //
  // Widget layout
  //
  RowLayout {
    spacing: 0
    anchors.margins: 8
    anchors.fill: parent

    //
    // Spacer
    //
    Item {
      Layout.fillWidth: true
    }

    //
    // Bar/Tank rectangle
    //
    Rectangle {
      id: tank
      clip: true
      border.width: 1
      Layout.fillHeight: true
      Layout.minimumWidth: root.width * 0.3
      Layout.maximumWidth: root.width * 0.3
      color: Cpp_ThemeManager.colors["widget_base"]
      border.color: Cpp_ThemeManager.colors["widget_border"]

      //
      // Normal level indicator
      //
      Rectangle {
        id: normalIndicator
        color: root.color
        width: parent.width - 2 * tank.border.width
        height: root.model.alarmFractionalValue > 0
                ? Math.min(root.model.fractionalValue, root.model.alarmFractionalValue) * (tank.height - 2 * tank.border.width)
                : root.model.fractionalValue * (tank.height - 2 * tank.border.width)

        anchors {
          left: parent.left
          right: parent.right
          bottom: parent.bottom
          margins: parent.border.width
        }

      }

      //
      // Alarm level indicator
      //
      Rectangle {
        color: Cpp_ThemeManager.colors["alarm"]
        width: parent.width - 2 * tank.border.width
        height: root.model.alarmFractionalValue > 0
                ? Math.max(0, root.model.fractionalValue - root.model.alarmFractionalValue) * (tank.height - 2 * tank.border.width)
                : 0

        anchors {
          bottomMargin: 0
          left: parent.left
          right: parent.right
          bottom: normalIndicator.top
          margins: parent.border.width
        }

        visible: root.model.alarmValue !== 0 && root.model.value >= root.model.alarmValue
      }
    }

    //
    // Spacer
    //
    Item {
      Layout.fillWidth: true
    }

    //
    // Spacer
    //
    Item {
      implicitWidth: 4
    }

    //
    // Range/scale + current value display
    //
    VisualRange {
      value: model.value
      units: model.units
      maxValue: model.maxValue
      minValue: model.minValue
      maximumWidth: root.width * 0.3
      rangeVisible: root.height >= 120
      alarm: root.model.alarmValue !== 0 && root.model.value >= root.model.alarmValue

      Layout.fillHeight: true
      Layout.minimumWidth: implicitWidth
    }

    //
    // Spacer
    //
    Item {
      implicitWidth: 8
    }
  }
}
