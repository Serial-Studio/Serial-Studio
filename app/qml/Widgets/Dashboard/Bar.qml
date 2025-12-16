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
      Layout.maximumWidth: isHorizontal ? root.width - 32 :
                                          ((root.width > root.height) ? root.width * 0.5 : root.width)


      property color normalColor: root.color
      property color alarmColor: Cpp_ThemeManager.colors["alarm"]
      readonly property bool isHorizontal: root.width > 1.5 * root.height

      function formatValue(val) {
        return Cpp_UI_Dashboard.formatValue(val, model.minValue, model.maxValue)
      }

      function getScaledFontSize() {
        const minSize = Math.min(root.width, root.height)
        const baseFontSize = Math.max(8, Math.min(12, minSize / 25))
        return baseFontSize
      }

      function getStepCount() {
        if (isHorizontal) {
          const availableWidth = root.width - 100
          const labelWidth = labelMetrics.width
          return Math.max(3, Math.min(7, Math.floor(availableWidth / (labelWidth * 2))))
        } else {
          const availableHeight = root.height - 100
          const labelHeight = labelMetrics.height
          return Math.max(3, Math.min(7, Math.floor(availableHeight / (labelHeight * 3))))
        }
      }

      property real labelPadding: labelMetrics.width + Math.max(8, root.width * 0.02)

      Connections {
        target: Cpp_ThemeManager
        function onThemeChanged() {
          control.requestPaint()
        }
      }

      TextMetrics {
        id: labelMetrics
        font.pixelSize: control.getScaledFontSize()
        font.family: Cpp_Misc_CommonFonts.monoFont.family
        text: {
          const a = control.formatValue(model.minValue)
          const b = control.formatValue(model.maxValue)
          return (a.length >= b.length) ? a : b
        }
      }

      onPaint: {
        const ctx = getContext("2d")

        // Clear paint & get font
        ctx.clearRect(0, 0, width, height)
        ctx.font = `${labelMetrics.font.pixelSize}px '${labelMetrics.font.family}'`

        // Constants
        const bw = 2
        const w = width
        const h = height
        const steps = control.getStepCount()
        const value = root.value
        const min = model.minValue
        const max = model.maxValue
        const isAlarm = model.alarmTriggered
        const normVal = model.normalizedValue
        const fillColor = isAlarm ? alarmColor : normalColor
        const clampedVal = Math.max(min, Math.min(max, value))
        const widgetBase = Cpp_ThemeManager.colors["widget_base"]
        const widgetText = Cpp_ThemeManager.colors["widget_text"]
        const widgetBorder = Cpp_ThemeManager.colors["widget_border"]
        const minDimension = Math.min(w, h)

        // Draw a vertical bar
        if (!control.isHorizontal) {
          const barX = Math.round(labelPadding)
          const barW = Math.round(w - labelPadding - bw)
          const barY = Math.round(bw + labelMetrics.height)
          const barH = Math.round(h - 2 * bw - (2 * labelMetrics.height))
          const fillH = Math.round(normVal * barH)
          const fillY = Math.round(barY + barH - fillH)

          // Background
          ctx.fillStyle = widgetBase
          ctx.fillRect(barX, barY, barW, barH)

          // Fill
          ctx.fillStyle = fillColor
          ctx.fillRect(barX, fillY, barW, fillH)

          // Ticks & labels
          ctx.strokeStyle = widgetBorder
          ctx.fillStyle = widgetText
          ctx.textAlign = "right"
          ctx.textBaseline = "middle"
          const showLabels = minDimension > 80 && labelPadding < w * 0.5
          for (let i = 0; i <= steps; i++) {
            const frac = i / steps
            const valAtTick = min + frac * (max - min)
            const relY = 1 - frac
            const yLine = Math.round(barY + relY * barH)

            ctx.beginPath()
            ctx.moveTo(barX - 6, yLine)
            ctx.lineTo(barX, yLine)
            ctx.stroke()

            if (showLabels) {
              ctx.fillText(formatValue(valAtTick),
                           barX - 8,
                           yLine)
            }
          }

          // Border
          ctx.strokeStyle = widgetBorder
          ctx.lineWidth = bw
          ctx.strokeRect(barX, barY, barW, barH)
        }

        // Draw horizontal bar
        else {
          const labelSpaceBottom = labelMetrics.height + 16
          const barH = Math.max(20, Math.round((h - labelSpaceBottom) * 0.6))
          const barY = Math.round((h - labelSpaceBottom - barH) / 2)
          const barX = Math.round(bw + labelMetrics.width)
          const barW = Math.round(w - 2 * barX)
          const fillW = Math.round(normVal * barW)

          // Background
          ctx.fillStyle = widgetBase
          ctx.fillRect(barX, barY, barW, barH)

          // Fill
          ctx.fillStyle = fillColor
          ctx.fillRect(barX, barY, fillW, barH)

          // Ticks & labels
          ctx.strokeStyle = widgetBorder
          ctx.fillStyle = widgetText
          ctx.textAlign = "center"
          ctx.textBaseline = "top"
          const showLabels = minDimension > 80 && (labelMetrics.width * (steps + 1) * 1.5) < barW
          for (let i = 0; i <= steps; i++) {
            const frac = i / steps
            const valAtTick = min + frac * (max - min)
            const xLine = Math.round(barX + frac * barW)

            ctx.beginPath()
            ctx.moveTo(xLine, barY + barH)
            ctx.lineTo(xLine, barY + barH + 6)
            ctx.stroke()

            if (showLabels) {
              ctx.fillText(formatValue(valAtTick),
                           xLine,
                           barY + barH + 8)
            }
          }

          // Border
          ctx.strokeStyle = widgetBorder
          ctx.lineWidth = bw
          ctx.strokeRect(barX, barY, barW, barH)
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
      maximumWidth: Math.min(root.width * 0.5, 200)
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
