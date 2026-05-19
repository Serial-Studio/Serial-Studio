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
import QtQuick.Effects
import QtQuick.Layouts

import SerialStudio

import "../"

Item {
  id: root

  required property color color
  required property var windowRoot
  required property string widgetId
  required property MeterModel model

  property real normalizedValue: model.normalizedValue
  Behavior on normalizedValue {NumberAnimation{duration: 140; easing.type: Easing.OutCubic}}

  readonly property color fillColor: model.alarmTriggered ? Cpp_ThemeManager.colors["alarm"] : root.color

  //
  // Theme-aware chrome stops -- adapts lighten/darken amounts to widget_base luminance
  //
  readonly property bool darkBg: {
    const c = Cpp_ThemeManager.colors["widget_base"]
    return (0.299 * c.r + 0.587 * c.g + 0.114 * c.b) < 0.5
  }
  readonly property color chromeTop: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], darkBg ? 2.0 : 1.30)
  readonly property color chromeMid: Cpp_ThemeManager.colors["widget_base"]
  readonly property color chromeBot: Qt.darker(Cpp_ThemeManager.colors["widget_base"], darkBg ? 1.05 : 1.18)

  readonly property real startAngleDeg: -85
  readonly property real endAngleDeg: 85
  readonly property real angleRangeDeg: endAngleDeg - startAngleDeg
  readonly property int subTicksPerMajor: 4
  readonly property real fontSize: Math.max(10, Math.min(14, Math.min(width, height) / 22))
                                   * Cpp_Misc_CommonFonts.widgetFontScale

  function evenTickValues(minV, maxV, n) {
    if (n < 2 || minV >= maxV) return [minV, maxV]
    const ticks = []
    const step = (maxV - minV) / (n - 1)
    for (let i = 0; i < n; i++)
      ticks.push(minV + step * i)

    return ticks
  }

  function formatValue(val) {
    const fmt = model.displayFormat
    if (!fmt || fmt === "" || fmt === "auto")
      return Cpp_UI_Dashboard.formatValue(val, model.minValue, model.maxValue)

    if (fmt === "0d") return Math.round(val).toString()
    if (fmt === "1d") return val.toFixed(1)
    if (fmt === "2d") return val.toFixed(2)
    if (fmt === "3d") return val.toFixed(3)
    if (fmt === "sci") return val.toExponential(2)
    const fm = fmt.match(/^%[\d\.]*\.(\d+)f$/)
    if (fm) return val.toFixed(parseInt(fm[1]))
    const fe = fmt.match(/^%[\d\.]*\.(\d+)e$/)
    if (fe) return val.toExponential(parseInt(fe[1]))
    return Cpp_UI_Dashboard.formatValue(val, model.minValue, model.maxValue)
  }

  function niceTickValues(minV, maxV, target) {
    if (minV >= maxV || target < 2) return [minV, maxV]
    const range = maxV - minV
    const roughStep = range / Math.max(1, target - 1)
    const mag = Math.pow(10, Math.floor(Math.log10(roughStep)))
    const norm = roughStep / mag
    let niceStep
    if (norm < 1.5) niceStep = 1 * mag
    else if (norm < 3) niceStep = 2 * mag
    else if (norm < 7) niceStep = 5 * mag
    else niceStep = 10 * mag
    const ticks = []
    const startNice = Math.ceil(minV / niceStep) * niceStep
    const endNice = Math.floor(maxV / niceStep) * niceStep
    for (let v = startNice; v <= endNice + 0.0001 * niceStep; v += niceStep) {
      if (v >= minV - 0.0001 && v <= maxV + 0.0001) {
        ticks.push(Math.round(v * 1e6) / 1e6)
      }
    }
    return ticks.length > 0 ? ticks : [minV, maxV]
  }

  TextMetrics {
    id: tickLabelMetrics

    font.pixelSize: fontSize
    font.family: Cpp_Misc_CommonFonts.widgetFontFamily
    text: {
      const a = formatValue(model.minValue)
      const b = formatValue(model.maxValue)
      return a.length >= b.length ? a : b
    }
  }

  ColumnLayout {
    spacing: 4
    anchors.margins: 8
    anchors.fill: parent

    Item {
      id: meterArea

      Layout.fillWidth: true
      Layout.fillHeight: true

      readonly property bool showLabels: width >= 130 && height >= 90
      readonly property real chromeW: Math.max(3, Math.min(8, Math.min(width, height) * 0.025))
      readonly property real topMargin: 2
      readonly property real bottomMargin: 4
      readonly property real faceR: Math.max(16, Math.min(
        width / 2 - meterArea.chromeW - 2,
        (height - meterArea.topMargin - meterArea.bottomMargin) / 1.16
      ))
      readonly property real visualHeight: meterArea.faceR + Math.max(meterArea.chromeW, meterArea.faceR * 0.13 + 2)
      readonly property real faceCx: width / 2
      readonly property real faceCy: Math.max(meterArea.faceR + meterArea.topMargin,
                                              (height - meterArea.visualHeight) / 2 + meterArea.faceR)
      readonly property real arcBand: Math.max(3, meterArea.faceR * 0.11)
      readonly property int autoTargetTickCount: {
        const arcLen = (angleRangeDeg * Math.PI / 180) * meterArea.faceR
        const labelW = fontSize * 4.0
        const minTicks = meterArea.faceR < 40 ? 3 : 4
        const maxTicks = meterArea.faceR < 80 ? 6 : 9
        return Math.max(minTicks, Math.min(maxTicks, Math.floor(arcLen / (labelW * 1.6))))
      }

      readonly property var tickValues: root.model.displayTickCount > 0
        ? root.evenTickValues(root.model.minValue, root.model.maxValue, root.model.displayTickCount)
        : root.niceTickValues(root.model.minValue, root.model.maxValue, meterArea.autoTargetTickCount)
      readonly property int tickCount: meterArea.tickValues.length

      readonly property bool labelsFit: {
        if (meterArea.tickCount < 2) return true
        const arcStep = (angleRangeDeg * Math.PI / 180) * meterArea.faceR * 0.7 / (meterArea.tickCount - 1)
        return arcStep > tickLabelMetrics.width + 4
      }

      //
      // Outer chrome ring -- hidden source for MultiEffect drop shadow
      //
      Shape {
        id: chromeShape

        visible: false
        layer.samples: 8
        antialiasing: true
        layer.enabled: true
        anchors.fill: parent

        readonly property real rOuter: meterArea.faceR + meterArea.chromeW

        ShapePath {
          strokeColor: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.40)
          strokeWidth: 1
          fillGradient: LinearGradient {
            x2: meterArea.faceCx; y2: meterArea.faceCy
            x1: meterArea.faceCx; y1: meterArea.faceCy - chromeShape.rOuter
            GradientStop { position: 0.0; color: root.chromeTop }
            GradientStop { position: 0.5; color: root.chromeMid }
            GradientStop { position: 1.0; color: root.chromeBot }
          }

          startY: meterArea.faceCy
          startX: meterArea.faceCx - chromeShape.rOuter
          PathArc {
            y: meterArea.faceCy
            radiusX: chromeShape.rOuter
            radiusY: chromeShape.rOuter
            direction: PathArc.Clockwise
            x: meterArea.faceCx + chromeShape.rOuter
          }
          PathLine {
            y: meterArea.faceCy
            x: meterArea.faceCx - chromeShape.rOuter
          }
        }
      }
      MultiEffect {
        shadowBlur: 0.50
        source: chromeShape
        shadowEnabled: true
        shadowOpacity: 0.28
        shadowColor: "#000000"
        shadowVerticalOffset: 2
        anchors.fill: chromeShape
      }

Shape {
        id: faceShape

        layer.samples: 8
        antialiasing: true
        layer.enabled: true
        anchors.fill: parent

        ShapePath {
          strokeColor: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.25)
          strokeWidth: 1.5
          capStyle: ShapePath.FlatCap
          joinStyle: ShapePath.RoundJoin
          fillGradient: LinearGradient {
            x2: meterArea.faceCx; y2: meterArea.faceCy
            x1: meterArea.faceCx; y1: meterArea.faceCy - meterArea.faceR
            GradientStop { position: 0.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.28) }
            GradientStop { position: 0.5; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.20) }
            GradientStop { position: 1.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.10) }
          }

          startY: meterArea.faceCy
          startX: meterArea.faceCx - meterArea.faceR
          PathArc {
            y: meterArea.faceCy
            radiusX: meterArea.faceR
            radiusY: meterArea.faceR
            direction: PathArc.Clockwise
            x: meterArea.faceCx + meterArea.faceR
          }
          PathLine {
            y: meterArea.faceCy
            x: meterArea.faceCx - meterArea.faceR
          }
        }
      }

      Shape {
        id: arcFillShape

        layer.samples: 8
        antialiasing: true
        layer.enabled: true
        anchors.fill: parent
        visible: root.normalizedValue > 0.001

        readonly property real rOut: meterArea.faceR - 4
        readonly property real startA: startAngleDeg * Math.PI / 180
        readonly property real rIn: arcFillShape.rOut - meterArea.arcBand
        readonly property real endA: (startAngleDeg + root.normalizedValue * angleRangeDeg) * Math.PI / 180

        ShapePath {
          strokeWidth: -1
          fillColor: fillColor

          startX: meterArea.faceCx + arcFillShape.rOut * Math.sin(arcFillShape.startA)
          startY: meterArea.faceCy - arcFillShape.rOut * Math.cos(arcFillShape.startA)

          PathArc {
            radiusX: arcFillShape.rOut
            radiusY: arcFillShape.rOut
            direction: PathArc.Clockwise
            x: meterArea.faceCx + arcFillShape.rOut * Math.sin(arcFillShape.endA)
            y: meterArea.faceCy - arcFillShape.rOut * Math.cos(arcFillShape.endA)
          }
          PathLine {
            x: meterArea.faceCx + arcFillShape.rIn * Math.sin(arcFillShape.endA)
            y: meterArea.faceCy - arcFillShape.rIn * Math.cos(arcFillShape.endA)
          }
          PathArc {
            radiusX: arcFillShape.rIn
            radiusY: arcFillShape.rIn
            direction: PathArc.Counterclockwise
            x: meterArea.faceCx + arcFillShape.rIn * Math.sin(arcFillShape.startA)
            y: meterArea.faceCy - arcFillShape.rIn * Math.cos(arcFillShape.startA)
          }
          PathLine {
            x: meterArea.faceCx + arcFillShape.rOut * Math.sin(arcFillShape.startA)
            y: meterArea.faceCy - arcFillShape.rOut * Math.cos(arcFillShape.startA)
          }
        }
      }

      Repeater {
        model: meterArea.tickValues
        delegate: Item {
          required property int index
          required property var modelData
          readonly property real tickValue: modelData
          readonly property real rTickOuter: meterArea.faceR - 2
          readonly property real angleRad: angleDeg * Math.PI / 180
          readonly property real angleDeg: startAngleDeg + frac * angleRangeDeg
          readonly property real rTickInner: rTickOuter - Math.max(8, meterArea.faceR * 0.10)
          readonly property real rLabel: meterArea.faceR - 4 - meterArea.arcBand - Math.max(10, fontSize * 1.0)
          readonly property real tickMidX: meterArea.faceCx + Math.sin(angleRad) * (rTickOuter + rTickInner) / 2
          readonly property real tickMidY: meterArea.faceCy - Math.cos(angleRad) * (rTickOuter + rTickInner) / 2
          readonly property real frac: (modelData - root.model.minValue) / (root.model.maxValue - root.model.minValue)

          Rectangle {
            width: 2
            radius: 1
            antialiasing: true
            height: parent.rTickOuter - parent.rTickInner
            color: Cpp_ThemeManager.colors["widget_border"]
            transformOrigin: Item.Center
            rotation: parent.angleDeg
            x: parent.tickMidX - width / 2
            y: parent.tickMidY - height / 2
          }

          Text {
            visible: meterArea.showLabels && meterArea.labelsFit
            font.pixelSize: fontSize
            text: formatValue(parent.tickValue)
            color: Cpp_ThemeManager.colors["widget_text"]
            font.family: Cpp_Misc_CommonFonts.widgetFontFamily
            x: meterArea.faceCx + Math.sin(parent.angleRad) * parent.rLabel - width / 2
            y: meterArea.faceCy - Math.cos(parent.angleRad) * parent.rLabel - height / 2
          }
        }
      }

      Repeater {
        model: Math.max(0, meterArea.tickValues.length - 1) * subTicksPerMajor
        delegate: Item {
          required property int index
          readonly property int subIndex: (index % subTicksPerMajor) + 1
          readonly property int majorIndex: Math.floor(index / subTicksPerMajor)
          readonly property real valueA: meterArea.tickValues[majorIndex]
          readonly property real valueB: meterArea.tickValues[majorIndex + 1]
          readonly property real tickValue: valueA + subIndex * (valueB - valueA) / (subTicksPerMajor + 1)
          readonly property real frac: (tickValue - root.model.minValue) / (root.model.maxValue - root.model.minValue)
          readonly property real angleDeg: startAngleDeg + frac * angleRangeDeg
          readonly property real angleRad: angleDeg * Math.PI / 180
          readonly property real rOuter: meterArea.faceR - 2
          readonly property real rInner: rOuter - Math.max(4, meterArea.faceR * 0.05)
          readonly property real midX: meterArea.faceCx + Math.sin(angleRad) * (rOuter + rInner) / 2
          readonly property real midY: meterArea.faceCy - Math.cos(angleRad) * (rOuter + rInner) / 2

          Rectangle {
            width: 1
            opacity: 0.7
            antialiasing: true
            height: parent.rOuter - parent.rInner
            color: Cpp_ThemeManager.colors["widget_border"]
            transformOrigin: Item.Center
            rotation: parent.angleDeg
            x: parent.midX - width / 2
            y: parent.midY - height / 2
          }
        }
      }

      //
      // Alarm zone highlights -- colored arc bands at the outer rim
      //
      Repeater {
        model: [
          { fromValue: root.model.minValue, toValue: root.model.alarmLow,  active: root.model.alarmsDefined && root.model.alarmLow  > root.model.minValue && root.model.alarmLow  < root.model.maxValue },
          { fromValue: root.model.alarmHigh, toValue: root.model.maxValue, active: root.model.alarmsDefined && root.model.alarmHigh > root.model.minValue && root.model.alarmHigh < root.model.maxValue }
        ]
        delegate: Shape {
          id: alarmZoneShape

          required property var modelData
          antialiasing: true
          anchors.fill: parent
          visible: modelData.active
          preferredRendererType: Shape.CurveRenderer

          readonly property real rOut: meterArea.faceR - 2
          readonly property real rIn: alarmZoneShape.rOut - Math.max(3, meterArea.faceR * 0.035)
          readonly property real angA: (startAngleDeg + alarmZoneShape.fracA * angleRangeDeg) * Math.PI / 180
          readonly property real angB: (startAngleDeg + alarmZoneShape.fracB * angleRangeDeg) * Math.PI / 180
          readonly property real fracA: (modelData.fromValue - root.model.minValue) / (root.model.maxValue - root.model.minValue)
          readonly property real fracB: (modelData.toValue   - root.model.minValue) / (root.model.maxValue - root.model.minValue)

          ShapePath {
            strokeWidth: -1
            fillColor: Cpp_ThemeManager.colors["alarm"]

            startX: meterArea.faceCx + alarmZoneShape.rOut * Math.sin(alarmZoneShape.angA)
            startY: meterArea.faceCy - alarmZoneShape.rOut * Math.cos(alarmZoneShape.angA)

            PathArc {
              radiusX: alarmZoneShape.rOut
              radiusY: alarmZoneShape.rOut
              direction: PathArc.Clockwise
              x: meterArea.faceCx + alarmZoneShape.rOut * Math.sin(alarmZoneShape.angB)
              y: meterArea.faceCy - alarmZoneShape.rOut * Math.cos(alarmZoneShape.angB)
            }
            PathLine {
              x: meterArea.faceCx + alarmZoneShape.rIn * Math.sin(alarmZoneShape.angB)
              y: meterArea.faceCy - alarmZoneShape.rIn * Math.cos(alarmZoneShape.angB)
            }
            PathArc {
              radiusX: alarmZoneShape.rIn
              radiusY: alarmZoneShape.rIn
              direction: PathArc.Counterclockwise
              x: meterArea.faceCx + alarmZoneShape.rIn * Math.sin(alarmZoneShape.angA)
              y: meterArea.faceCy - alarmZoneShape.rIn * Math.cos(alarmZoneShape.angA)
            }
            PathLine {
              x: meterArea.faceCx + alarmZoneShape.rOut * Math.sin(alarmZoneShape.angA)
              y: meterArea.faceCy - alarmZoneShape.rOut * Math.cos(alarmZoneShape.angA)
            }
          }
        }
      }

      //
      // Pointed needle -- tapered triangle with a small counterweight tail
      //
      Item {
        id: needleHolder

        width: meterArea.faceR * 2
        height: meterArea.faceR * 2
        x: meterArea.faceCx - width / 2
        y: meterArea.faceCy - height / 2
        rotation: startAngleDeg + root.normalizedValue * angleRangeDeg
        Behavior on rotation {NumberAnimation{duration: 140; easing.type: Easing.OutCubic}}

        Shape {
          id: needleShape

          antialiasing: true
          anchors.fill: parent
          preferredRendererType: Shape.CurveRenderer

          readonly property real cx: needleShape.width / 2
          readonly property real cy: needleShape.height / 2
          readonly property real tipLen: meterArea.faceR * 0.70
          readonly property real baseW: Math.max(4, meterArea.faceR * 0.07)
          readonly property real tailW: Math.max(3, meterArea.faceR * 0.05)
          readonly property real tailLen: Math.max(6, meterArea.faceR * 0.13)

          ShapePath {
            strokeWidth: 0.6
            joinStyle: ShapePath.MiterJoin
            strokeColor: Qt.darker(root.fillColor, 1.35)
            fillGradient: LinearGradient {
              y1: needleShape.cy
              y2: needleShape.cy
              x1: needleShape.cx - needleShape.baseW / 2
              x2: needleShape.cx + needleShape.baseW / 2
              GradientStop { position: 0.0; color: Qt.lighter(root.fillColor, 1.06) }
              GradientStop { position: 0.5; color: root.fillColor }
              GradientStop { position: 1.0; color: Qt.darker(root.fillColor, 1.10) }
            }

            startY: needleShape.cy
            startX: needleShape.cx - needleShape.baseW / 2
            PathLine { x: needleShape.cx; y: needleShape.cy - needleShape.tipLen }
            PathLine { x: needleShape.cx + needleShape.baseW / 2; y: needleShape.cy }
            PathLine { x: needleShape.cx + needleShape.tailW / 2; y: needleShape.cy + needleShape.tailLen }
            PathLine { x: needleShape.cx - needleShape.tailW / 2; y: needleShape.cy + needleShape.tailLen }
            PathLine { x: needleShape.cx - needleShape.baseW / 2; y: needleShape.cy }
          }
        }
      }

      //
      // Pivot hub (unified style across Gauge and Meter)
      //
      Rectangle {
        id: pivotHub

        x: meterArea.faceCx - width / 2
        y: meterArea.faceCy - height / 2
        width: Math.max(10, meterArea.faceR * 0.11)
        height: width
        radius: width / 2
        antialiasing: true
        border.width: 1
        border.color: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.45)
        gradient: Gradient {
          GradientStop { position: 0.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_border"], 1.35) }
          GradientStop { position: 0.5; color: Cpp_ThemeManager.colors["widget_border"] }
          GradientStop { position: 1.0; color: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.30) }
        }

        Rectangle {
          height: width
          radius: width / 2
          anchors.centerIn: parent
          width: parent.width * 0.40
          color: Qt.rgba(0, 0, 0, 0.45)
        }
      }

    }

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
      maximumWidth: Math.min(root.width * 0.85, 320)
      Layout.preferredHeight: visible ? implicitHeight : 0
      visible: model.showValueDisplay && root.height >= 110
    }
  }
}
