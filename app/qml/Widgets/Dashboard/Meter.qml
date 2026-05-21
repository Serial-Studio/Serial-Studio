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
  readonly property real digitalFontSize: Math.max(11, Math.min(18, Math.min(width, height) / 16))
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

  //
  // Range-driven precision, left-padded for stable digital-box width (matches VisualRange).
  //
  function getPaddedText(val) {
    const a = Cpp_UI_Dashboard.formatValue(model.minValue, model.minValue, model.maxValue)
    const b = Cpp_UI_Dashboard.formatValue(model.maxValue, model.minValue, model.maxValue)
    const v = Cpp_UI_Dashboard.formatValue(val,            model.minValue, model.maxValue)
    const refLen = Math.max(a.length, b.length, v.length)
    const pad = " ".repeat(Math.max(0, refLen - v.length))
    const units = model.units.length > 0 ? " " + model.units : ""
    return pad + v + units
  }

  //
  // Padded text honoring the dataset displayFormat.
  //
  function getPaddedFormattedText(val) {
    const a = formatValue(model.minValue)
    const b = formatValue(model.maxValue)
    const v = formatValue(val)
    const refLen = Math.max(a.length, b.length, v.length)
    const pad = " ".repeat(Math.max(0, refLen - v.length))
    const units = model.units.length > 0 ? " " + model.units : ""
    return pad + v + units
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

  //
  // SwipeView -- page 0 = analog meter, page 1 = digital readout.
  // Active page is persisted per-widget via Cpp_JSON_ProjectModel.
  //
  SwipeView {
    id: swipeView

    clip: true
    interactive: true
    anchors.fill: parent
    anchors.bottomMargin: pageIndicator.height + 4

    //
    // PAGE 0 -- Analog meter
    //
    Item {
      clip: true
      visible: opacity > 0
      opacity: SwipeView.isCurrentItem ? 1.0 : 0.0
      Behavior on opacity { NumberAnimation { duration: 150 } }

      Item {
        id: meterArea

        anchors.margins: 14
        anchors.fill: parent

        readonly property real topMargin: 6
        readonly property real bottomMargin: 8
        readonly property real sideMargin: 6
        readonly property bool showLabels: width >= 130 && height >= 90
        readonly property bool showFaceLabels: width >= 140 && height >= 150
        readonly property real labelBaseRowHeight: meterArea.showFaceLabels
                                                   ? Math.round(digitalFontSize * 2.0 + 12) : 0
        readonly property real chromeW: Math.max(4, Math.min(14, Math.min(width, height) * 0.045))
        readonly property real faceR: meterArea.showFaceLabels
                                      ? Math.max(16, Math.min(
                                                   width / 2 - meterArea.chromeW - meterArea.sideMargin,
                                                   (height - meterArea.topMargin - meterArea.bottomMargin
                                                    - meterArea.labelBaseRowHeight - meterArea.chromeW - 2) / 1.13))
                                      : Math.max(16, Math.min(
                                                   width / 2 - meterArea.chromeW - meterArea.sideMargin,
                                                   (height - meterArea.topMargin - meterArea.bottomMargin) / 1.16))
        readonly property real tailExtension: Math.max(meterArea.chromeW, meterArea.faceR * 0.13 + 2)
        readonly property real labelBaseHeight: meterArea.showFaceLabels
                                                ? meterArea.tailExtension + meterArea.labelBaseRowHeight + meterArea.chromeW
                                                : 0
        readonly property real visualHeight: meterArea.faceR + meterArea.tailExtension
        readonly property real faceCx: width / 2
        readonly property real faceCy: Math.max(meterArea.faceR + meterArea.topMargin,
                                                (meterArea.height - meterArea.labelBaseHeight
                                                 - meterArea.faceR) / 2 + meterArea.faceR)
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

        readonly property real labelsArcStep: meterArea.tickCount < 2
                                              ? 0
                                              : (angleRangeDeg * Math.PI / 180) * meterArea.faceR * 0.7 / (meterArea.tickCount - 1)
        readonly property bool labelsFitAll: meterArea.tickCount < 2
                                             || meterArea.labelsArcStep > tickLabelMetrics.width + 4
        readonly property bool labelsFitAlternate: meterArea.tickCount < 2
                                                   || meterArea.labelsArcStep * 2 > tickLabelMetrics.width + 4

        //
        // Outer chrome ring -- hidden source for MultiEffect drop shadow
        //
        Shape {
          id: chromeShape

          smooth: true
          layer.samples: 16
          layer.smooth: true
          antialiasing: true
          anchors.fill: parent
          preferredRendererType: Shape.CurveRenderer
          visible: !Cpp_Misc_GraphicsBackend.effectsEnabled
          layer.enabled: Cpp_Misc_GraphicsBackend.effectsEnabled

          readonly property real bottomExtension: meterArea.labelBaseHeight
          readonly property real rOuter: meterArea.faceR + meterArea.chromeW

          ShapePath {
            strokeColor: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.40)
            strokeWidth: 1
            fillGradient: LinearGradient {
              x1: meterArea.faceCx
              x2: meterArea.faceCx
              y1: meterArea.faceCy - chromeShape.rOuter
              y2: meterArea.faceCy + chromeShape.bottomExtension
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
              x: meterArea.faceCx + chromeShape.rOuter
              y: meterArea.faceCy + chromeShape.bottomExtension
            }
            PathLine {
              x: meterArea.faceCx - chromeShape.rOuter
              y: meterArea.faceCy + chromeShape.bottomExtension
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
          visible: Cpp_Misc_GraphicsBackend.effectsEnabled
          enabled: Cpp_Misc_GraphicsBackend.effectsEnabled
        }

        Shape {
          id: faceShape

          smooth: true
          layer.samples: 16
          layer.smooth: true
          antialiasing: true
          anchors.fill: parent
          preferredRendererType: Shape.CurveRenderer
          layer.enabled: Cpp_Misc_GraphicsBackend.effectsEnabled

          readonly property real bottomExtension: Math.max(0, meterArea.labelBaseHeight - meterArea.chromeW)

          ShapePath {
            strokeColor: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.25)
            strokeWidth: 1.5
            capStyle: ShapePath.FlatCap
            joinStyle: ShapePath.RoundJoin
            fillGradient: LinearGradient {
              x1: meterArea.faceCx
              x2: meterArea.faceCx
              y1: meterArea.faceCy - meterArea.faceR
              y2: meterArea.faceCy + faceShape.bottomExtension
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
              x: meterArea.faceCx + meterArea.faceR
              y: meterArea.faceCy + faceShape.bottomExtension
            }
            PathLine {
              x: meterArea.faceCx - meterArea.faceR
              y: meterArea.faceCy + faceShape.bottomExtension
            }
            PathLine {
              y: meterArea.faceCy
              x: meterArea.faceCx - meterArea.faceR
            }
          }
        }

        Repeater {
          model: meterArea.tickValues
          delegate: Item {
            z: 1
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
              visible: meterArea.showLabels && (meterArea.labelsFitAll
                                                || (meterArea.labelsFitAlternate
                                                    && (parent.index % 2 === 0
                                                        || parent.index === meterArea.tickCount - 1)))
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
            z: 1
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

            smooth: true
            opacity: 0.60
            layer.samples: 16
            layer.smooth: true
            antialiasing: true
            anchors.fill: parent
            visible: modelData.active
            preferredRendererType: Shape.CurveRenderer
            layer.enabled: Cpp_Misc_GraphicsBackend.effectsEnabled

            required property var modelData
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
        // Display panel fill
        //
        Rectangle {
          antialiasing: true
          visible: meterArea.showFaceLabels
          y: meterArea.faceCy + 1.5
          x: meterArea.faceCx - meterArea.faceR + 0.75
          width: meterArea.faceR * 2 - 1.5
          height: meterArea.labelBaseHeight - meterArea.chromeW - 1.5
          color: Cpp_ThemeManager.colors["widget_base"]
        }

        //
        // Display panel top border
        //
        Rectangle {
          height: 1.5
          antialiasing: true
          visible: meterArea.showFaceLabels
          y: meterArea.faceCy
          x: meterArea.faceCx - meterArea.faceR + 0.75
          width: meterArea.faceR * 2 - 1.5
          color: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.25)
        }

        //
        // Label row
        //
        Item {
          id: labelBase

          y: meterArea.faceCy + 2
          width: meterArea.faceR * 2 - 2
          visible: meterArea.showFaceLabels
          x: meterArea.faceCx - meterArea.faceR + 1
          height: meterArea.labelBaseHeight - meterArea.chromeW - 4

          //
          // Hide the title and let the value box span the full row when either the
          // title would elide or the value text would overflow its half-row slot.
          //
          readonly property real rowInnerWidth: width - 12
          readonly property real halfWidth: (rowInnerWidth - 4) / 2
          readonly property bool titleFits: titleTextMetrics.width <= halfWidth - 8
          readonly property bool valueFits: valueBoxMetrics.width + 18 <= halfWidth
          readonly property bool showTitle: root.model.title.length > 0
                                            && titleFits && valueFits

          TextMetrics {
            id: titleTextMetrics

            font.bold: true
            text: root.model.title
            font.pixelSize: digitalFontSize
            font.family: Cpp_Misc_CommonFonts.widgetFontFamily
          }

          RowLayout {
            spacing: 4
            anchors.fill: parent
            anchors.leftMargin: 6
            anchors.rightMargin: 6

            Item {
              Layout.fillWidth: true
              Layout.fillHeight: true
              visible: labelBase.showTitle

              Text {
                id: titleText

                font.bold: true
                elide: Text.ElideRight
                anchors.fill: parent
                anchors.leftMargin: 4
                anchors.rightMargin: 4
                text: root.model.title
                font.pixelSize: digitalFontSize
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                color: Cpp_ThemeManager.colors["widget_text"]
                font.family: Cpp_Misc_CommonFonts.widgetFontFamily
              }
            }

            //
            // Value readout
            //
            Item {
              Layout.fillWidth: true
              Layout.fillHeight: true

              Rectangle {
                id: valueBox

                radius: 3
                border.width: 1
                antialiasing: true
                anchors.centerIn: parent
                height: valueText.implicitHeight + 8
                width: Math.min(parent.width, valueBoxMetrics.width + 18)
                border.color: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.35)
                color: model.alarmTriggered
                       ? (valueBox.alarmFlashOn
                          ? Cpp_ThemeManager.colors["alarm"]
                          : Cpp_ThemeManager.colors["console_base"])
                       : Cpp_ThemeManager.colors["console_base"]

                Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }

                property bool alarmFlashOn: false

                TextMetrics {
                  id: valueBoxMetrics

                  font.bold: true
                  font.pixelSize: digitalFontSize * 1.05
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
                  font.pixelSize: digitalFontSize * 1.05
                  font.family: Cpp_Misc_CommonFonts.widgetFontFamily
                  color: model.alarmTriggered
                         ? (valueBox.alarmFlashOn
                            ? "#ffffff"
                            : Cpp_ThemeManager.colors["alarm"])
                         : Cpp_ThemeManager.colors["console_text"]
                  text: root.getPaddedFormattedText(root.model.value)

                  Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }
                }
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

            smooth: true
            layer.samples: 16
            layer.smooth: true
            antialiasing: true
            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer
            layer.enabled: Cpp_Misc_GraphicsBackend.effectsEnabled

            readonly property real cx: needleShape.width / 2
            readonly property real cy: needleShape.height / 2
            readonly property real baseW: Math.max(4, meterArea.faceR * 0.07)
            readonly property real tailW: Math.max(3, meterArea.faceR * 0.05)
            readonly property real tailLen: Math.max(6, meterArea.faceR * 0.13)
            readonly property real tipLen: meterArea.faceR - 2 - Math.max(4, meterArea.faceR * 0.05)

            ShapePath {
              strokeWidth: 0.6
              joinStyle: ShapePath.MiterJoin
              strokeColor: Qt.darker(root.fillColor, 1.35)
              fillGradient: LinearGradient {
                x1: needleShape.cx
                x2: needleShape.cx
                y1: needleShape.cy
                y2: needleShape.cy - needleShape.tipLen
                GradientStop { position: 0.0; color: Qt.darker(root.fillColor, 1.10) }
                GradientStop { position: 0.5; color: root.fillColor }
                GradientStop { position: 1.0; color: Qt.lighter(root.fillColor, 1.18) }
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
            antialiasing: true
            color: Qt.rgba(0, 0, 0, 0.45)
            x: (parent.width  - width)  / 2
            y: (parent.height - height) / 2
            width: Math.round(parent.width * 0.40)
          }
        }
      }
    }

    //
    // PAGE 1 -- Big digital readout
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
          const a = Cpp_UI_Dashboard.formatValue(root.model.minValue, root.model.minValue, root.model.maxValue)
          const b = Cpp_UI_Dashboard.formatValue(root.model.maxValue, root.model.minValue, root.model.maxValue)
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
                      ? Qt.darker(Cpp_ThemeManager.colors["alarm"], 1.20)
                      : Qt.darker(root.color, 1.30)
        width: Math.min(parent.width - 16, digitalColumn.implicitWidth + 32)
        height: Math.min(parent.height - 16, digitalColumn.implicitHeight + 24)
        color: root.model.alarmTriggered && digitalBox.alarmFlashOn
               ? Cpp_ThemeManager.colors["alarm"]
               : Cpp_ThemeManager.colors["console_base"]

        Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }

        property bool alarmFlashOn: false
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
                   ? (digitalBox.alarmFlashOn ? "#ffffff" : Cpp_ThemeManager.colors["alarm"])
                   : root.color
            Behavior on color { ColorAnimation { duration: 280; easing.type: Easing.InOutQuad } }
          }

          Text {
            opacity: 0.80
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(implicitWidth, digitalPage.width - 32)
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            text: root.model.title
            visible: root.model.title.length > 0
            color: root.model.alarmTriggered
                   ? (digitalBox.alarmFlashOn ? "#ffffff" : Cpp_ThemeManager.colors["alarm"])
                   : root.color
            font.family: Cpp_Misc_CommonFonts.monoFont.family
            font.pixelSize: bigValueText.font.pixelSize * 0.30
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
  // Restore per-widget page from project settings, then persist on change.
  //
  Component.onCompleted: {
    const s = Cpp_JSON_ProjectModel.widgetSettings(root.widgetId)
    if (s["page"] !== undefined)
      swipeView.currentIndex = parseInt(s["page"])
  }
  Connections {
    target: swipeView
    function onCurrentIndexChanged() {
      Cpp_JSON_ProjectModel.saveWidgetSetting(
            root.widgetId, "page", swipeView.currentIndex)
    }
  }
}
