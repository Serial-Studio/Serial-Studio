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
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
  id: root

  spacing: 0
  implicitWidth: labelBg.width

  //
  // Custom Properties
  //
  // value: current numeric value to display
  // minValue: minimum range value (shown at bottom if rangeVisible)
  // maxValue: maximum range value (shown at top if rangeVisible)
  // units: optional unit string to append to displayed values
  // alarm: flag to indicate alarm state (changes color theme)
  // textValue: optional text override for value display
  // rangeVisible: controls visibility of min/max labels and connectors
  // maximumWidth: maximum width allowed for labels
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
  // Calculates the maximum string length (including digits,
  // decimal point, and minus sign) required to display
  // the minValue or maxValue, based on configured precision.
  // This is used to determine how much horizontal space
  // to reserve for the value display.
  //
  function maxTextLength() {
    const precision = Cpp_UI_Dashboard.precision
    const minStr = root.minValue.toFixed(precision)
    const maxStr = root.maxValue.toFixed(precision)
    let maxLen = Math.max(minStr.length, maxStr.length)
    if (root.minValue >= 0 && root.maxValue >= 0 && root.value < 0) {
      maxLen += 1
    }
    return maxLen
  }

  //
  // Pads the numeric value with left spaces to match the maximum
  // reference length calculated by maxTextLength(). This ensures
  // consistent visual width regardless of sign or digit count.
  //
  function getPaddedText(value) {
    const precision = Cpp_UI_Dashboard.precision
    const referenceLen = maxTextLength()
    let valueText = value.toFixed(precision)
    const padding = " ".repeat(Math.max(0, referenceLen - valueText.length))
    return padding + valueText
  }

  //
  // Measures the pixel width of text strings based on the font
  // used by valueLabel. This allows dynamic calculation of
  // label width to prevent layout shifts.
  //
  FontMetrics {
    id: fontMetrics
    font: valueLabel.font
  }

  //
  // Displays the maximum value at the top of the widget.
  // Uses monospaced font for alignment and allows eliding
  // if the text overflows the maximum width.
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
  // Spacer (4 px)
  //
  Item { implicitHeight: 4 }

  //
  // Top Connector Line
  //
  Rectangle {
    implicitWidth: 2
    Layout.fillHeight: true
    visible: root.rangeVisible
    Layout.alignment: Qt.AlignHCenter
    color: root.alarm ? Cpp_ThemeManager.colors["alarm"] :
                        Cpp_ThemeManager.colors["widget_border"]
    Behavior on color { ColorAnimation{} }
  }

  //
  // Spacer (8 px)
  //
  Item { implicitHeight: 8 }

  //
  // Displays the current value in the center of the widget.
  // Uses a custom monospaced font that scales slightly when
  // rangeVisible is enabled. Calculates its width dynamically
  // based on the maximum padded text and reserves extra space
  // (+32 px) to account for background padding and margins.
  //
  Label {
    id: valueLabel
    elide: Qt.ElideRight
    Layout.alignment: Qt.AlignHCenter
    horizontalAlignment: Text.AlignHCenter
    font: root.rangeVisible ? Cpp_Misc_CommonFonts.customMonoFont(1.16) :
                              Cpp_Misc_CommonFonts.customMonoFont(1)
    color: Cpp_ThemeManager.colors["widget_text"]

    // Calculate max text width
    readonly property string paddedMaxText: root.getPaddedText(root.maxValue) + " " + root.units
    readonly property int labelWidth: fontMetrics.boundingRect(paddedMaxText).width + 32

    Layout.minimumWidth: labelWidth
    Layout.maximumWidth: labelWidth

    text: root.textValue !== "" ? root.textValue :
                                  root.getPaddedText(root.value) + " " + root.units

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
      Behavior on border.color { ColorAnimation{} }
    }
  }

  //
  // Spacer (8 px)
  //
  Item { implicitHeight: 8 }

  //
  // Bottom Connector Line
  //
  Rectangle {
    implicitWidth: 2
    Layout.fillHeight: true
    visible: root.rangeVisible
    Layout.alignment: Qt.AlignHCenter
    color: root.alarm ? Cpp_ThemeManager.colors["alarm"] :
                        Cpp_ThemeManager.colors["widget_border"]
    Behavior on color { ColorAnimation{} }
  }

  //
  // Spacer (4 px)
  //
  Item { implicitHeight: 4 }

  //
  // Displays the minimum value at the bottom of the widget.
  // Uses monospaced font for consistent alignment with the
  // max value label. Allows text eliding if the text exceeds
  // the maximum width.
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
