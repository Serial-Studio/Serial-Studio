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
// A flush panel push button: coloured head in a knurled metal ring, lower and darker
// while held, lit with a halo when it is illuminated and on. Static geometry.
//
Item {
  id: root

  property bool lit: false
  property bool dimmed: false
  property bool illuminated: false
  property color headColor: Cpp_ThemeManager.colors["highlight"]

  readonly property real cx: width / 2
  readonly property real cy: height / 2
  readonly property bool held: touch.pressed
  readonly property real ring: root.side / 2
  readonly property real side: Math.min(width, height)
  readonly property real headRadius: root.ring * (root.held ? 0.71 : 0.755)
  //
  // Enamel of the head: a coloured head is lifted to a bright enamel, since deep inks picked
  // to read as text look dim on a button; a grey or black head is left as it is
  //
  readonly property color enamel: {
    const base = root.headColor
    if (base.hsvSaturation < 0.15)
      return base

    return Qt.hsva(Math.max(0, base.hsvHue), base.hsvSaturation * 0.92,
                   Math.max(base.hsvValue, 0.84), 1)
  }

  readonly property color face: {
    var tone = root.enamel
    if (root.illuminated)
      tone = root.lit ? Qt.lighter(root.enamel, 1.15) : Qt.darker(root.enamel, 1.7)

    return root.held ? Qt.darker(tone, 1.35) : tone
  }

  signal clicked()

  activeFocusOnTab: true
  opacity: root.dimmed ? 0.45 : 1.0
  Keys.onSpacePressed: root.clicked()
  Keys.onReturnPressed: root.clicked()

  Widgets.PanelBezel {
    haloReach: 1.3
    knurled: false
    width: root.side
    height: root.side
    haloColor: root.enamel
    anchors.centerIn: parent
    halo: root.illuminated && root.lit
  }

  Shape {
    smooth: true
    antialiasing: true
    anchors.fill: parent
    preferredRendererType: Shape.CurveRenderer

    ShapePath {
      strokeWidth: -1
      fillColor: Qt.rgba(0.24, 0.26, 0.28, 1)

      startY: root.cy
      startX: root.cx + root.ring * 0.785
      PathArc {
        y: root.cy
        radiusX: root.ring * 0.785
        radiusY: root.ring * 0.785
        x: root.cx - root.ring * 0.785
      }
      PathArc {
        y: root.cy
        radiusX: root.ring * 0.785
        radiusY: root.ring * 0.785
        x: root.cx + root.ring * 0.785
      }
    }

    ShapePath {
      strokeColor: Qt.darker(root.face, 1.45)
      strokeWidth: Math.max(0.75, root.headRadius * 0.02)
      fillGradient: RadialGradient {
        focalRadius: 0
        centerX: root.cx
        centerY: root.cy
        centerRadius: root.headRadius * 1.25
        focalX: root.cx - root.headRadius * 0.38
        focalY: root.cy - root.headRadius * 0.42
        GradientStop { position: 0.0; color: Qt.lighter(root.face, 1.45) }
        GradientStop { position: 0.35; color: Qt.lighter(root.face, 1.08) }
        GradientStop { position: 0.75; color: root.face }
        GradientStop { position: 1.0; color: Qt.darker(root.face, 1.45) }
      }

      startY: root.cy
      startX: root.cx + root.headRadius
      PathArc {
        y: root.cy
        radiusX: root.headRadius
        radiusY: root.headRadius
        x: root.cx - root.headRadius
      }
      PathArc {
        y: root.cy
        radiusX: root.headRadius
        radiusY: root.headRadius
        x: root.cx + root.headRadius
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
    width: root.side + 6
    height: root.side + 6
    visible: root.activeFocus
    anchors.centerIn: parent
    border.color: Cpp_ThemeManager.colors["highlight"]
  }

  MouseArea {
    id: touch

    width: root.side
    height: root.side
    anchors.centerIn: parent
    onClicked: {
      root.forceActiveFocus()
      root.clicked()
    }
  }
}
