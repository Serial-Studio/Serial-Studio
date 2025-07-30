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
  required property var windowRoot
  required property BarModel model

  property real value: 0
  Behavior on value {NumberAnimation{duration: 100}}
  onValueChanged: control.requestPaint()

  //
  // Repaint widget when needed
  //
  Connections {
    target: root.model
    function onUpdated() {
      root.value = model.value
    }
  }

  //
  // Main layout
  //
  ColumnLayout {
    spacing: 0
    anchors.margins: 8
    anchors.fill: parent

    //
    // Horizontal spacer
    //
    Item {
      Layout.fillWidth: true
    }

    //
    // Bar/tank widget
    //
    Canvas {
      id: control

      antialiasing: true
      Layout.fillWidth: true
      Layout.fillHeight: true
      onWidthChanged: requestPaint()
      onHeightChanged: requestPaint()
      Layout.alignment: Qt.AlignHCenter
      Layout.maximumWidth: Math.min(256,
                                    root.width > root.height ? root.width * 0.5 : root.width)

      property color normalColor: root.color
      property real labelPadding: labelMetrics.width + 10
      property color alarmColor: Cpp_ThemeManager.colors["alarm"]

      function formatValue(val) {
        return Cpp_UI_Dashboard.formatValue(val, model.minValue, model.maxValue)
      }

      Connections {
        target: Cpp_ThemeManager
        function onThemeChanged() {
          control.requestPaint()
        }
      }

      TextMetrics {
        id: labelMetrics
        font: Cpp_Misc_CommonFonts.customMonoFont(0.91, false)
        text: {
          const a = control.formatValue(model.minValue)
          const b = control.formatValue(model.maxValue)
          return (a.length >= b.length) ? a : b
        }
      }

      onPaint: {
        const ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)

        // Set basic geometry
        const bw = 2
        const w = width
        const h = height

        // Set painter font
        ctx.font = `${labelMetrics.font.pixelSize}px '${labelMetrics.font.family}'`

        // Dynamic Padding based on label width
        const barX = labelPadding
        const barW = w - labelPadding - bw
        const barY = bw + labelMetrics.height;
        const barH = h - 2 * bw - (2 * labelMetrics.height);

        // Extract and sanitize model values
        const value = root.value
        const min = model.minValue
        const max = model.maxValue
        const isAlarm = model.alarmTriggered
        const alarmLow = isNaN(model.normalizedAlarmLow) ? -1 : model.normalizedAlarmLow
        const alarmHigh = isNaN(model.normalizedAlarmHigh) ? 2 : model.normalizedAlarmHigh

        // Set final values for painting
        const clampedVal = Math.max(min, Math.min(max, value))
        const normVal = (clampedVal - min) / (max - min)
        const fillColor = isAlarm ? alarmColor : normalColor

        // Paint background
        ctx.fillStyle = Cpp_ThemeManager.colors["widget_base"]
        ctx.fillRect(barX, barY, barW, barH)

        // Paint fill
        const fillH = normVal * barH
        const fillY = barY + barH - fillH
        ctx.fillStyle = fillColor
        ctx.fillRect(barX, fillY, barW, fillH)

        // Paint border
        ctx.strokeStyle = Cpp_ThemeManager.colors["widget_border"]
        ctx.lineWidth = bw
        ctx.strokeRect(barX + 0.5, barY + 0.5, barW - 1, barH - 1)

        // Tick Marks + Labels
        ctx.strokeStyle = Cpp_ThemeManager.colors["widget_border"]
        ctx.fillStyle = Cpp_ThemeManager.colors["widget_text"]
        ctx.textAlign = "right"
        ctx.lineWidth = 0.5
        const steps = 5
        for (let i = 0; i <= steps; i++) {
          const frac = i / steps
          const valAtTick = min + frac * (max - min)
          const relY = 1 - frac
          const yLine = barY + relY * barH

          // Tick line
          ctx.beginPath();
          ctx.moveTo(barX - 6, yLine)
          ctx.lineTo(barX, yLine)
          ctx.stroke()

          // Label
          ctx.fillText(formatValue(valAtTick),
                       barX - 8,
                       yLine + labelMetrics.height / 4)
        }
      }
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
      id: range
      value: model.value
      units: model.units
      rangeVisible: false
      maxValue: model.maxValue
      minValue: model.minValue
      alarm: model.alarmTriggered
      maximumWidth: root.width * 0.3
      Layout.alignment: Qt.AlignHCenter
      Layout.minimumWidth: implicitWidth
      Layout.leftMargin: control.labelPadding
    }

    //
    // Spacer
    //
    Item {
      implicitWidth: 8
    }
  }
}
