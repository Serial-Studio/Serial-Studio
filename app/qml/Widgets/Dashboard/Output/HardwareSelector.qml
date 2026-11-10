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
// Rotary selector switch (spec 0113): a black knob with a white pointer in a metal ring,
// one detent mark and one label per position. Click a position, or use the arrow keys.
//
Item {
  id: root

  property real to: 2
  property real from: 0
  property real step: 1
  property real value: 0
  property var labels: []
  property bool dimmed: false
  property bool springReturn: false
  property bool keyHeld: false

  signal moved(real value)
  signal released()

  readonly property bool pressed: area.pressed || root.keyHeld

  readonly property real cx: width / 2
  readonly property real cy: height / 2
  readonly property bool labelRow: root.count <= 3
  readonly property real ring: Math.min(width, height) / 2 * 0.56
  readonly property real span: Math.min(270, (root.count - 1) * 60)
  readonly property int count: {
    const n = Math.round((root.to - root.from) / Math.max(root.step, 1e-9)) + 1
    return Math.max(2, Math.min(12, n))
  }
  readonly property int position: {
    const index = Math.round((root.value - root.from) / Math.max(root.step, 1e-9))
    return Math.max(0, Math.min(root.count - 1, index))
  }

  //
  // Degrees clockwise from straight up for one position
  //
  function angleAt(index) {
    return -root.span / 2 + root.span * index / (root.count - 1)
  }

  //
  // Moves to a position and reports its value
  //
  function select(index) {
    const clamped = Math.max(0, Math.min(root.count - 1, index))
    if (clamped !== root.position)
      root.moved(root.from + clamped * root.step)
  }

  //
  // Position nearest to a point of the item
  //
  function nearest(px, py) {
    const degrees = Math.atan2(px - root.cx, root.cy - py) * 180 / Math.PI
    var best = 0
    var gap = 1e9
    for (var i = 0; i < root.count; ++i) {
      const d = Math.abs(degrees - root.angleAt(i))
      if (d < gap) {
        gap = d
        best = i
      }
    }

    return best
  }

  activeFocusOnTab: true
  opacity: root.dimmed ? 0.45 : 1.0
  Keys.onLeftPressed: (event) => root.stepBy(event, -1)
  Keys.onRightPressed: (event) => root.stepBy(event, 1)
  Keys.onReleased: (event) => {
    if (!root.springReturn || event.isAutoRepeat || !root.keyHeld)
      return

    if (event.key !== Qt.Key_Left && event.key !== Qt.Key_Right)
      return

    root.keyHeld = false
    root.released()
  }

  //
  // Arrow keys step one position; a spring-return selector holds it until the key is let go
  //
  function stepBy(event, delta) {
    if (event.isAutoRepeat && root.springReturn)
      return

    root.keyHeld = root.springReturn
    root.select(root.position + delta)
  }

  //
  // The position a press or click picks: the other one of two, else the nearest mark
  //
  function picked(mouse) {
    return root.count === 2 ? (root.position === 0 ? 1 : 0) : root.nearest(mouse.x, mouse.y)
  }

  Widgets.PanelBezel {
    knurled: false
    width: root.ring * 2
    height: root.ring * 2
    anchors.centerIn: parent
  }

  //
  // Detent marks just outside the ring; the one the pointer stands at is highlighted
  //
  Repeater {
    model: root.count

    delegate: Rectangle {
      required property int index

      readonly property real reach: root.ring * 1.16
      readonly property real turn: root.angleAt(index) * Math.PI / 180

      radius: width / 2
      antialiasing: true
      rotation: root.angleAt(index)
      height: Math.max(4, root.ring * 0.16)
      width: Math.max(2, root.ring * 0.055)
      x: root.cx + reach * Math.sin(turn) - width / 2
      y: root.cy - reach * Math.cos(turn) - height / 2
      color: index === root.position ? Cpp_ThemeManager.colors["highlight"]
                                     : Cpp_ThemeManager.colors["widget_text"]
    }
  }

  //
  // Position labels: two or three positions share one line above the switch, each in its
  // own slot, so they never sit at different heights; more positions go around the marks
  //
  Repeater {
    model: root.count

    delegate: Text {
      required property int index

      readonly property real slot: root.width / root.count
      readonly property real turn: root.angleAt(index) * Math.PI / 180
      readonly property real reach: root.ring * 1.34 + Math.max(width, height) / 2

      elide: Text.ElideRight
      horizontalAlignment: Text.AlignHCenter
      color: Cpp_ThemeManager.colors["widget_text"]
      width: root.labelRow ? Math.min(implicitWidth, slot - 6) : implicitWidth
      font: Cpp_Misc_CommonFonts.customUiFont(0.8, index === root.position)
      y: root.labelRow ? Math.max(0, root.cy - root.ring * 1.3 - height - 2)
                       : root.cy - reach * Math.cos(turn) - height / 2
      x: root.labelRow ? index * slot + (slot - width) / 2
                       : root.cx + reach * Math.sin(turn) - width / 2
      text: index < root.labels.length ? root.labels[index] : (root.from + index * root.step)
    }
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
      startX: root.cx + root.ring * 0.74
      PathArc {
        y: root.cy
        radiusX: root.ring * 0.74
        radiusY: root.ring * 0.74
        x: root.cx - root.ring * 0.74
      }
      PathArc {
        y: root.cy
        radiusX: root.ring * 0.74
        radiusY: root.ring * 0.74
        x: root.cx + root.ring * 0.74
      }
    }
  }

  //
  // Lever across the knob; its pointing half is white
  //
  Item {
    id: lever

    width: root.ring * 0.6
    height: root.ring * 1.46
    anchors.centerIn: parent
    rotation: root.angleAt(root.position)

    Behavior on rotation {
      enabled: !Cpp_Misc_GraphicsBackend.reduceMotion

      NumberAnimation {
        duration: 140
        easing.type: Easing.OutCubic
      }
    }

    Rectangle {
      border.width: 1
      anchors.fill: parent
      radius: width * 0.28
      color: Qt.rgba(0.09, 0.1, 0.12, 1)
      border.color: Qt.rgba(0.29, 0.31, 0.35, 1)
    }

    Rectangle {
      border.width: 1
      radius: width * 0.26
      width: parent.width * 0.88
      height: parent.height * 0.5
      color: Qt.rgba(0.96, 0.96, 0.95, 1)
      border.color: Qt.rgba(0.72, 0.73, 0.74, 1)
      anchors {
        topMargin: 2
        top: parent.top
        horizontalCenter: parent.horizontalCenter
      }
    }
  }

  MouseArea {
    id: area

    anchors.fill: parent
    onPressed: (mouse) => {
      root.forceActiveFocus()
      if (root.springReturn)
        root.select(root.picked(mouse))
    }
    onClicked: (mouse) => {
      if (!root.springReturn)
        root.select(root.picked(mouse))
    }
    onReleased: {
      if (root.springReturn)
        root.released()
    }
    onCanceled: {
      if (root.springReturn)
        root.released()
    }
    onWheel: (wheel) => {
      if (!root.springReturn)
        root.select(root.position + (wheel.angleDelta.y > 0 ? 1 : -1))
    }
  }
}
