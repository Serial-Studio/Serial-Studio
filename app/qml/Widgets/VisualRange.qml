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
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
  id: root

  spacing: 0
  implicitWidth: labelBg.width

  //
  // Custom properties
  //
  property real value: 0
  property real minValue: 0
  property real maxValue: 0
  property string units: ""
  property bool alarm: false
  property string textValue: ""
  property bool rangeVisible: false
  property real maximumWidth: root.width

  //
  // Helper function to calculate padding
  //
  function getPaddedText(value) {
    const valueText = value.toFixed(Cpp_UI_Dashboard.precision)
    const maxText = root.maxValue.toFixed(Cpp_UI_Dashboard.precision)
    const leftSpaces = " ".repeat(Math.max(0, maxText.length - valueText.length))
    return leftSpaces + valueText
  }

  //
  // Max value
  //
  Label {
    opacity: 0.8
    elide: Qt.ElideRight
    visible: root.rangeVisible
    Layout.alignment: Qt.AlignHCenter
    font: Cpp_Misc_CommonFonts.monoFont
    Layout.maximumWidth: root.maximumWidth
    horizontalAlignment: Text.AlignHCenter
    color: Cpp_ThemeManager.colors["widget_text"]
    text: root.maxValue.toFixed(Cpp_UI_Dashboard.precision) + " " + root.units
  }

  //
  // Spacer
  //
  Item {
    implicitHeight: 4
  }

  //
  // Connector line
  //
  Rectangle {
    implicitWidth: 2
    Layout.fillHeight: true
    visible: root.rangeVisible
    Layout.alignment: Qt.AlignHCenter
    color: root.alarm ? Cpp_ThemeManager.colors["alarm"] :
                        Cpp_ThemeManager.colors["widget_border"]

    Behavior on color {ColorAnimation{}}
  }

  //
  // Spacer
  //
  Item {
    implicitHeight: 8
  }

  //
  // Value display
  //
  Label {
    elide: Qt.ElideRight
    Layout.alignment: Qt.AlignHCenter
    Layout.maximumWidth: root.maximumWidth
    horizontalAlignment: Text.AlignHCenter
    color: Cpp_ThemeManager.colors["widget_text"]
    text: root.textValue !== "" ? root.textValue :
                                  root.getPaddedText(root.value) + " " + root.units
    font: root.rangeVisible ? Cpp_Misc_CommonFonts.customMonoFont(1.16) :
                              Cpp_Misc_CommonFonts.customMonoFont(1)

    background: Rectangle {
      z: 0
      radius: 5
      id: labelBg
      border.width: 2
      anchors.fill: parent
      anchors.margins: root.rangeVisible ? -8 : -4
      color: Cpp_ThemeManager.colors["widget_base"]
      border.color: root.alarm ? Cpp_ThemeManager.colors["alarm"] :
                                 Cpp_ThemeManager.colors["widget_border"]

      Behavior on border.color {ColorAnimation{}}
    }
  }

  //
  // Spacer
  //
  Item {
    implicitHeight: 8
  }

  //
  // Connector line
  //
  Rectangle {
    implicitWidth: 2
    Layout.fillHeight: true
    visible: root.rangeVisible
    Layout.alignment: Qt.AlignHCenter
    color: root.alarm ? Cpp_ThemeManager.colors["alarm"] :
                        Cpp_ThemeManager.colors["widget_border"]

    Behavior on color {ColorAnimation{}}
  }

  //
  // Spacer
  //
  Item {
    implicitHeight: 4
  }

  //
  // Minimum value
  //
  Label {
    opacity: 0.8
    elide: Qt.ElideRight
    visible: root.rangeVisible
    Layout.alignment: Qt.AlignHCenter
    font: Cpp_Misc_CommonFonts.monoFont
    Layout.maximumWidth: root.maximumWidth
    horizontalAlignment: Text.AlignHCenter
    color: Cpp_ThemeManager.colors["widget_text"]
    text: root.minValue.toFixed(Cpp_UI_Dashboard.precision) + " " + root.units
  }
}
