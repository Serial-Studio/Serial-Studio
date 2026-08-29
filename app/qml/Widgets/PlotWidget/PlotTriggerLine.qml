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
// Sweep-mode trigger indicator: dashed cursor-B-styled level line that fades out after
// each edit, so it stays legible while the trigger dialog is open without masking data.
//
Item {
  id: root

  //
  // The PlotWidget served; the trigger level and its world-to-pixel map are read from it
  //
  required property Item plot

  clip: true
  visible: root.plot.sweepMode && root.plot.triggerEditing

  readonly property color lineColor: Cpp_ThemeManager.colors["plot_cursor_b"]
  readonly property real lineY: root.plot.worldToPixelY(root.plot.triggerLevel)
  readonly property bool inRange: root.plot.triggerLevel >= root.plot.yVisibleMin
                                  && root.plot.triggerLevel <= root.plot.yVisibleMax

  //
  // Restart the fade; only meaningful while the dialog is editing
  //
  function flash() {
    if (!root.plot.sweepMode || !root.plot.triggerEditing)
      return

    _fade.stop()
    _triggerContent.opacity = 1
    _fade.start()
  }

  onVisibleChanged: {
    if (visible)
      flash()
  }

  Connections {
    target: root.plot
    enabled: root.plot.triggerEditing
    function onTriggerLevelChanged() { root.flash() }
  }

  Item {
    id: _triggerContent

    anchors.fill: parent
    visible: root.inRange

    SequentialAnimation {
      id: _fade

      PauseAnimation { duration: 2500 }
      NumberAnimation {
        to: 0
        duration: 750
        property: "opacity"
        target: _triggerContent
        easing.type: Easing.InOutQuad
      }
    }

    Canvas {
      id: _triggerCanvas

      x: 0
      height: 2
      y: root.lineY - 1
      width: parent.width

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

      onWidthChanged: requestPaint()

      Connections {
        target: root
        function onLineColorChanged() { _triggerCanvas.requestPaint() }
      }
    }

    Label {
      text: "T"
      padding: 4
      color: Cpp_ThemeManager.colors["widget_base"]
      font: (Cpp_Misc_CommonFonts.widgetFontRevision,
             Cpp_Misc_CommonFonts.widgetFont(0.9, true))
      background: Rectangle {
        radius: 3
        opacity: 0.9
        color: root.lineColor
      }
      anchors {
        rightMargin: 5
        right: parent.right
        verticalCenter: parent.top
        verticalCenterOffset: root.lineY
      }
    }
  }
}
