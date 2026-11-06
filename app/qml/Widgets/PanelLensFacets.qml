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
// Facet grid and specular highlight over a panel lamp's lens: translucent white only,
// and the facets are chords of the lens circle, so no clip is needed.
//
Item {
  id: root

  property bool lit: false

  readonly property real cx: width / 2
  readonly property real cy: height / 2
  readonly property real lens: Math.min(width, height) / 2 * 0.92

  readonly property var facets: {
    var segments = []
    if (root.lens < 4)
      return segments

    const step = root.lens / 2.5
    for (var n = -2; n <= 2; ++n) {
      const offset = n * step
      const half = Math.sqrt(Math.max(0, root.lens * root.lens - offset * offset))
      segments.push([Qt.point(root.cx + offset, root.cy - half),
                     Qt.point(root.cx + offset, root.cy + half)])
      segments.push([Qt.point(root.cx - half, root.cy + offset),
                     Qt.point(root.cx + half, root.cy + offset)])
    }

    return segments
  }

  Shape {
    id: shine

    smooth: true
    antialiasing: true
    anchors.fill: parent
    preferredRendererType: Shape.CurveRenderer

    readonly property real rx: root.lens * 0.3
    readonly property real ry: root.lens * 0.23
    readonly property real px: root.cx - root.lens * 0.36
    readonly property real py: root.cy - root.lens * 0.36

    ShapePath {
      fillColor: "transparent"
      capStyle: ShapePath.FlatCap
      strokeWidth: Math.max(0.5, root.lens * 0.045)
      strokeColor: Qt.rgba(1, 1, 1, root.lit ? 0.26 : 0.14)

      PathMultiline { paths: root.facets }
    }

    ShapePath {
      strokeWidth: -1
      fillGradient: RadialGradient {
        focalRadius: 0
        centerX: root.cx
        centerY: root.cy
        centerRadius: root.lens
        focalX: root.cx - root.lens * 0.3
        focalY: root.cy - root.lens * 0.35
        GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, root.lit ? 0.3 : 0.2) }
        GradientStop { position: 0.6; color: Qt.rgba(1, 1, 1, 0) }
        GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.3) }
      }

      startY: root.cy
      startX: root.cx + root.lens
      PathArc {
        y: root.cy
        radiusX: root.lens
        radiusY: root.lens
        x: root.cx - root.lens
      }
      PathArc {
        y: root.cy
        radiusX: root.lens
        radiusY: root.lens
        x: root.cx + root.lens
      }
    }

    ShapePath {
      strokeWidth: -1
      fillColor: Qt.rgba(1, 1, 1, root.lit ? 0.55 : 0.35)

      startY: shine.py
      startX: shine.px + shine.rx
      PathArc {
        y: shine.py
        radiusX: shine.rx
        radiusY: shine.ry
        x: shine.px - shine.rx
      }
      PathArc {
        y: shine.py
        radiusX: shine.rx
        radiusY: shine.ry
        x: shine.px + shine.rx
      }
    }
  }
}
