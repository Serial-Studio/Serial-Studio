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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */


import QtQuick
import QtQuick.Shapes

//
// The knurled metal ring a panel lamp, push button or selector sits in, lit from the
// upper left, with an optional halo of the part's own light; static geometry.
//
Item {
  id: root

  property bool halo: false
  property bool knurled: true
  property real haloReach: 1.5
  property color haloColor: Cpp_ThemeManager.colors["alarm_ok"]

  readonly property color metalEdge: Cpp_ThemeManager.colors["shadow"]
  readonly property color metalMid: Qt.darker(Cpp_ThemeManager.colors["mid"], 1.1)
  readonly property color metalDark: Qt.darker(Cpp_ThemeManager.colors["dark"], 1.15)
  readonly property color metalLight: Qt.lighter(Cpp_ThemeManager.colors["mid"], 1.16)

  readonly property real cx: width / 2
  readonly property real cy: height / 2
  readonly property real ring: Math.min(width, height) / 2

  //
  // Radial ticks around the rim, one path for the whole ring
  //
  readonly property var knurl: {
    var segments = []
    if (!root.knurled || root.ring < 6)
      return segments

    const count = Math.max(24, Math.min(72, Math.round(root.ring * 3)))
    for (var i = 0; i < count; ++i) {
      const a = 2 * Math.PI * i / count
      const c = Math.cos(a)
      const s = Math.sin(a)
      segments.push([Qt.point(root.cx + root.ring * 0.84 * c, root.cy + root.ring * 0.84 * s),
                     Qt.point(root.cx + root.ring * 0.97 * c, root.cy + root.ring * 0.97 * s)])
    }

    return segments
  }

  Shape {
    id: ringShape

    smooth: true
    antialiasing: true
    anchors.fill: parent
    preferredRendererType: Shape.CurveRenderer

    readonly property real lip: root.ring * 0.8
    readonly property real drop: Math.max(1, root.ring * 0.06)

    ShapePath {
      strokeWidth: -1
      fillGradient: RadialGradient {
        focalRadius: 0
        focalX: root.cx
        centerX: root.cx
        centerRadius: root.ring * 1.12
        focalY: root.cy + ringShape.drop
        centerY: root.cy + ringShape.drop
        GradientStop { position: 0.82; color: Qt.rgba(0, 0, 0, 0.28) }
        GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0) }
      }

      startY: root.cy + ringShape.drop
      startX: root.cx + root.ring * 1.12
      PathArc {
        radiusX: root.ring * 1.12
        radiusY: root.ring * 1.12
        y: root.cy + ringShape.drop
        x: root.cx - root.ring * 1.12
      }
      PathArc {
        radiusX: root.ring * 1.12
        radiusY: root.ring * 1.12
        y: root.cy + ringShape.drop
        x: root.cx + root.ring * 1.12
      }
    }

    ShapePath {
      strokeColor: root.metalEdge
      strokeWidth: Math.max(0.75, root.ring * 0.018)
      fillGradient: LinearGradient {
        x1: root.cx - root.ring * 0.7
        y1: root.cy - root.ring * 0.7
        x2: root.cx + root.ring * 0.7
        y2: root.cy + root.ring * 0.7
        GradientStop { position: 0.0; color: root.metalLight }
        GradientStop { position: 0.5; color: root.metalMid }
        GradientStop { position: 1.0; color: root.metalDark }
      }

      startY: root.cy
      startX: root.cx + root.ring
      PathArc {
        y: root.cy
        radiusX: root.ring
        radiusY: root.ring
        x: root.cx - root.ring
      }
      PathArc {
        y: root.cy
        radiusX: root.ring
        radiusY: root.ring
        x: root.cx + root.ring
      }
    }

    ShapePath {
      fillColor: "transparent"
      capStyle: ShapePath.FlatCap
      strokeColor: Qt.rgba(0, 0, 0, 0.16)
      strokeWidth: Math.max(0.5, root.ring * 0.022)

      PathMultiline { paths: root.knurl }
    }

    ShapePath {
      fillColor: "transparent"
      strokeColor: Qt.rgba(0, 0, 0, 0.3)
      strokeWidth: Math.max(0.5, root.ring * 0.015)

      startY: root.cy
      startX: root.cx + ringShape.lip
      PathArc {
        y: root.cy
        radiusX: ringShape.lip
        radiusY: ringShape.lip
        x: root.cx - ringShape.lip
      }
      PathArc {
        y: root.cy
        radiusX: ringShape.lip
        radiusY: ringShape.lip
        x: root.cx + ringShape.lip
      }
    }
  }

  Shape {
    id: glow

    smooth: true
    antialiasing: true
    visible: root.halo
    anchors.fill: parent
    preferredRendererType: Shape.CurveRenderer

    readonly property real reach: root.ring * root.haloReach

    ShapePath {
      strokeWidth: -1
      fillGradient: RadialGradient {
        focalRadius: 0
        focalX: root.cx
        focalY: root.cy
        centerX: root.cx
        centerY: root.cy
        centerRadius: glow.reach
        GradientStop {
          position: 0.0
          color: Qt.rgba(root.haloColor.r, root.haloColor.g, root.haloColor.b, 0.6)
        }
        GradientStop {
          position: 1.0
          color: Qt.rgba(root.haloColor.r, root.haloColor.g, root.haloColor.b, 0)
        }
      }

      startY: root.cy
      startX: root.cx + glow.reach
      PathArc {
        y: root.cy
        radiusX: glow.reach
        radiusY: glow.reach
        x: root.cx - glow.reach
      }
      PathArc {
        y: root.cy
        radiusX: glow.reach
        radiusY: glow.reach
        x: root.cx + glow.reach
      }
    }
  }
}
