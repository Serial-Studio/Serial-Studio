/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

import QtCore
import QtQuick
import QtQuick.Shapes
import QtQuick.Effects
import QtQuick.Controls

import SerialStudio

Item {
  id: root

  implicitWidth: 280
  implicitHeight: 280

  //
  // Widget data inputs (unused -- Clock has no project-bound model)
  //
  property var color
  property var model
  property var windowRoot
  property string widgetId

  //
  // Live tick: 1 Hz drives second-hand jump + digital readout
  //
  property date now: new Date()
  Timer {
    repeat: true
    running: true
    interval: 1000
    triggeredOnStart: true
    onTriggered: root.now = new Date()
  }

  //
  // Decomposed time so each needle binds to a stable scalar
  //
  readonly property int hours:   now.getHours()
  readonly property int minutes: now.getMinutes()
  readonly property int seconds: now.getSeconds()
  readonly property real secondAngle: seconds * 6
  readonly property real minuteAngle: (minutes + seconds / 60) * 6
  readonly property real hourAngle:   ((hours % 12) + minutes / 60) * 30
  readonly property real fontSize: Math.max(10, Math.min(14, Math.min(width, height) / 22))
                                   * Cpp_Misc_CommonFonts.widgetFontScale

  function pad(n) { return (n < 10 ? "0" : "") + n }

  function formatDate(d) {
    return Qt.formatDate(d, "ddd dd MMM yyyy")
  }

  function formatTime12(d) {
    const h24  = d.getHours()
    const h12  = ((h24 + 11) % 12) + 1
    const ampm = h24 < 12 ? "AM" : "PM"
    return pad(h12) + ":" + pad(d.getMinutes()) + ":" + pad(d.getSeconds()) + " " + ampm
  }

  //
  // Theme-aware chrome stops -- shared between the analog face and the digital
  // pane background so both pages feel like one widget.
  //
  readonly property bool darkBg: {
    const c = Cpp_ThemeManager.colors["widget_base"]
    return (0.299 * c.r + 0.587 * c.g + 0.114 * c.b) < 0.5
  }
  readonly property color chromeTop: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], darkBg ? 2.0 : 1.30)
  readonly property color chromeMid: Cpp_ThemeManager.colors["widget_base"]
  readonly property color chromeBot: Qt.darker(Cpp_ThemeManager.colors["widget_base"], darkBg ? 1.05 : 1.18)

  //
  // Persist the active SwipeView page across sessions.
  //
  Settings {
    category: "ClockWidget"
    property alias clockPageIndex: swipeView.currentIndex
  }

  //
  // SwipeView: page 0 = analog face, page 1 = large digital readout.
  //
  SwipeView {
    id: swipeView

    clip: true
    interactive: true
    anchors.fill: parent
    anchors.bottomMargin: pageIndicator.height + 4

    //
    // PAGE 0 -- Analog clock
    //
    Item {
      id: analogPage

      clip: true
      visible: opacity > 0
      opacity: SwipeView.isCurrentItem ? 1.0 : 0.0
      Behavior on opacity { NumberAnimation { duration: 150 } }

      Item {
        id: faceArea

        height: width
        anchors.centerIn: parent
        width:  Math.min(parent.width, parent.height) - 8

        readonly property real gaugeSize: width * 0.92
        readonly property real chromeW: Math.max(3, gaugeSize * 0.025)

        readonly property bool showHourLabels: gaugeSize >= 130
        readonly property real fontSize: Math.max(10, Math.min(14, gaugeSize / 22))
                                         * Cpp_Misc_CommonFonts.widgetFontScale

        //
        // Outer chrome ring -- invisible disc whose shadow gives the raised look.
        //
        Rectangle {
          id: chromeRing

          visible: !Cpp_Misc_GraphicsBackend.effectsEnabled
          width: gaugeFace.width + faceArea.chromeW * 2
          height: gaugeFace.height + faceArea.chromeW * 2
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
          visible: Cpp_Misc_GraphicsBackend.effectsEnabled
          enabled: Cpp_Misc_GraphicsBackend.effectsEnabled
        }

        //
        // Silver bezel ring with a cream inner face. ONE inner border line,
        // bezel width matched to Gauge.qml.
        //
        Rectangle {
          id: gaugeFace

          radius: width / 2
          antialiasing: true
          anchors.centerIn: parent
          width: faceArea.gaugeSize
          height: faceArea.gaugeSize
          border.color: "transparent"
          border.width: Math.max(2, width / 44)
          gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_base"], 1.15) }
            GradientStop { position: 0.5; color: Cpp_ThemeManager.colors["widget_base"] }
            GradientStop { position: 1.0; color: Qt.darker(Cpp_ThemeManager.colors["widget_base"], 1.10) }
          }

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

          //
          // Hour ticks + numerals (12 majors)
          //
          Repeater {
            model: 12

            delegate: Item {
              id: hourTickItem

              z: 1
              required property int index
              readonly property real angleDeg: index * 30
              readonly property real tickRadius: (tickOuter + tickInner) / 2
              readonly property real angleRad: (angleDeg - 90) * Math.PI / 180
              readonly property string label: index === 0 ? "12" : String(index)
              readonly property real tickOuter: gaugeFace.width / 2 - gaugeFace.border.width - 4
              readonly property real tickInner: tickOuter - Math.max(8, gaugeFace.width * 0.060)
              readonly property real labelRadius: tickInner - Math.max(10, faceArea.fontSize * 1.0)

              Rectangle {
                width: 3
                radius: 1
                opacity: 0.95
                antialiasing: true
                transformOrigin: Item.Center
                rotation: hourTickItem.angleDeg
                color: Cpp_ThemeManager.colors["widget_border"]
                height: hourTickItem.tickOuter - hourTickItem.tickInner
                x: gaugeFace.width / 2 + Math.cos(hourTickItem.angleRad) * hourTickItem.tickRadius - width / 2
                y: gaugeFace.height / 2 + Math.sin(hourTickItem.angleRad) * hourTickItem.tickRadius - height / 2
              }

              Text {
                font.pixelSize: fontSize
                text: hourTickItem.label
                visible: faceArea.showHourLabels
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                color: Cpp_ThemeManager.colors["widget_text"]
                font.family: Cpp_Misc_CommonFonts.widgetFontFamily
                x: gaugeFace.width / 2 + Math.cos(hourTickItem.angleRad) * hourTickItem.labelRadius - width / 2
                y: gaugeFace.height / 2 + Math.sin(hourTickItem.angleRad) * hourTickItem.labelRadius - height / 2
              }
            }
          }

          //
          // Minute sub-ticks (48 between the 12 hour markers)
          //
          Repeater {
            model: 60

            delegate: Item {
              id: minuteTickItem

              z: 1
              required property int index
              readonly property real angleDeg: index * 6
              readonly property bool onHour: (index % 5) === 0
              readonly property real tickRadius: (tickOuter + tickInner) / 2
              readonly property real angleRad: (angleDeg - 90) * Math.PI / 180
              readonly property real tickOuter: gaugeFace.width / 2 - gaugeFace.border.width - 4
              readonly property real tickInner: tickOuter - Math.max(3, gaugeFace.width * 0.025)

              Rectangle {
                width: 1
                radius: 0
                opacity: 0.65
                antialiasing: true
                transformOrigin: Item.Center
                visible: !minuteTickItem.onHour
                rotation: minuteTickItem.angleDeg
                height: minuteTickItem.tickOuter - minuteTickItem.tickInner
                color: Cpp_ThemeManager.colors["widget_border"]
                x: gaugeFace.width / 2 + Math.cos(minuteTickItem.angleRad) * minuteTickItem.tickRadius - width / 2
                y: gaugeFace.height / 2 + Math.sin(minuteTickItem.angleRad) * minuteTickItem.tickRadius - height / 2
              }
            }
          }

          //
          // Needle layer -- hour/minute use the widget_border silver palette,
          // second hand keeps the alarm tint.
          //
          Item {
            id: needleLayer

            z: 2
            anchors.fill: parent

            readonly property real cx: width / 2
            readonly property real cy: height / 2

            readonly property color handBase:   Cpp_ThemeManager.colors["widget_border"]
            readonly property color handStroke: Qt.darker(handBase, 1.35)

            //
            // Hour hand -- short, tapered to a rounded tip; back end stops at pivot
            //
            Shape {
              id: hourShape

              smooth: true
              layer.samples: 16
              layer.smooth: true
              antialiasing: true
              anchors.fill: parent
              rotation: root.hourAngle
              transformOrigin: Item.Center
              preferredRendererType: Shape.CurveRenderer
              layer.enabled: Cpp_Misc_GraphicsBackend.effectsEnabled

              readonly property real tipLen: gaugeFace.width * 0.24
              readonly property real tipW:  Math.max(4, hourShape.baseW * 0.50)
              readonly property real baseW: Math.max(7, gaugeFace.width * 0.062)

              Behavior on rotation { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }

              ShapePath {
                strokeWidth: 0.6
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.MiterJoin
                strokeColor: needleLayer.handStroke
                fillGradient: LinearGradient {
                  x1: needleLayer.cx
                  x2: needleLayer.cx
                  y1: needleLayer.cy
                  y2: needleLayer.cy - hourShape.tipLen
                  GradientStop { position: 0.0; color: Qt.lighter(needleLayer.handBase, 1.15) }
                  GradientStop { position: 0.5; color: needleLayer.handBase }
                  GradientStop { position: 1.0; color: Qt.darker(needleLayer.handBase, 1.18) }
                }

                startY: needleLayer.cy
                startX: needleLayer.cx - hourShape.baseW / 2

                PathLine {
                  x: needleLayer.cx - hourShape.tipW / 2
                  y: needleLayer.cy - hourShape.tipLen + hourShape.tipW / 2
                }
                PathArc {
                  radiusX: hourShape.tipW / 2
                  radiusY: hourShape.tipW / 2
                  direction: PathArc.Clockwise
                  x: needleLayer.cx + hourShape.tipW / 2
                  y: needleLayer.cy - hourShape.tipLen + hourShape.tipW / 2
                }
                PathLine {
                  y: needleLayer.cy
                  x: needleLayer.cx + hourShape.baseW / 2
                }
                PathLine {
                  y: needleLayer.cy
                  x: needleLayer.cx - hourShape.baseW / 2
                }
              }
            }

            //
            // Minute hand -- longer than hour, same rounded-tip / no-tail style
            //
            Shape {
              id: minuteShape

              smooth: true
              layer.samples: 16
              layer.smooth: true
              antialiasing: true
              anchors.fill: parent
              rotation: root.minuteAngle
              transformOrigin: Item.Center
              preferredRendererType: Shape.CurveRenderer
              layer.enabled: Cpp_Misc_GraphicsBackend.effectsEnabled

              readonly property real tipLen: gaugeFace.width * 0.36
              readonly property real baseW: Math.max(5, gaugeFace.width * 0.044)
              readonly property real tipW:  Math.max(3, minuteShape.baseW * 0.50)

              Behavior on rotation { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }

              ShapePath {
                strokeWidth: 0.6
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.MiterJoin
                strokeColor: needleLayer.handStroke
                fillGradient: LinearGradient {
                  x1: needleLayer.cx
                  x2: needleLayer.cx
                  y1: needleLayer.cy
                  y2: needleLayer.cy - minuteShape.tipLen
                  GradientStop { position: 0.0; color: Qt.lighter(needleLayer.handBase, 1.15) }
                  GradientStop { position: 0.5; color: needleLayer.handBase }
                  GradientStop { position: 1.0; color: Qt.darker(needleLayer.handBase, 1.18) }
                }

                startY: needleLayer.cy
                startX: needleLayer.cx - minuteShape.baseW / 2

                PathLine {
                  x: needleLayer.cx - minuteShape.tipW / 2
                  y: needleLayer.cy - minuteShape.tipLen + minuteShape.tipW / 2
                }
                PathArc {
                  direction: PathArc.Clockwise
                  radiusX: minuteShape.tipW / 2
                  radiusY: minuteShape.tipW / 2
                  x: needleLayer.cx + minuteShape.tipW / 2
                  y: needleLayer.cy - minuteShape.tipLen + minuteShape.tipW / 2
                }
                PathLine {
                  y: needleLayer.cy
                  x: needleLayer.cx + minuteShape.baseW / 2
                }
                PathLine {
                  y: needleLayer.cy
                  x: needleLayer.cx - minuteShape.baseW / 2
                }
              }
            }

            //
            // Second hand -- thin, accent-colored, hard-snap each tick.
            // Keeps its counterweight tail.
            //
            Shape {
              id: secondShape

              smooth: true
              layer.samples: 16
              layer.smooth: true
              antialiasing: true
              anchors.fill: parent
              rotation: root.secondAngle
              transformOrigin: Item.Center
              preferredRendererType: Shape.CurveRenderer
              layer.enabled: Cpp_Misc_GraphicsBackend.effectsEnabled

              readonly property color tint: Cpp_ThemeManager.colors["alarm"]
              readonly property real tipLen: gaugeFace.width * 0.42
              readonly property real baseW: Math.max(2, gaugeFace.width * 0.022)
              readonly property real tailW: Math.max(2, gaugeFace.width * 0.018)
              readonly property real tailLen: Math.max(8, gaugeFace.width * 0.13)

              ShapePath {
                strokeWidth: 0.4
                joinStyle: ShapePath.MiterJoin
                strokeColor: Qt.darker(secondShape.tint, 1.35)
                fillGradient: LinearGradient {
                  x1: needleLayer.cx
                  x2: needleLayer.cx
                  y1: needleLayer.cy
                  y2: needleLayer.cy - secondShape.tipLen
                  GradientStop { position: 0.0; color: Qt.darker(secondShape.tint, 1.10) }
                  GradientStop { position: 0.5; color: secondShape.tint }
                  GradientStop { position: 1.0; color: Qt.lighter(secondShape.tint, 1.18) }
                }

                startY: needleLayer.cy
                startX: needleLayer.cx - secondShape.baseW / 2

                PathLine { x: needleLayer.cx; y: needleLayer.cy - secondShape.tipLen }
                PathLine { x: needleLayer.cx + secondShape.baseW / 2; y: needleLayer.cy }
                PathLine { x: needleLayer.cx + secondShape.tailW / 2; y: needleLayer.cy + secondShape.tailLen }
                PathLine { x: needleLayer.cx - secondShape.tailW / 2; y: needleLayer.cy + secondShape.tailLen }
                PathLine { x: needleLayer.cx - secondShape.baseW / 2; y: needleLayer.cy }
              }
            }

            //
            // Pivot hub -- gauge-style silver gradient
            //
            Rectangle {
              id: pivotHub

              anchors.centerIn: parent
              width: Math.max(12, gaugeFace.width * 0.075)
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
    }

    //
    // PAGE 1 -- Large digital clock (AM/PM time + full date)
    //
    Item {
      id: digitalPage

      clip: true
      visible: opacity > 0
      opacity: SwipeView.isCurrentItem ? 1.0 : 0.0
      Behavior on opacity { NumberAnimation { duration: 150 } }

      //
      // Reference measurement -- back-solves the largest font that fits the page width.
      //
      TextMetrics {
        id: bigTimeMetrics

        font.bold: true
        font.pixelSize: 100
        text: "00:00:00 PM"
        font.family: Cpp_Misc_CommonFonts.monoFont.family
      }

      readonly property real availableW: Math.max(0, width  - 32)
      readonly property real availableH: Math.max(0, height - 64)
      readonly property real bigTimeFontPx: {
        const metricsW = Math.max(8, bigTimeMetrics.width)
        const widthFit  = (digitalPage.availableW * 0.78 / metricsW) * bigTimeMetrics.font.pixelSize
        const heightFit = digitalPage.availableH * 0.45
        return Math.max(20, Math.min(160, widthFit, heightFit))
      }

      Rectangle {
        id: digitalBox

        radius: 4
        antialiasing: true
        anchors.centerIn: parent
        border.width: 1
        color: Cpp_ThemeManager.colors["console_base"]
        border.color: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.35)
        width: Math.min(parent.width - 16, digitalColumn.implicitWidth + 32)
        height: Math.min(parent.height - 16, digitalColumn.implicitHeight + 24)

        Column {
          id: digitalColumn

          spacing: 6
          anchors.centerIn: parent

          Text {
            id: bigTimeText

            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(implicitWidth, digitalPage.width - 32)
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            text: root.formatTime12(root.now)
            color: Cpp_ThemeManager.colors["console_text"]
            font.family: Cpp_Misc_CommonFonts.monoFont.family
            font.bold: true
            font.pixelSize: digitalPage.bigTimeFontPx
          }

          Text {
            id: bigDateText

            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(implicitWidth, digitalPage.width - 32)
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            opacity: 0.85
            text: root.formatDate(root.now)
            color: Cpp_ThemeManager.colors["console_text"]
            font.family: Cpp_Misc_CommonFonts.monoFont.family
            font.pixelSize: bigTimeText.font.pixelSize * 0.42
          }
        }
      }
    }
  }

  //
  // Page indicator -- swipe gesture is primary, but the dots are clickable too
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
}
