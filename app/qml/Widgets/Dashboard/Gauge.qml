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
import QtQuick.Shapes
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
  required property GaugeModel model

  //
  // Custom properties
  //
  property real normalizedValue: model.normalizedValue
  Behavior on normalizedValue {NumberAnimation{duration: 100}}
  onNormalizedValueChanged: control.requestPaint()

  //
  // Widget layout
  //
  ColumnLayout {
    spacing: 0
    anchors.fill: parent
    anchors.leftMargin: 8
    anchors.rightMargin: 8

    //
    // Spacer
    //
    Item {
      implicitHeight: 4
    }

    //
    // Gauge control
    //
    Canvas {
      id: control

      antialiasing: true
      Layout.fillWidth: true
      Layout.fillHeight: true

      property int subSteps: 4
      property real arcWidth: 10
      property real endAngle: 45
      property real tickLength: 8
      property real needleWidth: 3
      property real startAngle: 135
      property real subTickLength: 4
      property color needleColor: root.color
      property color tickColor: Cpp_ThemeManager.colors["widget_border"]
      property color borderColor: Cpp_ThemeManager.colors["widget_border"]
      property color backgroundColor: Cpp_ThemeManager.colors["widget_base"]
      property color minorTickColor: Cpp_ThemeManager.colors["widget_border"]
      property color alarmColor: model.alarmsDefined ? Cpp_ThemeManager.colors["alarm"] : root.color

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
        text: model.maxValue.toFixed(0)
        font: Cpp_Misc_CommonFonts.customMonoFont(1, true)
      }

      onPaint: {
        const ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)

        // Determine value scaling factor
        const min = model.minValue
        const max = model.maxValue
        const range = max - min
        let magnitude = 1
        let suffix = ""

        // Dynamic scaling for both large and small ranges
        if (range >= 1e6) {
          magnitude = 1e6
          suffix = "×10⁶"
        } else if (range >= 1e3) {
          magnitude = 1e3
          suffix = "×1000"
        } else if (range >= 100) {
          magnitude = 100
          suffix = "×100"
        } else if (range >= 10) {
          magnitude = 10
          suffix = "×10"
        } else if (range >= 1) {
          magnitude = 1
          suffix = ""
        } else if (range >= 1e-1) {
          magnitude = 1e-1
          suffix = "×0.1"
        } else if (range >= 1e-2) {
          magnitude = 1e-2
          suffix = "×0.01"
        } else if (range >= 1e-3) {
          magnitude = 1e-3
          suffix = "×0.001"
        } else {
          magnitude = 1e-6
          suffix = "×10⁻⁶"
        }

        // Label formatting
        function formatLabel(val) {
          const scaled = val / magnitude
          return radius < 60 ? scaled.toFixed(1) : scaled.toFixed(2)
        }

        // Iterator variables
        let i = 0
        let j = 0

        // Font & context setup
        ctx.textAlign = "center"
        ctx.textBaseline = "middle"
        ctx.font = `${labelMetrics.font.pixelSize}px '${labelMetrics.font.family}'`

        // Dimensions
        const w = width
        const h = height
        const padding = labelMetrics.height * 2
        const radius = Math.min(w, h - padding) / 2 - 10
        const cx = w / 2
        const cy = h / 2 + labelMetrics.height * 0.5

        // Angles
        const startRad = startAngle * Math.PI / 180
        const endRad = endAngle * Math.PI / 180
        const angleSpan = (2 * Math.PI + endRad - startRad) % (2 * Math.PI)

        // Value calculations
        const normVal = root.normalizedValue
        const needleAngle = startRad + angleSpan * normVal

        const alarmLowNorm = isNaN(model.alarmLow) ? -1 : model.normalizedAlarmLow
        const alarmHighNorm = isNaN(model.alarmHigh) ? 2 : model.normalizedAlarmHigh

        const arcStart = radius
        const arcWidthPx = arcWidth

        // Size dependent calculations
        const availableArcLength = angleSpan * radius
        const approxSpacingPerStep = availableArcLength / 5
        const maxLabelWidth = ctx.measureText(formatLabel(max)).width
        const steps = Math.max(3, Math.floor(availableArcLength / (3 * maxLabelWidth)))
        const labelsVisible = 2 * maxLabelWidth < radius

        // Enable shadows
        ctx.shadowColor = Cpp_ThemeManager.colors["shadow"]
        ctx.shadowBlur = 4

        // Draw inner background circle
        ctx.beginPath()
        ctx.arc(cx, cy, radius - arcWidthPx, 0, 2 * Math.PI)
        ctx.fillStyle = backgroundColor
        ctx.fill()

        // Draw safe arc segment
        const safeStart = Math.max(startRad + angleSpan * alarmLowNorm, startRad)
        const safeEnd = Math.min(startRad + angleSpan * alarmHighNorm, endRad)
        ctx.beginPath()
        ctx.arc(cx, cy, radius, safeStart, safeEnd)
        ctx.strokeStyle = Cpp_ThemeManager.colors["widget_highlight"]
        ctx.lineWidth = arcWidthPx
        ctx.stroke()

        // Draw alarm zones (red gradients)
        const gradient = ctx.createLinearGradient(cx - radius, cy, cx + radius, cy)
        gradient.addColorStop(0, alarmColor)
        gradient.addColorStop(1, alarmColor)
        if (alarmLowNorm >= 0 && alarmLowNorm < 1) {
          const aStart = startRad
          const aEnd = startRad + angleSpan * alarmLowNorm
          ctx.beginPath()
          ctx.arc(cx, cy, radius, aStart, aEnd)
          ctx.strokeStyle = gradient
          ctx.lineWidth = arcWidthPx
          ctx.stroke()
        }
        if (alarmHighNorm > 0 && alarmHighNorm <= 1) {
          const aStart = startRad + angleSpan * alarmHighNorm
          const aEnd = endRad
          ctx.beginPath()
          ctx.arc(cx, cy, radius, aStart, aEnd)
          ctx.strokeStyle = gradient
          ctx.lineWidth = arcWidthPx
          ctx.stroke()
        }

        // Arc border
        ctx.beginPath()
        ctx.arc(cx, cy, radius + arcWidthPx / 2, startRad, endRad)
        ctx.strokeStyle = borderColor
        ctx.lineWidth = arcWidthPx / 2
        ctx.stroke()

        // Ticks and labels
        const labelRadius = radius - arcWidthPx - 16
        const innerTickOffset = radius - (radius - arcWidthPx - 4) + 6
        const outerTickOffset = arcWidthPx - 4

        // Major ticks & labels
        ctx.lineWidth = 3
        ctx.strokeStyle = tickColor
        ctx.fillStyle = Cpp_ThemeManager.colors["widget_text"]
        for (i = 0; i <= steps; i++) {
          const frac = i / steps
          const angle = startRad + angleSpan * frac
          const cosA = Math.cos(angle)
          const sinA = Math.sin(angle)

          // Tick
          const x1 = cx + cosA * (radius - innerTickOffset)
          const y1 = cy + sinA * (radius - innerTickOffset)
          const x2 = cx + cosA * (radius + outerTickOffset)
          const y2 = cy + sinA * (radius + outerTickOffset)
          ctx.beginPath()
          ctx.moveTo(x1, y1)
          ctx.lineTo(x2, y2)
          ctx.stroke()

          // Label with collision check
          const rawVal = (1 - frac) * min + frac * max
          const displayVal = formatLabel(rawVal)
          const textWidth = ctx.measureText(displayVal).width
          let lx = cx + cosA * labelRadius
          let ly = cy + sinA * labelRadius

          // Calculate distance from center to label
          const labelDist = Math.sqrt(Math.pow(lx - cx, 2) + Math.pow(ly - cy, 2))

          // If it overflows inward, pull it closer to center
          const maxLabelRadius = radius - arcWidthPx - innerTickOffset
          if (labelDist + textWidth / 2 > maxLabelRadius) {
            const overshoot = (labelDist + textWidth / 2) - maxLabelRadius
            lx -= cosA * overshoot
            ly -= sinA * overshoot
          }

          if (labelsVisible)
            ctx.fillText(displayVal, lx, ly)
        }

        // Subticks
        ctx.lineWidth = 2
        ctx.strokeStyle = minorTickColor
        for (i = 0; i < steps; i++) {
          for (j = 1; j < subSteps; j++) {
            const frac = (i + j / subSteps) / steps
            const angle = startRad + angleSpan * frac
            const cosA = Math.cos(angle)
            const sinA = Math.sin(angle)

            const x1 = cx + cosA * (radius - innerTickOffset + outerTickOffset)
            const y1 = cy + sinA * (radius - innerTickOffset + outerTickOffset)
            const x2 = cx + cosA * (radius + outerTickOffset)
            const y2 = cy + sinA * (radius + outerTickOffset)
            ctx.beginPath()
            ctx.moveTo(x1, y1)
            ctx.lineTo(x2, y2)
            ctx.stroke()
          }
        }

        // Needle
        const needleLength = radius * 0.95
        const baseWidth = needleWidth * 3

        // Needle tip
        const tipX = cx + Math.cos(needleAngle) * needleLength
        const tipY = cy + Math.sin(needleAngle) * needleLength

        // Base corners (perpendicular to needle)
        const angleOffset = Math.PI / 2
        const leftX = cx + Math.cos(needleAngle - angleOffset) * baseWidth
        const leftY = cy + Math.sin(needleAngle - angleOffset) * baseWidth
        const rightX = cx + Math.cos(needleAngle + angleOffset) * baseWidth
        const rightY = cy + Math.sin(needleAngle + angleOffset) * baseWidth

        // Begin combined path
        ctx.beginPath()
        ctx.moveTo(tipX, tipY)
        ctx.lineTo(leftX, leftY)
        ctx.arc(cx, cy, baseWidth, needleAngle - Math.PI / 2, needleAngle + Math.PI / 2)
        ctx.lineTo(rightX, rightY)
        ctx.closePath()
        ctx.fillStyle = root.color
        ctx.fill()

        // Center dot
        ctx.beginPath()
        ctx.arc(cx, cy, baseWidth + 2, 0, 2 * Math.PI)
        ctx.fillStyle = borderColor
        ctx.fill()

        // Multiplier/suffix note
        if (labelsVisible) {
          ctx.fillStyle = root.color
          const suffixY = cy + radius - arcWidthPx * 3

          if (model.units.length > 0)
            ctx.fillText(model.units + " " + suffix, cx, suffixY)
          else
            ctx.fillText(suffix, cx, suffixY)
        }
      }
    }

    //
    // Spacer
    //
    Item {
      implicitHeight: 4
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
      Layout.alignment: Qt.AlignHCenter
      Layout.minimumWidth: implicitWidth
      Layout.leftMargin: control.labelPadding
    }

    //
    // Spacer
    //
    Item {
      implicitHeight: 16
    }
  }
}
