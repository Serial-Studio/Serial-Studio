/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */


import QtQuick
import QtQuick.Shapes

//
// Emergency-stop actuator (spec 0113 R24 to R27): a red mushroom head on a yellow legend
// ring. It latches on one press; a later, separate press releases it with a twist.
//
Item {
  id: root

  property bool dimmed: false
  property bool latched: false
  property bool sendable: true
  property string legend: qsTr("EMERGENCY STOP")

  readonly property real cx: width / 2
  readonly property real cy: height / 2
  readonly property real plate: root.side / 2
  readonly property real side: Math.min(width, height)
  readonly property real dome: root.plate * (root.latched ? 0.62 : 0.66)

  //
  // A release is refused this long after the press, so a double-click cannot do both
  //
  property double pressedAt: 0
  readonly property int releaseGuardMs: 600

  signal pressRequested()
  signal releaseRequested()

  // code-verify off
  // Safety colours are fixed by the spec: no theme and no control colour may change them
  readonly property color ink: "#1A1A1A"
  readonly property color red: "#D5281E"
  readonly property color yellow: "#F2C200"
  // code-verify on

  readonly property color head: root.latched ? Qt.darker(root.red, 1.18) : root.red

  //
  // Presses when released; releases when latched and the guard time has passed
  //
  function actuate() {
    if (!root.latched) {
      root.pressedAt = Date.now()
      root.pressRequested()
      return
    }

    if (Date.now() - root.pressedAt < root.releaseGuardMs)
      return

    if (!Cpp_Misc_GraphicsBackend.reduceMotion)
      twist.restart()

    root.releaseRequested()
  }

  activeFocusOnTab: true
  opacity: root.dimmed ? 0.45 : 1.0
  Keys.onSpacePressed: root.actuate()
  Keys.onReturnPressed: root.actuate()

  //
  // Legend glyphs placed around the ring: widths measured at a base size, then the whole
  // legend scaled to fit the arc it is given, once on the top and once on the bottom
  //
  FontMetrics {
    id: legendMetrics

    font: Cpp_Misc_CommonFonts.boldUiFont
  }

  readonly property real legendRadius: root.plate * 0.84
  readonly property real legendScale: {
    const text = root.legend.toUpperCase()
    const scale = root.plate * 0.2 / Math.max(1, legendMetrics.height)
    const span = legendMetrics.advanceWidth(text) * scale / Math.max(1, root.legendRadius)
    return span > 2.5 ? scale * 2.5 / span : scale
  }

  readonly property var glyphs: {
    var out = []
    const text = root.legend.toUpperCase()
    if (root.side < 24 || text.length === 0)
      return out

    var widths = []
    var total = 0
    for (var i = 0; i < text.length; ++i) {
      const w = legendMetrics.advanceWidth(text[i]) * root.legendScale / root.legendRadius
      widths.push(w)
      total += w
    }

    const centres = [-Math.PI / 2, Math.PI / 2]
    for (var c = 0; c < centres.length; ++c) {
      var angle = centres[c] - total / 2
      for (var k = 0; k < text.length; ++k) {
        out.push({ glyph: text[k], angle: angle + widths[k] / 2 })
        angle += widths[k]
      }
    }

    return out
  }

  Shape {
    smooth: true
    antialiasing: true
    anchors.fill: parent
    preferredRendererType: Shape.CurveRenderer

    ShapePath {
      fillColor: root.yellow
      strokeColor: Qt.darker(root.yellow, 1.5)
      strokeWidth: Math.max(1, root.plate * 0.012)

      startY: root.cy
      startX: root.cx + root.plate * 0.985
      PathArc {
        y: root.cy
        radiusX: root.plate * 0.985
        radiusY: root.plate * 0.985
        x: root.cx - root.plate * 0.985
      }
      PathArc {
        y: root.cy
        radiusX: root.plate * 0.985
        radiusY: root.plate * 0.985
        x: root.cx + root.plate * 0.985
      }
    }
  }

  Repeater {
    model: root.glyphs

    delegate: Text {
      required property var modelData

      color: root.ink
      text: modelData.glyph
      rotation: modelData.angle * 180 / Math.PI + 90
      font: Cpp_Misc_CommonFonts.customUiFont(root.legendScale, true)
      x: root.cx + root.legendRadius * Math.cos(modelData.angle) - width / 2
      y: root.cy + root.legendRadius * Math.sin(modelData.angle) - height / 2
    }
  }

  Shape {
    id: mushroom

    smooth: true
    antialiasing: true
    anchors.fill: parent
    preferredRendererType: Shape.CurveRenderer

    readonly property real drop: root.plate * 0.035

    ShapePath {
      strokeWidth: -1
      fillColor: Qt.rgba(0, 0, 0, 0.35)

      startY: root.cy + mushroom.drop
      startX: root.cx + mushroom.drop * 0.7 + root.dome * 1.03
      PathArc {
        radiusX: root.dome * 1.03
        radiusY: root.dome * 1.03
        y: root.cy + mushroom.drop
        x: root.cx + mushroom.drop * 0.7 - root.dome * 1.03
      }
      PathArc {
        radiusX: root.dome * 1.03
        radiusY: root.dome * 1.03
        y: root.cy + mushroom.drop
        x: root.cx + mushroom.drop * 0.7 + root.dome * 1.03
      }
    }

    ShapePath {
      strokeColor: Qt.darker(root.head, 2.2)
      strokeWidth: Math.max(1, root.dome * 0.05)
      fillGradient: RadialGradient {
        focalRadius: 0
        centerX: root.cx
        centerY: root.cy
        centerRadius: root.dome * 1.4
        focalY: root.cy - root.dome * 0.4
        focalX: root.cx - root.dome * 0.35
        GradientStop { position: 0.0; color: Qt.lighter(root.head, 1.5) }
        GradientStop { position: 0.45; color: root.head }
        GradientStop { position: 1.0; color: Qt.darker(root.head, 1.7) }
      }

      startY: root.cy
      startX: root.cx + root.dome
      PathArc {
        y: root.cy
        radiusX: root.dome
        radiusY: root.dome
        x: root.cx - root.dome
      }
      PathArc {
        y: root.cy
        radiusX: root.dome
        radiusY: root.dome
        x: root.cx + root.dome
      }
    }
  }

  //
  // Twist-release arrows moulded into the head; they turn when the stop is released
  //
  Shape {
    id: arrows

    smooth: true
    antialiasing: true
    anchors.fill: parent
    preferredRendererType: Shape.CurveRenderer

    readonly property real arc: root.dome * 0.55

    ShapePath {
      fillColor: "transparent"
      capStyle: ShapePath.RoundCap
      strokeColor: Qt.darker(root.head, 1.35)
      strokeWidth: Math.max(1, root.dome * 0.045)

      PathAngleArc {
        sweepAngle: 75
        startAngle: 200
        centerX: root.cx
        centerY: root.cy
        radiusX: arrows.arc
        radiusY: arrows.arc
      }

      PathAngleArc {
        startAngle: 20
        sweepAngle: 75
        centerX: root.cx
        centerY: root.cy
        radiusX: arrows.arc
        radiusY: arrows.arc
      }
    }

    ShapePath {
      strokeWidth: -1
      fillColor: Qt.darker(root.head, 1.35)

      PathPolyline { path: root.arrowHead(275) }
      PathPolyline { path: root.arrowHead(95) }
    }

    RotationAnimation {
      id: twist

      from: 0
      to: 360
      duration: 420
      target: arrows
      easing.type: Easing.OutCubic
    }
  }

  //
  // Triangle at the end of a twist arc, pointing along the direction of travel
  //
  function arrowHead(endDegrees) {
    const tip = (endDegrees + 12) * Math.PI / 180
    const back = endDegrees * Math.PI / 180
    const inner = arrows.arc - root.dome * 0.09
    const outer = arrows.arc + root.dome * 0.09
    return [Qt.point(root.cx + arrows.arc * Math.cos(tip), root.cy + arrows.arc * Math.sin(tip)),
            Qt.point(root.cx + inner * Math.cos(back), root.cy + inner * Math.sin(back)),
            Qt.point(root.cx + outer * Math.cos(back), root.cy + outer * Math.sin(back)),
            Qt.point(root.cx + arrows.arc * Math.cos(tip), root.cy + arrows.arc * Math.sin(tip))]
  }

  //
  // Said in words when a press cannot reach the device: a head that only looks down is a lie
  //
  Rectangle {
    radius: 3
    color: root.ink
    anchors.centerIn: parent
    width: notSent.implicitWidth + 12
    height: notSent.implicitHeight + 6
    visible: root.latched && !root.sendable

    Text {
      id: notSent

      color: root.yellow
      text: qsTr("NOT SENT")
      anchors.centerIn: parent
      font: Cpp_Misc_CommonFonts.customUiFont(Math.max(0.7, root.legendScale * 0.8), true)
    }
  }

  Rectangle {
    border.width: 2
    radius: width / 2
    color: "transparent"
    width: root.side + 6
    height: root.side + 6
    visible: root.activeFocus
    anchors.centerIn: parent
    border.color: Cpp_ThemeManager.colors["highlight"]
  }

  MouseArea {
    width: root.side
    height: root.side
    anchors.centerIn: parent
    onClicked: {
      root.forceActiveFocus()
      root.actuate()
    }
  }
}
