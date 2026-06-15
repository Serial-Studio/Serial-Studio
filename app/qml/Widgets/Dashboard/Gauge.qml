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
import QtQuick.Controls

import SerialStudio

import "../"
import "ValueFormat.js" as ValueFormat

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
  //
  // Spring follower emulates a real spring-driven movement: it tracks moving targets
  // continuously instead of restarting a fixed-duration animation on every sample.
  //
  property real normalizedValue: model.normalizedValue
  Behavior on normalizedValue {
    SpringAnimation {
      spring: 4.5
      damping: 0.4
      epsilon: 0.001
    }
  }

  //
  // Helper properties
  //

  //
  // Theme-aware chrome stops that adapt lighten/darken amounts to widget_base luminance
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
  readonly property real startAngleDeg: -135
  readonly property real angleRangeDeg: endAngleDeg - startAngleDeg

  //
  // Odd minor-tick count adapted to the arc space per major gap, so a half tick
  // always lands on the midpoint between two majors.
  //
  readonly property int subTicksPerMajor: {
    if (tickCount < 2)
      return 0

    const arcStep = (angleRangeDeg * Math.PI / 180) * Math.min(width, height) * 0.32 / (tickCount - 1)
    const fit = Math.floor(arcStep / 6) - 1
    const odd = fit % 2 === 0 ? fit - 1 : fit
    return Math.max(1, Math.min(7, odd))
  }
  readonly property bool showLabels: width >= 130 && height >= 130
  readonly property real fontSize: Math.max(10, Math.min(22, Math.min(width, height) / 28))
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

  readonly property real labelsArcStep: {
    if (tickCount < 2) return 0
    const approxR = Math.min(width, height) * 0.32
    return (angleRangeDeg * Math.PI / 180) * approxR / (tickCount - 1)
  }
  //
  // Smallest 1-in-N label stride that keeps adjacent shown labels from overlapping;
  // cascades (all -> every other -> every third ...) as the gauge shrinks.
  //
  readonly property int labelStride: {
    if (tickCount < 2 || labelsArcStep <= 0) return 1
    return Math.max(1, Math.ceil((tickLabelMetrics.width + 6) / labelsArcStep))
  }

  //
  // Measures the widest label as actually displayed (trimmed), so the fit test
  // does not overestimate width from trailing zeros.
  //
  TextMetrics {
    id: tickLabelMetrics

    font.pixelSize: fontSize
    font.family: Cpp_Misc_CommonFonts.widgetFontFamily
    text: {
      let longest = ""
      for (let i = 0; i < root.tickValues.length; ++i) {
        const s = formatTickValue(root.tickValues[i])
        if (s.length > longest.length)
          longest = s
      }
      return longest
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

  //
  // Shared instrument-widget formatters (ValueFormat.js), bound to this model
  //
  function formatValue(val) { return ValueFormat.formatValue(model, val) }
  function formatTickValue(val) { return ValueFormat.formatTickValue(model, val) }
  function getPaddedText(val) { return ValueFormat.getPaddedText(model, val) }
  function getPaddedFormattedText(val) { return ValueFormat.getPaddedFormattedText(model, val) }

  //
  // SwipeView: page 0 = analog dial, page 1 = digital readout.
  // Active page is persisted per-widget via Cpp_JSON_ProjectModel.
  //
  SwipeView {
    id: swipeView

    clip: true
    interactive: true
    anchors.fill: parent
    anchors.bottomMargin: pageIndicator.height + 4

    //
    // PAGE 0: Analog dial
    //
    Item {
      clip: true
      visible: opacity > 0
      opacity: SwipeView.isCurrentItem ? 1.0 : 0.0
      Behavior on opacity { NumberAnimation { duration: 150 } }

      Dial {
        id: control

        enabled: false
        anchors.margins: 8
        anchors.fill: parent

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

          readonly property real gaugeSize: Math.min(control.width, control.height) * 0.95

          //
          // Outer chrome ring: silver gradient rendered via MultiEffect with shadow
          //
          readonly property real chromeW: Math.max(3, gaugeSize * 0.025)

          Rectangle {
            id: chromeRing

            visible: !Cpp_Misc_GraphicsBackend.effectsEnabled
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
            shadowBlur: 0.60
            source: chromeRing
            shadowEnabled: true
            shadowOpacity: 0.15
            shadowColor: "#000000"
            shadowVerticalOffset: 1
            anchors.fill: chromeRing
            visible: Cpp_Misc_GraphicsBackend.effectsEnabled
            enabled: Cpp_Misc_GraphicsBackend.effectsEnabled
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

              border.width: 1.5
              radius: width / 2
              antialiasing: true
              anchors.fill: parent
              anchors.margins: parent.border.width
              border.color: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.40)
              gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.18) }
                GradientStop { position: 1.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.06) }
              }
            }

            Repeater {
              model: root.tickValues
              delegate: Item {
                z: 1
                required property int index
                required property var modelData
                readonly property real tickValue: modelData
                readonly property real tickRadius: (tickOuter + tickInner) / 2
                readonly property real angleRad: (angleDeg - 90) * Math.PI / 180
                readonly property real angleDeg: startAngleDeg + frac * angleRangeDeg
                readonly property real labelRadius: tickInner - Math.max(8, fontSize * 0.9)
                readonly property real tickInner: tickOuter - Math.max(8, gaugeFace.width * 0.05)
                readonly property real tickOuter: gaugeFace.width / 2 - gaugeFace.border.width - 0.75
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
                  visible: root.showLabels && (parent.index % root.labelStride === 0)

                  font.pixelSize: fontSize
                  text: formatTickValue(parent.tickValue)
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
            // Alarm-band highlights: colored arc segments at the outer rim, one per dataset band.
            //
            Repeater {
              model: root.model.alarmBands
              delegate: Shape {
                id: alarmZoneShape

                smooth: true
                opacity: 0.60
                layer.samples: 16
                layer.smooth: true
                antialiasing: true
                anchors.fill: parent
                preferredRendererType: Shape.CurveRenderer
                layer.enabled: Cpp_Misc_GraphicsBackend.effectsEnabled

                required property var modelData
                readonly property color bandColor: modelData.customColor && modelData.customColor.length > 0
                                                   ? modelData.customColor
                                                   : Cpp_ThemeManager.alarmColorForSeverity(modelData.severity)
                readonly property real rOut: gaugeFace.width / 2 - gaugeFace.border.width - 0.5
                readonly property real rIn: alarmZoneShape.rOut - Math.max(6, gaugeFace.width * 0.0250)
                readonly property real angA: (startAngleDeg + modelData.fracMin * angleRangeDeg) * Math.PI / 180
                readonly property real angB: (startAngleDeg + modelData.fracMax * angleRangeDeg) * Math.PI / 180
                readonly property bool largeArc: (modelData.fracMax - modelData.fracMin) * angleRangeDeg > 180

                ShapePath {
                  strokeWidth: -1
                  fillColor: alarmZoneShape.bandColor

                  startX: gaugeFace.width / 2 + alarmZoneShape.rOut * Math.sin(alarmZoneShape.angA)
                  startY: gaugeFace.height / 2 - alarmZoneShape.rOut * Math.cos(alarmZoneShape.angA)

                  PathArc {
                    radiusX: alarmZoneShape.rOut
                    radiusY: alarmZoneShape.rOut
                    direction: PathArc.Clockwise
                    useLargeArc: alarmZoneShape.largeArc
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
                    useLargeArc: alarmZoneShape.largeArc
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
                z: 1
                required property int index
                readonly property int subIndex: (index % subTicksPerMajor) + 1
                readonly property int majorIndex: Math.floor(index / subTicksPerMajor)
                readonly property bool halfTick: 2 * subIndex === subTicksPerMajor + 1
                readonly property real valueA: root.tickValues[majorIndex]
                readonly property real valueB: root.tickValues[majorIndex + 1]
                readonly property real tickValue: valueA + subIndex * (valueB - valueA) / (subTicksPerMajor + 1)
                readonly property real frac: (tickValue - root.model.minValue) / (root.model.maxValue - root.model.minValue)
                readonly property real angleDeg: startAngleDeg + frac * angleRangeDeg
                readonly property real angleRad: (angleDeg - 90) * Math.PI / 180
                readonly property real subTickOuter: gaugeFace.width / 2 - gaugeFace.border.width - 0.75
                readonly property real subTickInner: subTickOuter - (halfTick
                                                                     ? Math.max(6, gaugeFace.width * 0.0375)
                                                                     : Math.max(4, gaugeFace.width * 0.025))
                readonly property real tickRadius: (subTickOuter + subTickInner) / 2

                Rectangle {
                  width: parent.halfTick ? 1.5 : 1
                  radius: 0
                  opacity: parent.halfTick ? 0.8 : 0.65
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

            //
            // Bezel inner shadow: radial falloff just inside the rim so the bezel
            // reads as overhanging the dial face.
            //
            Shape {
              id: bezelShadow

              z: 1
              smooth: true
              antialiasing: true
              anchors.fill: parent
              preferredRendererType: Shape.CurveRenderer

              readonly property real cx: width / 2
              readonly property real cy: height / 2
              readonly property real rOut: width / 2 - gaugeFace.border.width - 1.5
              readonly property real rIn: bezelShadow.rOut - Math.max(5, width * 0.05)

              ShapePath {
                strokeWidth: -1
                fillRule: ShapePath.OddEvenFill
                fillGradient: RadialGradient {
                  focalRadius: 0
                  focalX: bezelShadow.cx
                  focalY: bezelShadow.cy
                  centerX: bezelShadow.cx
                  centerY: bezelShadow.cy
                  centerRadius: bezelShadow.rOut
                  GradientStop { position: bezelShadow.rIn / bezelShadow.rOut; color: "transparent" }
                  GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.10) }
                }

                startY: bezelShadow.cy
                startX: bezelShadow.cx + bezelShadow.rOut
                PathArc {
                  y: bezelShadow.cy
                  radiusX: bezelShadow.rOut
                  radiusY: bezelShadow.rOut
                  x: bezelShadow.cx - bezelShadow.rOut
                }
                PathArc {
                  y: bezelShadow.cy
                  radiusX: bezelShadow.rOut
                  radiusY: bezelShadow.rOut
                  x: bezelShadow.cx + bezelShadow.rOut
                }
                PathMove {
                  y: bezelShadow.cy
                  x: bezelShadow.cx + bezelShadow.rIn
                }
                PathArc {
                  y: bezelShadow.cy
                  radiusX: bezelShadow.rIn
                  radiusY: bezelShadow.rIn
                  x: bezelShadow.cx - bezelShadow.rIn
                }
                PathArc {
                  y: bezelShadow.cy
                  radiusX: bezelShadow.rIn
                  radiusY: bezelShadow.rIn
                  x: bezelShadow.cx + bezelShadow.rIn
                }
              }
            }

            //
            // Inner-face ring redrawn opaque above the bands and ticks so the rim line stays
            // crisp and bands never show through the border.
            //
            Rectangle {
              z: 1
              border.width: 1
              radius: width / 2
              antialiasing: true
              anchors.fill: parent
              color: "transparent"
              anchors.margins: parent.border.width
              border.color: Qt.darker(Cpp_ThemeManager.colors["widget_base"], 1.15)
            }

            //
            // Face labels
            //
            Column {
              id: faceLabels

              spacing: 2
              visible: gaugeFace.width >= 120
              anchors.bottom: gaugeFace.bottom
              anchors.bottomMargin: gaugeFace.border.width + 16 + labelLift
              anchors.horizontalCenter: gaugeFace.horizontalCenter
              width: Math.min(Math.max(gaugeFace.width * 0.55, valueBoxMetrics.width + 22),
                              gaugeFace.width - 16)

              //
              // Tick clearance for the face label: chord through the central
              // tick-free disk at the title's bottom edge (the tighter y).
              //
              readonly property real labelLift: gaugeFace.height * 0.07
                                                * Math.max(0, Math.min(1, (gaugeFace.height - 180) / 60))
              readonly property real tickInnerR: gaugeFace.width / 2 - gaugeFace.border.width - 0.75
                                                 - Math.max(8, gaugeFace.width * 0.05)
              readonly property real titleBottomFromCenter: gaugeFace.height / 2
                                                            - gaugeFace.border.width - 23
                                                            - labelLift
                                                            - valueBox.height
              readonly property real titleClearanceWidth: {
                const y = Math.abs(titleBottomFromCenter)
                if (y >= tickInnerR) return 0
                return 2 * Math.sqrt(tickInnerR * tickInnerR - y * y) - 8
              }

              //
              // Value box sits lower than the title, where the tick-free chord is narrower;
              // scale its text down so it never runs into the bottom tick labels.
              //
              readonly property real valueMidFromCenter: gaugeFace.height / 2 - gaugeFace.border.width
                                                         - 16 - labelLift - valueBoxMetrics.height / 2
              readonly property real valueClearanceWidth: {
                const y = Math.abs(valueMidFromCenter)
                if (y >= tickInnerR) return gaugeFace.width - 24
                return 2 * Math.sqrt(tickInnerR * tickInnerR - y * y) - 16
              }
              readonly property real valueScale: Math.max(0.5, Math.min(1, valueClearanceWidth
                                                 / Math.max(1, valueBoxMetrics.width + 14)))

              TextMetrics {
                id: titleLabelMetrics

                font.bold: true
                text: model.title
                font.pixelSize: fontSize * 1.05
                font.family: Cpp_Misc_CommonFonts.widgetFontFamily
              }

              Text {
                font.bold: true
                text: model.title
                style: Text.Raised
                font.pixelSize: fontSize * 1.05
                styleColor: Qt.rgba(0, 0, 0, 0.3)
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
                color: Cpp_ThemeManager.colors["widget_text"]
                font.family: Cpp_Misc_CommonFonts.widgetFontFamily
                visible: model.title.length > 0
                         && titleLabelMetrics.width <= parent.titleClearanceWidth
              }

              Item { width: 1; height: 3 }

              Rectangle {
                id: valueBox

                radius: 3
                border.width: 1
                antialiasing: true
                height: valueText.implicitHeight + 8
                anchors.horizontalCenter: parent.horizontalCenter
                width: Math.min(parent.width, valueBoxMetrics.width * faceLabels.valueScale + 18)
                border.color: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.35)
                color: model.alarmTriggered
                       ? (valueBox.alarmFlashOn
                          ? Cpp_ThemeManager.alarmColorForSeverity(root.model.activeBandSeverity)
                          : Cpp_ThemeManager.colors["console_base"])
                       : Cpp_ThemeManager.colors["console_base"]

                Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }

                property bool alarmFlashOn: false

                TextMetrics {
                  id: valueBoxMetrics

                  font.bold: true
                  font.pixelSize: fontSize * 1.05
                  font.family: Cpp_Misc_CommonFonts.widgetFontFamily
                  text: {
                    const a = root.formatValue(root.model.minValue)
                    const b = root.formatValue(root.model.maxValue)
                    const longer = a.length >= b.length ? a : b
                    return longer + (root.model.units.length > 0 ? " " + root.model.units : "")
                  }
                }

                SequentialAnimation {
                  loops: Animation.Infinite
                  running: model.alarmTriggered
                  PropertyAction { target: valueBox; property: "alarmFlashOn"; value: true }
                  PauseAnimation { duration: 450 }
                  PropertyAction { target: valueBox; property: "alarmFlashOn"; value: false }
                  PauseAnimation { duration: 450 }
                }

                Text {
                  id: valueText

                  font.bold: true
                  anchors.centerIn: parent
                  font.pixelSize: fontSize * 1.05 * faceLabels.valueScale
                  font.family: Cpp_Misc_CommonFonts.widgetFontFamily
                  color: model.alarmTriggered
                         ? (valueBox.alarmFlashOn
                            ? "#ffffff"
                            : Cpp_ThemeManager.alarmColorForSeverity(root.model.activeBandSeverity))
                         : Cpp_ThemeManager.colors["console_text"]
                  text: root.getPaddedFormattedText(model.value)

                  Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }
                }
              }
            }
          }
        }

        handle: Item {
          id: handleItem

          anchors.centerIn: parent
          width: control.background.gaugeSize
          height: control.background.gaugeSize

          //
          // Needle drop shadow: same silhouette offset straight down in screen
          // space, so the light direction stays fixed while the needle rotates.
          //
          Shape {
            smooth: true
            layer.samples: 16
            layer.smooth: true
            antialiasing: true
            width: parent.width
            height: parent.height
            transformOrigin: Item.Center
            y: Math.max(1, gaugeFace.width * 0.004)
            preferredRendererType: Shape.CurveRenderer
            layer.enabled: Cpp_Misc_GraphicsBackend.effectsEnabled
            rotation: startAngleDeg + root.normalizedValue * angleRangeDeg

            ShapePath {
              strokeWidth: -1
              fillColor: Qt.rgba(0, 0, 0, 0.10)

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
          // Pointed needle: tapered triangle with counterweight tail
          //
          Shape {
            id: needleShape

            smooth: true
            layer.samples: 16
            layer.smooth: true
            antialiasing: true
            anchors.fill: parent
            transformOrigin: Item.Center
            preferredRendererType: Shape.CurveRenderer
            layer.enabled: Cpp_Misc_GraphicsBackend.effectsEnabled
            rotation: startAngleDeg + root.normalizedValue * angleRangeDeg

            readonly property real cx: needleShape.width / 2
            readonly property real cy: needleShape.height / 2
            readonly property real tipLen: gaugeFace.width / 2 - gaugeFace.border.width - 0.75
                                           - Math.max(4, gaugeFace.width * 0.025)
            readonly property real baseW: Math.max(5, handleItem.width * 0.045)
            readonly property real tailW: Math.max(3, handleItem.width * 0.030)
            readonly property real tailLen: Math.max(6, handleItem.width * 0.10)

            ShapePath {
              strokeWidth: 0.6
              joinStyle: ShapePath.MiterJoin
              strokeColor: Qt.darker(root.color, 1.35)
              fillGradient: LinearGradient {
                x1: needleShape.cx
                x2: needleShape.cx
                y1: needleShape.cy
                y2: needleShape.cy - needleShape.tipLen
                GradientStop { position: 0.0; color: Qt.darker(root.color, 1.10) }
                GradientStop { position: 0.5; color: root.color }
                GradientStop { position: 1.0; color: Qt.lighter(root.color, 1.18) }
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
          // Pivot hub shadow: soft radial falloff under the hub
          //
          Shape {
            id: hubShadow

            smooth: true
            height: width
            antialiasing: true
            anchors.centerIn: parent
            preferredRendererType: Shape.CurveRenderer
            width: Math.max(12, handleItem.width * 0.075) * 1.7
            anchors.verticalCenterOffset: Math.max(1, gaugeFace.width * 0.006)

            readonly property real r: width / 2

            ShapePath {
              strokeWidth: -1
              fillGradient: RadialGradient {
                focalRadius: 0
                focalX: hubShadow.r
                focalY: hubShadow.r
                centerX: hubShadow.r
                centerY: hubShadow.r
                centerRadius: hubShadow.r
                GradientStop { position: 0.0;  color: Qt.rgba(0, 0, 0, 0.20) }
                GradientStop { position: 0.55; color: Qt.rgba(0, 0, 0, 0.14) }
                GradientStop { position: 1.0;  color: "transparent" }
              }

              startY: hubShadow.r
              startX: hubShadow.width
              PathArc {
                x: 0
                y: hubShadow.r
                radiusX: hubShadow.r
                radiusY: hubShadow.r
              }
              PathArc {
                y: hubShadow.r
                x: hubShadow.width
                radiusX: hubShadow.r
                radiusY: hubShadow.r
              }
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
              antialiasing: true
              color: Qt.rgba(0, 0, 0, 0.45)
              x: (parent.width  - width)  / 2
              y: (parent.height - height) / 2
              width: Math.round(parent.width * 0.40)
            }
          }
        }
      }
    }

    //
    // PAGE 1: Big digital readout (mirrors the Clock digital page)
    //
    Item {
      id: digitalPage

      clip: true
      visible: opacity > 0
      opacity: SwipeView.isCurrentItem ? 1.0 : 0.0
      Behavior on opacity { NumberAnimation { duration: 150 } }

      TextMetrics {
        id: bigValueMetrics

        font.bold: true
        font.pixelSize: 100
        font.family: Cpp_Misc_CommonFonts.monoFont.family
        text: {
          const a = root.formatValue(root.model.minValue)
          const b = root.formatValue(root.model.maxValue)
          const longer = a.length >= b.length ? a : b
          return longer + (root.model.units.length > 0 ? " " + root.model.units : "")
        }
      }

      readonly property real availableW: Math.max(0, width  - 32)
      readonly property real availableH: Math.max(0, height - 64)
      readonly property real bigValueFontPx: {
        const metricsW = Math.max(8, bigValueMetrics.width)
        const widthFit  = (digitalPage.availableW * 0.85 / metricsW) * bigValueMetrics.font.pixelSize
        const heightFit = digitalPage.availableH * 0.50
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
                      : Qt.darker(root.color, 1.30)
        width: Math.min(parent.width - 16, digitalColumn.implicitWidth + 32)
        height: Math.min(parent.height - 16, digitalColumn.implicitHeight + 24)
        color: digitalBox.targetColor

        Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }

        property bool alarmFlashOn: false
        readonly property color targetColor: root.model.alarmTriggered && digitalBox.alarmFlashOn
                                              ? Cpp_ThemeManager.alarmColorForSeverity(root.model.activeBandSeverity)
                                              : Cpp_ThemeManager.colors["console_base"]
        SequentialAnimation {
          loops: Animation.Infinite
          running: root.model.alarmTriggered
          PropertyAction { target: digitalBox; property: "alarmFlashOn"; value: true }
          PauseAnimation { duration: 450 }
          PropertyAction { target: digitalBox; property: "alarmFlashOn"; value: false }
          PauseAnimation { duration: 450 }
        }

        Column {
          id: digitalColumn

          spacing: 6
          anchors.centerIn: parent

          Text {
            id: bigValueText

            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(implicitWidth, digitalPage.width - 32)
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            text: getPaddedText(root.model.value)
            font.family: Cpp_Misc_CommonFonts.monoFont.family
            font.bold: true
            font.pixelSize: digitalPage.bigValueFontPx
            color: root.model.alarmTriggered
                   ? (digitalBox.alarmFlashOn ? "#ffffff" : Cpp_ThemeManager.alarmColorForSeverity(root.model.activeBandSeverity))
                   : root.color
            Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }
          }

          Text {
            id: titleText

            opacity: 0.80
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(implicitWidth, digitalPage.width - 32)
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            text: root.model.title
            visible: root.model.title.length > 0
            font.family: Cpp_Misc_CommonFonts.monoFont.family
            font.pixelSize: bigValueText.font.pixelSize * 0.30
            color: root.model.alarmTriggered
                   ? (digitalBox.alarmFlashOn ? "#ffffff" : Cpp_ThemeManager.alarmColorForSeverity(root.model.activeBandSeverity))
                   : root.color
            Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }
          }
        }
      }
    }
  }

  //
  // Page indicator
  //
  PageIndicator {
    id: pageIndicator

    interactive: true
    count: swipeView.count
    anchors.bottomMargin: 4
    anchors.bottom: parent.bottom
    currentIndex: swipeView.currentIndex
    anchors.horizontalCenter: parent.horizontalCenter

    delegate: Rectangle {
      required property int index
      implicitWidth: 8
      implicitHeight: 8
      radius: width / 2
      antialiasing: true
      color: Cpp_ThemeManager.colors["widget_text"]
      opacity: index === pageIndicator.currentIndex ? 0.95 : 0.40
      Behavior on opacity { NumberAnimation { duration: 120 } }
    }

    onCurrentIndexChanged: {
      if (swipeView.currentIndex !== currentIndex)
        swipeView.currentIndex = currentIndex
    }
  }

  //
  // Suppresses the page auto-save while restore assigns the persisted index
  //
  property bool restoringPage: false

  //
  // Restore per-widget page from project settings, then persist on change.
  //
  Component.onCompleted: {
    root.restoringPage = true
    const s = Cpp_JSON_ProjectModel.widgetSettings(root.widgetId)
    if (s["page"] !== undefined)
      swipeView.currentIndex = parseInt(s["page"])

    root.restoringPage = false
  }
  Connections {
    target: swipeView
    function onCurrentIndexChanged() {
      if (root.restoringPage)
        return

      Cpp_JSON_ProjectModel.saveWidgetSetting(
            root.widgetId, "page", swipeView.currentIndex)
    }
  }
}
