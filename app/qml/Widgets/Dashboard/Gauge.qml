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
  required property string widgetId
  required property GaugeModel model

  //
  // Custom properties
  //
  property real normalizedValue: model.normalizedValue
  Behavior on normalizedValue {NumberAnimation{duration: 120; easing.type: Easing.OutCubic}}

  //
  // Helper properties
  //
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

  //
  // Gauge parameters
  //
  readonly property real endAngleDeg: 135
  readonly property int subTicksPerMajor: 4
  readonly property real startAngleDeg: -135
  readonly property real angleRangeDeg: endAngleDeg - startAngleDeg
  readonly property bool showLabels: width >= 130 && height >= 130
  readonly property real fontSize: Math.max(10, Math.min(14, Math.min(width, height) / 22))
                                   * Cpp_Misc_CommonFonts.widgetFontScale
  readonly property int autoTargetTickCount: {
    const size = Math.min(width, height) * 0.75
    const radius = size / 2
    const arcLength = (angleRangeDeg * Math.PI / 180) * radius
    const labelWidth = fontSize * 5
    const minTicks = 5
    const maxTicks = radius < 100 ? 7 : 9
    return Math.max(minTicks, Math.min(maxTicks, Math.floor(arcLength / (labelWidth * 3))))
  }

  readonly property var tickValues: model.displayTickCount > 0
    ? evenTickValues(model.minValue, model.maxValue, model.displayTickCount)
    : niceTickValues(model.minValue, model.maxValue, autoTargetTickCount)
  readonly property int tickCount: tickValues.length

  readonly property bool labelsFit: {
    if (tickCount < 2) return true
    const approxR = Math.min(width, height) * 0.32
    const arcStep = (angleRangeDeg * Math.PI / 180) * approxR / (tickCount - 1)
    return arcStep > tickLabelMetrics.width + 6
  }

  TextMetrics {
    id: tickLabelMetrics

    font.pixelSize: fontSize
    font.family: Cpp_Misc_CommonFonts.widgetFontFamily
    text: {
      const a = formatValue(root.model.minValue)
      const b = formatValue(root.model.maxValue)
      return a.length >= b.length ? a : b
    }
  }

  function evenTickValues(minV, maxV, n) {
    if (n < 2 || minV >= maxV) return [minV, maxV]
    const ticks = []
    const step = (maxV - minV) / (n - 1)
    for (let i = 0; i < n; i++)
      ticks.push(minV + step * i)

    return ticks
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

  //
  // Widget layout -- vertical (stacked) when tall, horizontal (side-by-side) when wide
  //
  GridLayout {
    id: gaugeLayout

    rowSpacing: 4
    columnSpacing: 8
    anchors.margins: 8
    anchors.fill: parent

    rows: gaugeLayout.wide ? 1 : 2
    columns: gaugeLayout.wide ? 2 : 1
    readonly property bool wide: root.width > root.height * 1.4

    //
    // Gauge control
    //
    Dial {
      id: control

      enabled: false
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.row: gaugeLayout.wide ? 0 : 0
      Layout.column: gaugeLayout.wide ? 1 : 0
      Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

      value: model.value
      to: model.maxValue
      from: model.minValue
      endAngle: endAngleDeg
      snapMode: Dial.NoSnap
      inputMode: Dial.Vertical
      startAngle: startAngleDeg

      background: Item {
        implicitWidth: 200
        implicitHeight: 200

        readonly property real gaugeSize: Math.min(control.width, control.height) * (range.visible ? 0.88 : 0.95)

        //
        // Outer chrome ring -- silver gradient rendered via MultiEffect with shadow
        //
        readonly property real chromeW: Math.max(3, parent.gaugeSize * 0.025)

        Rectangle {
          id: chromeRing

          visible: false
          width: gaugeFace.width + parent.chromeW * 2
          height: gaugeFace.height + parent.chromeW * 2
          radius: width / 2
          anchors.centerIn: gaugeFace
          border.width: 1
          border.color: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.40)
          gradient: Gradient {
            GradientStop { position: 0.0; color: root.chromeTop }
            GradientStop { position: 0.5; color: root.chromeMid }
            GradientStop { position: 1.0; color: root.chromeBot }
          }
        }
        MultiEffect {
          shadowBlur: 0.50
          source: chromeRing
          shadowEnabled: true
          shadowOpacity: 0.28
          shadowColor: "#000000"
          shadowVerticalOffset: 2
          anchors.fill: chromeRing
        }


        Rectangle {
          id: gaugeFace

          radius: width / 2
          width: parent.gaugeSize
          anchors.centerIn: parent
          height: parent.gaugeSize
          border.color: "transparent"
          border.width: Math.max(3, width / 36)

          //
          // Bezel ring: light at top, darker at bottom (metal-pressed feel)
          //
          gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.15) }
            GradientStop { position: 0.5; color: Cpp_ThemeManager.colors["widget_base"] }
            GradientStop { position: 1.0; color: Qt.darker(Cpp_ThemeManager.colors["widget_base"], 1.10) }
          }

          //
          // Inner face: lighter cream/off-white interior (sits inside the bezel)
          //
          Rectangle {
            id: gaugeInnerFace

            border.width: 1
            radius: width / 2
            antialiasing: true
            anchors.fill: parent
            anchors.margins: parent.border.width
            border.color: Qt.rgba(0, 0, 0, 0.22)
            gradient: Gradient {
              GradientStop { position: 0.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.18) }
              GradientStop { position: 1.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.06) }
            }
          }

          Rectangle {
            border.width: 1
            radius: width / 2
            anchors.fill: parent
            color: "transparent"
            border.color: Qt.rgba(0, 0, 0, 0.18)
            anchors.margins: parent.border.width
          }

          Rectangle {
            opacity: 0.6
            border.width: 1
            radius: width / 2
            width: parent.width
            color: "transparent"
            height: parent.height
            anchors.centerIn: parent
            border.color: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.25)
          }

          Repeater {
            model: root.tickValues
            delegate: Item {
              required property int index
              required property var modelData
              readonly property real tickValue: modelData
              readonly property real tickRadius: (tickOuter + tickInner) / 2
              readonly property real angleRad: (angleDeg - 90) * Math.PI / 180
              readonly property real angleDeg: startAngleDeg + frac * angleRangeDeg
              readonly property real labelRadius: tickInner - Math.max(8, fontSize * 0.9)
              readonly property real tickOuter: gaugeFace.width / 2 - gaugeFace.border.width - 4
              readonly property real tickInner: tickOuter - Math.max(6, gaugeFace.width * 0.045)
              readonly property real frac: (modelData - root.model.minValue) / (root.model.maxValue - root.model.minValue)

              Rectangle {
                width: 2
                radius: 0
                opacity: 0.9
                antialiasing: true
                transformOrigin: Item.Center
                rotation: parent.angleDeg
                color: Cpp_ThemeManager.colors["widget_border"]
                height: parent.tickOuter - parent.tickInner
                x: gaugeFace.width / 2 + Math.cos(parent.angleRad) * parent.tickRadius - width / 2
                y: gaugeFace.height / 2 + Math.sin(parent.angleRad) * parent.tickRadius - height / 2
              }

              Text {
                visible: root.showLabels && root.labelsFit
                style: Text.Raised
                font.pixelSize: fontSize
                styleColor: Qt.rgba(0, 0, 0, 0.3)
                text: formatValue(parent.tickValue)
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                color: Cpp_ThemeManager.colors["widget_text"]
                font.family: Cpp_Misc_CommonFonts.widgetFontFamily
                x: gaugeFace.width / 2 + Math.cos(parent.angleRad) * parent.labelRadius - width / 2
                y: gaugeFace.height / 2 + Math.sin(parent.angleRad) * parent.labelRadius - height / 2
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

              readonly property real rOut: gaugeFace.width / 2 - gaugeFace.border.width - 1
              readonly property real rIn: alarmZoneShape.rOut - Math.max(3, gaugeFace.width * 0.025)
              readonly property real angA: (startAngleDeg + alarmZoneShape.fracA * angleRangeDeg) * Math.PI / 180
              readonly property real angB: (startAngleDeg + alarmZoneShape.fracB * angleRangeDeg) * Math.PI / 180
              readonly property real fracA: (modelData.fromValue - root.model.minValue) / (root.model.maxValue - root.model.minValue)
              readonly property real fracB: (modelData.toValue   - root.model.minValue) / (root.model.maxValue - root.model.minValue)

              ShapePath {
                strokeWidth: -1
                fillColor: Cpp_ThemeManager.colors["alarm"]

                startX: gaugeFace.width / 2 + alarmZoneShape.rOut * Math.sin(alarmZoneShape.angA)
                startY: gaugeFace.height / 2 - alarmZoneShape.rOut * Math.cos(alarmZoneShape.angA)

                PathArc {
                  radiusX: alarmZoneShape.rOut
                  radiusY: alarmZoneShape.rOut
                  direction: PathArc.Clockwise
                  x: gaugeFace.width / 2 + alarmZoneShape.rOut * Math.sin(alarmZoneShape.angB)
                  y: gaugeFace.height / 2 - alarmZoneShape.rOut * Math.cos(alarmZoneShape.angB)
                }
                PathLine {
                  x: gaugeFace.width / 2 + alarmZoneShape.rIn * Math.sin(alarmZoneShape.angB)
                  y: gaugeFace.height / 2 - alarmZoneShape.rIn * Math.cos(alarmZoneShape.angB)
                }
                PathArc {
                  radiusX: alarmZoneShape.rIn
                  radiusY: alarmZoneShape.rIn
                  direction: PathArc.Counterclockwise
                  x: gaugeFace.width / 2 + alarmZoneShape.rIn * Math.sin(alarmZoneShape.angA)
                  y: gaugeFace.height / 2 - alarmZoneShape.rIn * Math.cos(alarmZoneShape.angA)
                }
                PathLine {
                  x: gaugeFace.width / 2 + alarmZoneShape.rOut * Math.sin(alarmZoneShape.angA)
                  y: gaugeFace.height / 2 - alarmZoneShape.rOut * Math.cos(alarmZoneShape.angA)
                }
              }
            }
          }

          Repeater {
            model: Math.max(0, root.tickValues.length - 1) * subTicksPerMajor
            delegate: Item {
              required property int index
              readonly property int subIndex: (index % subTicksPerMajor) + 1
              readonly property int majorIndex: Math.floor(index / subTicksPerMajor)
              readonly property real valueA: root.tickValues[majorIndex]
              readonly property real valueB: root.tickValues[majorIndex + 1]
              readonly property real tickValue: valueA + subIndex * (valueB - valueA) / (subTicksPerMajor + 1)
              readonly property real frac: (tickValue - root.model.minValue) / (root.model.maxValue - root.model.minValue)
              readonly property real angleDeg: startAngleDeg + frac * angleRangeDeg
              readonly property real angleRad: (angleDeg - 90) * Math.PI / 180
              readonly property real subTickOuter: gaugeFace.width / 2 - gaugeFace.border.width - 4
              readonly property real subTickInner: subTickOuter - Math.max(3, gaugeFace.width * 0.022)
              readonly property real tickRadius: (subTickOuter + subTickInner) / 2

              Rectangle {
                width: 1
                radius: 0
                opacity: 0.65
                antialiasing: true
                transformOrigin: Item.Center
                rotation: parent.angleDeg
                height: parent.subTickOuter - parent.subTickInner
                color: Cpp_ThemeManager.colors["widget_border"]
                x: gaugeFace.width / 2 + Math.cos(parent.angleRad) * parent.tickRadius - width / 2
                y: gaugeFace.height / 2 + Math.sin(parent.angleRad) * parent.tickRadius - height / 2
              }
            }
          }

          Text {
            color: fillColor
            text: model.units
            style: Text.Raised
            font.pixelSize: fontSize
            anchors.bottom: parent.bottom
            styleColor: Qt.rgba(0, 0, 0, 0.3)
            anchors.bottomMargin: parent.border.width + 8
            anchors.horizontalCenter: parent.horizontalCenter
            font.family: Cpp_Misc_CommonFonts.widgetFontFamily
            visible: model.units.length > 0 && gaugeFace.width > 120
          }
        }
      }

      handle: Item {
        id: handleItem

        anchors.centerIn: parent
        width: control.background.gaugeSize
        height: control.background.gaugeSize

        //
        // Pointed needle -- tapered triangle with counterweight tail
        //
        Shape {
          id: needleShape

          antialiasing: true
          anchors.fill: parent
          rotation: control.angle
          transformOrigin: Item.Center
          preferredRendererType: Shape.CurveRenderer

          readonly property real cx: needleShape.width / 2
          readonly property real cy: needleShape.height / 2
          readonly property real tipLen: handleItem.width * 0.42
          readonly property real baseW: Math.max(5, handleItem.width * 0.045)
          readonly property real tailW: Math.max(3, handleItem.width * 0.030)
          readonly property real tailLen: Math.max(6, handleItem.width * 0.10)

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

        //
        // Pivot hub (unified style across Gauge and Meter)
        //
        Rectangle {
          id: pivotHub

          anchors.centerIn: parent
          width: Math.max(12, handleItem.width * 0.075)
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

    }

    //
    // Range/scale + current value display
    //
    VisualRange {
      id: range

      visible: model.showValueDisplay
               && (gaugeLayout.wide ? root.width >= 220 : root.height >= 130)
      value: model.value
      units: model.units
      rangeVisible: false
      maxValue: model.maxValue
      minValue: model.minValue
      alarm: model.alarmTriggered
      Layout.row: gaugeLayout.wide ? 0 : 1
      Layout.column: gaugeLayout.wide ? 0 : 0
      Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
      Layout.minimumWidth: implicitWidth
      Layout.preferredWidth: gaugeLayout.wide ? Math.min(root.width * 0.32, 280) : implicitWidth
      Layout.preferredHeight: visible ? implicitHeight : 0
      Layout.leftMargin: gaugeLayout.wide ? 12 : 0
      maximumWidth: gaugeLayout.wide ? Math.min(root.width * 0.32 - 12, 268)
                                     : Math.min(root.width * 0.85, 320)
    }
  }
}
