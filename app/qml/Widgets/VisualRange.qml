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
  // Uses C++ side formatting logic to compute the longest label,
  // ensuring visual alignment and consistent layout.
  //
  function maxTextLength() {
    const minStr = Cpp_UI_Dashboard.formatValue(root.minValue, root.minValue, root.maxValue)
    const maxStr = Cpp_UI_Dashboard.formatValue(root.maxValue, root.minValue, root.maxValue)
    const valStr = Cpp_UI_Dashboard.formatValue(root.value, root.minValue, root.maxValue)

    let maxLen = Math.max(minStr.length, maxStr.length, valStr.length)
    return maxLen
  }

  //
  // Pads the formatted value with spaces so it matches the width of the
  // largest possible formatted value (min/max/value). Useful for monospaced
  // alignment in visuals.
  //
  function getPaddedText(value) {
    const referenceLen = maxTextLength()
    const valueText = Cpp_UI_Dashboard.formatValue(value, root.minValue, root.maxValue)
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
