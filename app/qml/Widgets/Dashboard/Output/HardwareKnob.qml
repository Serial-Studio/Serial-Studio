/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */


import QtQuick
import QtQuick.Shapes

import "../.." as Widgets

//
// Pointer knob (spec 0113): a black knob with a white pointer in a metal ring over 270
// degrees of detent marks. Drag it round, or use the wheel or the arrow keys (Shift for ten).
//
Item {
  id: root

  property real to: 100
  property real from: 0
  property real step: 1
  property real value: 0
  property bool dimmed: false

  signal moved(real value)

  readonly property real span: 270
  readonly property real cx: width / 2
  readonly property real cy: height / 2
  readonly property bool pressed: area.pressed
  readonly property real ring: Math.min(width, height) / 2 * 0.62
  readonly property real range: Math.max(1e-9, root.to - root.from)
  readonly property int marks: root.steps <= 24 ? root.steps + 1 : 11
  readonly property int steps: Math.round(root.range / Math.max(root.step, 1e-9))

  //
  // Degrees clockwise from straight up for a value
  //
  function angleOf(v) {
    const t = Math.max(0, Math.min(1, (v - root.from) / root.range))
    return -root.span / 2 + root.span * t
  }

  //
  // Snaps a value to Step from Min inside the range and reports it when it moved
  //
  function propose(v) {
    const stepped = root.from + Math.round((v - root.from) / Math.max(root.step, 1e-9)) * root.step
    const clamped = Math.max(root.from, Math.min(root.to, stepped))
    if (Math.abs(clamped - root.value) > root.step * 1e-6)
      root.moved(clamped)
  }

  //
  // Value under a pointer position, by its angle round the centre
  //
  function valueAt(px, py) {
    const degrees = Math.atan2(px - root.cx, root.cy - py) * 180 / Math.PI
    const t = (Math.max(-root.span / 2, Math.min(root.span / 2, degrees)) + root.span / 2)
    return root.from + root.range * t / root.span
  }

  activeFocusOnTab: true
  opacity: root.dimmed ? 0.45 : 1.0
  Keys.onLeftPressed: (event) => root.nudge(event.modifiers, -1)
  Keys.onRightPressed: (event) => root.nudge(event.modifiers, 1)

  //
  // One Step in @p direction, or ten with Shift held
  //
  function nudge(modifiers, direction) {
    const tens = modifiers & Qt.ShiftModifier ? 10 : 1
    root.propose(root.value + root.step * tens * direction)
  }

  //
  // Detent marks just outside the ring; the ends are longer
  //
  Repeater {
    model: root.marks

    delegate: Rectangle {
      required property int index

      readonly property real turn: angle * Math.PI / 180
      readonly property real reach: root.ring * (end ? 1.22 : 1.18)
      readonly property bool end: index === 0 || index === root.marks - 1
      readonly property real angle: -root.span / 2 + root.span * index / (root.marks - 1)

      radius: width / 2
      antialiasing: true
      rotation: angle
      color: Cpp_ThemeManager.colors["widget_text"]
      width: Math.max(1.5, root.ring * 0.045)
      height: Math.max(3, root.ring * (end ? 0.2 : 0.12))
      x: root.cx + reach * Math.sin(turn) - width / 2
      y: root.cy - reach * Math.cos(turn) - height / 2
    }
  }

  Widgets.PanelBezel {
    knurled: false
    width: root.ring * 2
    height: root.ring * 2
    anchors.centerIn: parent
  }

  Shape {
    smooth: true
    antialiasing: true
    anchors.fill: parent
    preferredRendererType: Shape.CurveRenderer

    ShapePath {
      strokeColor: Qt.rgba(0.04, 0.05, 0.06, 1)
      strokeWidth: Math.max(1, root.ring * 0.03)
      fillGradient: RadialGradient {
        focalRadius: 0
        centerX: root.cx
        centerY: root.cy
        centerRadius: root.ring * 0.9
        focalX: root.cx - root.ring * 0.2
        focalY: root.cy - root.ring * 0.25
        GradientStop { position: 0.0; color: Qt.rgba(0.25, 0.27, 0.31, 1) }
        GradientStop { position: 1.0; color: Qt.rgba(0.07, 0.08, 0.09, 1) }
      }

      startY: root.cy
      startX: root.cx + root.ring * 0.78
      PathArc {
        y: root.cy
        radiusX: root.ring * 0.78
        radiusY: root.ring * 0.78
        x: root.cx - root.ring * 0.78
      }
      PathArc {
        y: root.cy
        radiusX: root.ring * 0.78
        radiusY: root.ring * 0.78
        x: root.cx + root.ring * 0.78
      }
    }
  }

  //
  // Pointer: a white line from near the centre to the knob's rim
  //
  Item {
    width: root.ring * 0.12
    height: root.ring * 1.56
    anchors.centerIn: parent
    rotation: root.angleOf(root.value)

    Behavior on rotation {
      enabled: !Cpp_Misc_GraphicsBackend.reduceMotion && !area.pressed

      NumberAnimation {
        duration: 120
        easing.type: Easing.OutCubic
      }
    }

    Rectangle {
      radius: width / 2
      width: parent.width
      height: parent.height * 0.36
      color: Qt.rgba(0.96, 0.96, 0.95, 1)
      anchors {
        top: parent.top
        topMargin: parent.height * 0.04
        horizontalCenter: parent.horizontalCenter
      }
    }
  }

  //
  // Focus mark: a thin ring just outside the bezel
  //
  Rectangle {
    border.width: 2
    radius: width / 2
    color: "transparent"
    width: root.ring * 2 + 6
    height: root.ring * 2 + 6
    visible: root.activeFocus
    anchors.centerIn: parent
    border.color: Cpp_ThemeManager.colors["highlight"]
  }

  MouseArea {
    id: area

    anchors.fill: parent
    onPressed: (mouse) => {
      root.forceActiveFocus()
      root.propose(root.valueAt(mouse.x, mouse.y))
    }
    onPositionChanged: (mouse) => {
      if (area.pressed)
        root.propose(root.valueAt(mouse.x, mouse.y))
    }
    onWheel: (wheel) => {
      if (!root.activeFocus)
        return

      root.nudge(wheel.modifiers, wheel.angleDelta.y > 0 ? 1 : -1)
    }
  }
}
