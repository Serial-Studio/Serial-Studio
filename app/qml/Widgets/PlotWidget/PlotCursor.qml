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
import QtQuick.Controls

//
// One measurement cursor: crosshair, center marker, chip and the two axis readouts. Fills
// the overlay so the readouts hang off its edges, while the crosshair rides a point item.
//
Item {
  id: root

  //
  // The PlotWidget served; the world-to-pixel map and the cursor mode are read from it
  //
  required property Item plot

  //
  // Identifier drawn on the chip ("A" / "B")
  //
  required property string name

  //
  // Cursor position in world coordinates, and whether it has been placed
  //
  required property real worldX
  required property real worldY
  required property bool cursorVisible

  //
  // Visibility gates the plot already computed: fully in view, and per-axis in range
  //
  required property bool inView
  required property bool xInRange
  required property bool yInRange

  //
  // Cursor palette (line/chip fill and the text drawn on it)
  //
  required property color lineColor
  required property color textColor

  //
  // Zero-sized point at the cursor position; the crosshair lines measure against the
  // overlay through it, exactly as the center marker and chip anchor to it
  //
  Item {
    id: _point

    x: root.plot.worldToPixelX(root.worldX)
    y: root.plot.worldToPixelY(root.worldY)
    visible: root.plot.cursorMode && root.cursorVisible && (root.xInRange || root.yInRange)

    //
    // Vertical line with dash-dot pattern (full height) - visible when X is in range
    //
    Canvas {
      id: _vertical

      x: 0
      width: 2
      y: -parent.y
      visible: root.xInRange
      height: parent.parent.height

      onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        ctx.strokeStyle = root.lineColor
        ctx.lineWidth = 2
        ctx.setLineDash([8, 4, 2, 4])
        ctx.lineDashOffset = 0

        ctx.beginPath()
        ctx.moveTo(1, 0)
        ctx.lineTo(1, height)
        ctx.stroke()
      }
    }

    //
    // Horizontal line with dash-dot pattern (full width) - visible when Y is in range
    //
    Canvas {
      id: _horizontal

      y: 0
      height: 2
      x: -parent.x
      visible: root.yInRange
      width: parent.parent.width

      onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        ctx.strokeStyle = root.lineColor
        ctx.lineWidth = 2
        ctx.setLineDash([8, 4, 2, 4])
        ctx.lineDashOffset = 0

        ctx.beginPath()
        ctx.moveTo(0, 1)
        ctx.lineTo(width, 1)
        ctx.stroke()
      }
    }

    //
    // Center marker - only visible when fully in view
    //
    Rectangle {
      width: 10
      radius: 5
      height: 10
      border.width: 2
      color: root.lineColor
      anchors.centerIn: parent
      visible: root.inView
      border.color: Cpp_ThemeManager.colors["widget_base"]
    }

    //
    // Cursor identifier label - only visible when fully in view
    //
    Label {
      padding: 4
      text: root.name
      visible: root.inView
      color: root.textColor
      font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.9, true))
      background: Rectangle {
        radius: 3
        opacity: 0.9
        color: root.lineColor
      }
      anchors {
        topMargin: 5
        leftMargin: 5
        left: parent.right
        top: parent.bottom
      }
    }

    //
    // Trigger repaint when cursor moves
    //
    onXChanged: {
      _vertical.requestPaint()
      _horizontal.requestPaint()
    }
    onYChanged: {
      _vertical.requestPaint()
      _horizontal.requestPaint()
    }
  }

  //
  // X position label (on X-axis) - visible when X is in range
  //
  Label {
    padding: 4
    color: root.textColor
    text: root.plot.displayValueX(root.worldX)
    visible: root.plot.cursorMode && root.cursorVisible && root.xInRange
    font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.8))

    background: Rectangle {
      radius: 3
      opacity: 0.9
      color: root.lineColor
    }

    x: Math.max(0, Math.min(parent.width - width,
                            root.plot.worldToPixelX(root.worldX) - width / 2))
    anchors {
      topMargin: 2
      top: parent.bottom
    }
  }

  //
  // Y position label (on Y-axis) - visible when Y is in range
  //
  Label {
    padding: 4
    color: root.textColor
    text: root.plot.displayValueY(root.worldY)
    visible: root.plot.cursorMode && root.cursorVisible && root.yInRange
    font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.8))

    background: Rectangle {
      radius: 3
      opacity: 0.9
      color: root.lineColor
    }

    y: Math.max(0, Math.min(parent.height - height,
                            root.plot.worldToPixelY(root.worldY) - height / 2))
    anchors {
      rightMargin: 2
      right: parent.left
    }
  }
}
