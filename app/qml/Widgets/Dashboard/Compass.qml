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

import QtQuick
import QtQuick.Shapes
import QtQuick.Effects
import QtQuick.Controls

import SerialStudio

Item {
  id: root

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property string widgetId
  required property CompassModel model

  //
  // Unbounded accumulator -- needle takes shortest path across the 359->0 wrap.
  //
  property real unwrappedHeading: 0
  Behavior on unwrappedHeading {
    NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
  }

  Connections {
    target: root.model
    function onUpdated() {
      const target = root.model.value
      const base   = ((root.unwrappedHeading % 360) + 360) % 360
      let delta    = target - base
      if (delta > 180)  delta -= 360
      else if (delta < -180) delta += 360
      root.unwrappedHeading += delta
    }
  }

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
  // Compass parameters -- full 360 deg rose with fixed N at top
  //
  readonly property bool showLabels: width >= 130 && height >= 130
  readonly property bool showSubLabels: width >= 200 && height >= 200
  readonly property real fontSize: Math.max(10, Math.min(14, Math.min(width, height) / 22))
                                   * Cpp_Misc_CommonFonts.widgetFontScale

  readonly property var cardinalLabels: ["N", "NE", "E", "SE", "S", "SW", "W", "NW"]

  //
  // Range-driven precision, left-padded for stable digital-box width (matches VisualRange).
  //
  function getPaddedText(val) {
    const a = Cpp_UI_Dashboard.formatValue(0,   0, 360)
    const b = Cpp_UI_Dashboard.formatValue(360, 0, 360)
    const v = Cpp_UI_Dashboard.formatValue(val, 0, 360)
    const refLen = Math.max(a.length, b.length, v.length)
    const pad = " ".repeat(Math.max(0, refLen - v.length))
    return pad + v
  }

  //
  // SwipeView -- page 0 = analog rose, page 1 = digital heading.
  // Active page is persisted per-widget via Cpp_JSON_ProjectModel.
  //
  SwipeView {
    id: swipeView

    clip: true
    interactive: true
    anchors.fill: parent
    anchors.bottomMargin: pageIndicator.height + 4

    //
    // PAGE 0 -- Analog compass rose
    //
    Item {
      id: analogPage

      Item {
        id: control

        anchors.margins: 8
        anchors.fill: parent

        readonly property real chromeW: Math.max(3, gaugeSize * 0.025)
        readonly property real gaugeSize: Math.min(control.width, control.height) * 0.95

        //
        // Outer chrome ring -- silver gradient rendered via MultiEffect with shadow
        //
        Rectangle {
          id: chromeRing

          visible: false
          width: gaugeFace.width + control.chromeW * 2
          height: gaugeFace.height + control.chromeW * 2
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

        //
        // Silver bezel + cream inner face (mirrors Gauge.qml chrome stack)
        //
        Rectangle {
          id: gaugeFace

          radius: width / 2
          width: control.gaugeSize
          anchors.centerIn: parent
          height: control.gaugeSize
          border.color: "transparent"
          border.width: Math.max(3, width / 36)
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
          // Tick ring -- 36 marks at 10 deg, every third is a longer major tick
          //
          Repeater {
            model: 36

            delegate: Item {
              z: 1
              required property int index
              readonly property bool isMajor: (index % 3) === 0
              readonly property real angleDeg: index * 10
              readonly property real angleRad: (angleDeg - 90) * Math.PI / 180
              readonly property real tickOuter: gaugeFace.width / 2 - gaugeFace.border.width - 4
              readonly property real tickInner: tickOuter - (isMajor
                                                             ? Math.max(8, gaugeFace.width * 0.060)
                                                             : Math.max(3, gaugeFace.width * 0.025))
              readonly property real tickRadius: (tickOuter + tickInner) / 2

              Rectangle {
                radius: 0
                antialiasing: true
                transformOrigin: Item.Center
                rotation: parent.angleDeg
                width: parent.isMajor ? 2 : 1
                opacity: parent.isMajor ? 0.95 : 0.6
                color: Cpp_ThemeManager.colors["widget_border"]
                height: parent.tickOuter - parent.tickInner
                x: gaugeFace.width / 2 + Math.cos(parent.angleRad) * parent.tickRadius - width / 2
                y: gaugeFace.height / 2 + Math.sin(parent.angleRad) * parent.tickRadius - height / 2
              }
            }
          }

          //
          // Cardinal labels -- N/E/S/W bold and oversized, NE/SE/SW/NW only on larger faces.
          // N is tinted with the widget's accent to signal the "this end of the needle" convention.
          //
          Repeater {
            model: root.cardinalLabels

            delegate: Item {
              z: 2
              required property int index
              required property string modelData
              readonly property bool isPrimary: (index % 2) === 0
              readonly property real angleDeg: index * 45
              readonly property real angleRad: (angleDeg - 90) * Math.PI / 180
              readonly property real labelRadius: (gaugeFace.width / 2 - gaugeFace.border.width - 4)
                                                  - Math.max(8, gaugeFace.width * 0.060)
                                                  - Math.max(10, root.fontSize * 1.10)

              Text {
                text: parent.modelData
                font.bold: parent.isPrimary
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: root.fontSize * (parent.isPrimary ? 1.35 : 1.00)
                font.family: Cpp_Misc_CommonFonts.widgetFontFamily
                visible: root.showLabels && (parent.isPrimary || root.showSubLabels)
                color: parent.modelData === "N"
                       ? root.fillColor
                       : Cpp_ThemeManager.colors["widget_text"]
                x: gaugeFace.width / 2 + Math.cos(parent.angleRad) * parent.labelRadius - width / 2
                y: gaugeFace.height / 2 + Math.sin(parent.angleRad) * parent.labelRadius - height / 2
              }
            }
          }

          //
          // Compass needle -- accent-colored north end and silver counterweight tail.
          // Rotates by the unwrapped accumulator so 359 -> 0 takes the short way.
          //
          Shape {
            id: needleShape

            antialiasing: true
            width: gaugeFace.width
            anchors.centerIn: parent
            height: gaugeFace.height
            transformOrigin: Item.Center
            rotation: root.unwrappedHeading
            preferredRendererType: Shape.CurveRenderer

            readonly property real cx: needleShape.width / 2
            readonly property real cy: needleShape.height / 2
            readonly property real tipLen: gaugeFace.width * 0.36
            readonly property real tailLen: gaugeFace.width * 0.30
            readonly property real baseW:  Math.max(5, gaugeFace.width * 0.050)
            readonly property real tailW:  Math.max(4, gaugeFace.width * 0.040)

            //
            // North end -- accent-colored, tapered triangle
            //
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
              PathLine { x: needleShape.cx;                                y: needleShape.cy - needleShape.tipLen }
              PathLine { x: needleShape.cx + needleShape.baseW / 2;        y: needleShape.cy }
              PathLine { x: needleShape.cx - needleShape.baseW / 2;        y: needleShape.cy }
            }

            //
            // South end -- silver counterweight, slightly shorter than the north tip
            //
            ShapePath {
              strokeWidth: 0.6
              joinStyle: ShapePath.MiterJoin
              strokeColor: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.35)
              fillGradient: LinearGradient {
                x1: needleShape.cx
                x2: needleShape.cx
                y1: needleShape.cy
                y2: needleShape.cy + needleShape.tailLen
                GradientStop { position: 0.0; color: Qt.lighter(Cpp_ThemeManager.colors["widget_border"], 1.15) }
                GradientStop { position: 0.5; color: Cpp_ThemeManager.colors["widget_border"] }
                GradientStop { position: 1.0; color: Qt.darker(Cpp_ThemeManager.colors["widget_border"], 1.18) }
              }

              startY: needleShape.cy
              startX: needleShape.cx - needleShape.tailW / 2
              PathLine { x: needleShape.cx;                                y: needleShape.cy + needleShape.tailLen }
              PathLine { x: needleShape.cx + needleShape.tailW / 2;        y: needleShape.cy }
              PathLine { x: needleShape.cx - needleShape.tailW / 2;        y: needleShape.cy }
            }
          }

          //
          // Pivot hub (unified style across Gauge / Meter / Compass)
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
              anchors.centerIn: parent
              width: parent.width * 0.40
              color: Qt.rgba(0, 0, 0, 0.45)
            }
          }

        }
      }
    }

    //
    // PAGE 1 -- Big digital readout (heading + cardinal, mirrors Gauge digital page)
    //
    Item {
      id: digitalPage

      TextMetrics {
        id: bigValueMetrics

        font.bold: true
        font.pixelSize: 100
        text: Cpp_UI_Dashboard.formatValue(360, 0, 360) + " NE"
        font.family: Cpp_Misc_CommonFonts.monoFont.family
      }

      readonly property real availableW: Math.max(0, width  - 32)
      readonly property real availableH: Math.max(0, height - 64)
      readonly property real bigValueFontPx: {
        const metricsW = Math.max(8, bigValueMetrics.width)
        const widthFit  = (digitalPage.availableW * 0.85 / metricsW) * bigValueMetrics.font.pixelSize
        const heightFit = digitalPage.availableH * 0.45
        return Math.max(20, Math.min(160, widthFit, heightFit))
      }

      Rectangle {
        id: digitalBox

        radius: 4
        antialiasing: true
        anchors.centerIn: parent
        border.width: 1
        width: digitalColumn.implicitWidth + 32
        height: digitalColumn.implicitHeight + 24
        color: Cpp_ThemeManager.colors["console_base"]
        border.color: Qt.darker(root.color, 1.30)

        Column {
          id: digitalColumn

          spacing: 6
          anchors.centerIn: parent

          Text {
            id: bigValueText

            font.bold: true
            color: root.color
            font.pixelSize: digitalPage.bigValueFontPx
            anchors.horizontalCenter: parent.horizontalCenter
            font.family: Cpp_Misc_CommonFonts.monoFont.family
            text: root.getPaddedText(root.model.value) + " "
                  + (root.model.cardinal.length < 2 ? root.model.cardinal + " "
                                                    : root.model.cardinal)
          }

          Text {
            id: titleText

            opacity: 0.80
            color: root.color
            text: root.model.title
            visible: root.model.title.length > 0
            anchors.horizontalCenter: parent.horizontalCenter
            font.family: Cpp_Misc_CommonFonts.monoFont.family
            font.pixelSize: bigValueText.font.pixelSize * 0.30
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
