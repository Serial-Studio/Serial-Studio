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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

//
// Spec 0127 R23: the name and Y range of each lane of a multi-plot in lane mode, with a
// separator between bands. Laid over the plot's curve layer; draws nothing in overlay mode.
//
Item {
  id: root

  //
  // The PlotWidget served, the curve index shown in each band (top to bottom), and the
  // per-curve titles, colors and {min, max} ranges, all indexed by curve
  //
  required property Item plot
  required property var bands
  required property var titles
  required property var colors
  required property var ranges

  readonly property real bandHeight: root.bands.length > 0 ? root.height / root.bands.length : 0

  //
  // Engineering-style number that stays short enough for a band's corner
  //
  function rangeText(value) {
    if (value === undefined || !isFinite(value))
      return "--"

    const magnitude = Math.abs(value)
    if (magnitude !== 0 && (magnitude >= 1e6 || magnitude < 1e-3))
      return value.toExponential(2)

    return String(parseFloat(value.toPrecision(5)))
  }

  z: 9
  x: root.plot.curveLayer.x
  y: root.plot.curveLayer.y
  width: root.plot.curveLayer.width
  height: root.plot.curveLayer.height

  Repeater {
    model: root.bands

    delegate: Item {
      id: band

      required property int index
      required property int modelData

      readonly property var range: root.ranges[band.modelData]

      width: root.width
      height: root.bandHeight
      y: band.index * root.bandHeight

      Rectangle {
        height: 1
        width: parent.width
        visible: band.index > 0
        color: Cpp_ThemeManager.colors["widget_border"]
      }

      Rectangle {
        x: 4
        y: 3
        radius: 3
        width: _text.implicitWidth + 10
        height: _text.implicitHeight + 2
        visible: root.bandHeight >= _text.implicitHeight + 6
        color: Qt.alpha(Cpp_ThemeManager.colors["widget_base"], 0.85)

        Label {
          id: _text

          anchors.centerIn: parent
          font: Cpp_Misc_CommonFonts.monoFont
          color: root.colors[band.modelData]
          text: qsTr("%1  %2 to %3").arg(root.titles[band.modelData])
                                    .arg(root.rangeText(band.range ? band.range.min : undefined))
                                    .arg(root.rangeText(band.range ? band.range.max : undefined))
        }
      }
    }
  }
}
