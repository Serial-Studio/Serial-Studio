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

  property real value: 0
  property real minValue: 0
  property real maxValue: 0
  property string units: ""
  property bool alarm: false
  property string textValue: ""
  property bool rangeVisible: false
  property real maximumWidth: root.width

  //
  // Longest formatted label length, for visual alignment.
  //
  function maxTextLength() {
    const minStr = Cpp_UI_Dashboard.formatValue(root.minValue, root.minValue, root.maxValue)
    const maxStr = Cpp_UI_Dashboard.formatValue(root.maxValue, root.minValue, root.maxValue)
    const valStr = Cpp_UI_Dashboard.formatValue(root.value, root.minValue, root.maxValue)

    let maxLen = Math.max(minStr.length, maxStr.length, valStr.length)
    return maxLen
  }

  //
  // Left-pad the formatted value to match the longest min/max/value label.
  //
  function getPaddedText(value) {
    const referenceLen = maxTextLength()
    const valueText = Cpp_UI_Dashboard.formatValue(value, root.minValue, root.maxValue)
    const padding = " ".repeat(Math.max(0, referenceLen - valueText.length))
    return padding + valueText
  }

  //
  // Pixel-width metrics matching valueLabel's font.
  //
  FontMetrics {
    id: fontMetrics

    font: valueLabel.font
  }

  //
  // Maximum value, rendered at the top.
  //
  Label {
    opacity: 0.8
    elide: Qt.ElideRight
    visible: root.rangeVisible
    Layout.alignment: Qt.AlignHCenter
    Layout.maximumWidth: root.maximumWidth
    horizontalAlignment: Text.AlignHCenter
    color: Cpp_ThemeManager.colors["widget_text"]
    font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont())
    text: Cpp_UI_Dashboard.formatValue(root.maxValue, root.minValue, root.maxValue) + " " + root.units
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
  // Current value (centered, font scales when rangeVisible).
  //
  Label {
    id: valueLabel

    elide: Qt.ElideRight
    Layout.alignment: Qt.AlignHCenter
    horizontalAlignment: Text.AlignHCenter
    color: Cpp_ThemeManager.colors["widget_text"]
    font: (Cpp_Misc_CommonFonts.widgetFontRevision,
           root.rangeVisible ? Cpp_Misc_CommonFonts.widgetFont(1.16) :
                               Cpp_Misc_CommonFonts.widgetFont(1))

    //
    // Calculate max text width
    //
    readonly property string paddedMaxText: root.getPaddedText(root.maxValue) + " " + root.units
    readonly property int labelWidth: Math.min(
      fontMetrics.boundingRect(paddedMaxText).width + 32,
      root.maximumWidth
    )

    Layout.minimumWidth: labelWidth
    Layout.maximumWidth: labelWidth

    text: root.textValue !== "" ? root.textValue :
                                  root.getPaddedText(root.value) + " " + root.units

    background: Rectangle {
      id: labelBg

      z: 0
      radius: 5
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
  // Minimum value, rendered at the bottom.
  //
  Label {
    opacity: 0.8
    elide: Qt.ElideRight
    visible: root.rangeVisible
    Layout.alignment: Qt.AlignHCenter
    Layout.maximumWidth: root.maximumWidth
    horizontalAlignment: Text.AlignHCenter
    color: Cpp_ThemeManager.colors["widget_text"]
    font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont())
    text: Cpp_UI_Dashboard.formatValue(root.minValue, root.minValue, root.maxValue) + " " + root.units
  }
}
