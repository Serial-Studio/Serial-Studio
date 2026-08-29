/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

//
// Edge layer of the project flow diagram: bezier connectors with arrow heads.
// Repaints are explicit; assign `arrows` then call requestPaint().
//
Canvas {
  id: canvas

  //
  // Input properties
  //
  property real zoom: 1.0
  property var arrows: []

  onPaint: {
    const ctx = getContext("2d")
    ctx.clearRect(0, 0, width, height)

    const z = canvas.zoom

    for (const a of canvas.arrows) {
      const x1 = a.x1 * z, y1 = a.y1 * z
      const x2 = a.x2 * z, y2 = a.y2 * z

      ctx.strokeStyle = Cpp_ThemeManager.colors["mid"]
      ctx.fillStyle   = Cpp_ThemeManager.colors["mid"]
      ctx.lineWidth   = 1.5 * z
      ctx.setLineDash([])

      const hl  = 7 * z
      const sin = Math.sin(Math.PI / 6)
      ctx.setLineDash([])

      if (a.verticalEnd) {
        const dirY = (y2 >= y1) ? 1 : -1
        const dirX = (x2 >= x1) ? 1 : -1
        const hly  = hl * dirY
        const y2a  = a.noHead ? y2 : (y2 - hly)

        const exitDx = Math.max(30 * z, Math.abs(x2 - x1) * 0.3) * dirX
        const c1x    = x1 + exitDx
        const c1y    = y1
        const c2x    = x2
        const c2y    = (y1 + y2a) / 2

        ctx.beginPath()
        ctx.moveTo(x1, y1)
        ctx.bezierCurveTo(c1x, c1y, c2x, c2y, x2, y2a)
        ctx.stroke()

        if (!a.noHead) {
          ctx.beginPath()
          ctx.moveTo(x2, y2)
          ctx.lineTo(x2 - hl * sin, y2 - hly)
          ctx.lineTo(x2 + hl * sin, y2 - hly)
          ctx.closePath()
          ctx.fill()
        }
      } else {
        const dirX = (x2 >= x1) ? 1 : -1
        const hlx  = hl * dirX
        const x2a  = a.noHead ? x2 : (x2 - hlx)
        const mx   = (x1 + x2a) / 2

        ctx.beginPath()
        ctx.moveTo(x1, y1)
        ctx.bezierCurveTo(mx, y1, mx, y2, x2a, y2)
        ctx.stroke()

        if (!a.noHead) {
          ctx.beginPath()
          ctx.moveTo(x2, y2)
          ctx.lineTo(x2 - hlx, y2 - hl * sin)
          ctx.lineTo(x2 - hlx, y2 + hl * sin)
          ctx.closePath()
          ctx.fill()
        }
      }
    }
  }

  Connections {
    target: Cpp_ThemeManager
    function onThemeChanged() { canvas.requestPaint() }
  }

  onWidthChanged:  requestPaint()
  onHeightChanged: requestPaint()
}
