/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
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
  property BarModel model
  property color color: Cpp_ThemeManager.colors["highlight"]

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
