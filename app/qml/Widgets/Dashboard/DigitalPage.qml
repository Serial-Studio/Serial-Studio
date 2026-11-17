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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick

import "../"
import "ValueFormat.js" as ValueFormat

// Digital readout page of the Bar, Gauge and Meter; the widget binds opacity to its swipe position.
Item {
  id: root

  required property var model
  required property color accent
  required property string title
  required property bool titleFrozenOut

  clip: true
  visible: opacity > 0
  Behavior on opacity { NumberAnimation { duration: 150 } }

  TextMetrics {
    id: bigValueMetrics

    font.bold: true
    font.pixelSize: 100
    font.family: Cpp_Misc_CommonFonts.monoFont.family
    text: {
      const a = ValueFormat.formatValue(root.model, root.model.minValue)
      const b = ValueFormat.formatValue(root.model, root.model.maxValue)
      const longer = a.length >= b.length ? a : b
      return longer + (root.model.units.length > 0 ? " " + root.model.units : "")
    }
  }

  readonly property real availableW: Math.max(0, width  - 32)
  readonly property real availableH: Math.max(0, height - 64)
  readonly property real bigValueFontPx: {
    const metricsW = Math.max(8, bigValueMetrics.width)
    const widthFit  = (root.availableW * 0.85 / metricsW) * bigValueMetrics.font.pixelSize
    const heightFit = root.availableH * 0.50
    return Math.max(20, Math.min(160, widthFit, heightFit))
  }

  Rectangle {
    id: digitalBox

    radius: 4
    antialiasing: true
    anchors.centerIn: parent
    border.width: 1
    border.color: root.model.alarmTriggered
                  ? Qt.darker(Cpp_ThemeManager.alarmColorForSeverity(root.model.activeBandSeverity), 1.20)
                  : Qt.darker(root.accent, 1.30)
    width: Math.min(parent.width - 16, digitalColumn.implicitWidth + 32)
    height: Math.min(parent.height - 16, digitalColumn.implicitHeight + 24)
    color: digitalBox.targetColor

    Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }

    readonly property color targetColor: digitalFlash.filled
                                         ? Cpp_ThemeManager.alarmColorForSeverity(root.model.activeBandSeverity)
                                         : Cpp_ThemeManager.colors["console_base"]

    AlarmFlash {
      id: digitalFlash

      hasData: root.model.hasData
      triggered: root.model.alarmTriggered
      severity: root.model.activeBandSeverity
    }

    Column {
      id: digitalColumn

      spacing: 6
      anchors.centerIn: parent

      Text {
        id: bigValueText

        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(implicitWidth, root.width - 32)
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        text: ValueFormat.getPaddedText(root.model, root.model.value)
        font.family: Cpp_Misc_CommonFonts.monoFont.family
        font.bold: true
        font.pixelSize: root.bigValueFontPx
        color: digitalFlash.filled
               ? digitalFlash.ink
               : (root.model.alarmTriggered
                  ? Cpp_ThemeManager.alarmColorForSeverity(root.model.activeBandSeverity)
                  : root.accent)
        Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }
      }

      Text {
        opacity: 0.80
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(implicitWidth, root.width - 32)
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        text: root.title
        visible: root.title.length > 0 && !root.titleFrozenOut
        font.family: Cpp_Misc_CommonFonts.monoFont.family
        font.pixelSize: bigValueText.font.pixelSize * 0.30
        color: digitalFlash.filled
               ? digitalFlash.ink
               : (root.model.alarmTriggered
                  ? Cpp_ThemeManager.alarmColorForSeverity(root.model.activeBandSeverity)
                  : root.accent)
        Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }
      }
    }
  }

  HdrBoost {
    target: digitalBox
    anchors.fill: digitalBox
    active: digitalFlash.filled && digitalFlash.hasData
    boost: digitalFlash.blinking ? Cpp_Misc_GraphicsBackend.hdrFlashIntensity
                                 : Cpp_Misc_GraphicsBackend.hdrSteadyIntensity
  }
}
