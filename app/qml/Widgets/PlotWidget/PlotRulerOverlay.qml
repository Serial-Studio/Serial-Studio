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
// X-axis ruler chrome (spec 0058): the zero line, the named markers and the hover marker.
// Fills the plot-area overlay, so every child measures against the plot area.
//
Item {
  id: root

  //
  // The PlotWidget served; the ruler state and the world-to-pixel map are read from it
  //
  required property Item plot

  //
  // Pointer position (plot-area pixels) and whether the hover marker may track it
  //
  required property real pointerX
  required property bool pointerActive

  //
  // Ruler zero line: drawn wherever the user set t = 0, with a chip on the top edge
  //
  Item {
    id: _zeroLine

    width: 1
    height: parent.height
    x: root.plot.worldToPixelX(root.plot.xZero)
    visible: root.plot.xZeroSet && !root.plot.logX && root.plot.xZero >= root.plot.xVisibleMin
             && root.plot.xZero <= root.plot.xVisibleMax

    Rectangle {
      width: 1
      opacity: 0.7
      height: parent.height
      color: Cpp_ThemeManager.colors["widget_text"]
    }

    Label {
      text: "0"
      padding: 3
      color: Cpp_ThemeManager.colors["widget_base"]
      font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.8, true))
      background: Rectangle {
        radius: 3
        opacity: 0.9
        color: Cpp_ThemeManager.colors["widget_text"]
      }
      anchors {
        topMargin: 4
        leftMargin: 3
        top: parent.top
        left: parent.right
      }
    }
  }

  //
  // Named markers: a thin line and a name chip, only while inside the visible window
  //
  Repeater {
    model: root.plot.xMarkers

    delegate: Item {
      id: _markerItem

      required property int index
      required property var modelData

      width: 1
      height: parent.height
      x: root.plot.worldToPixelX(modelData.x)
      visible: modelData.x >= root.plot.xVisibleMin && modelData.x <= root.plot.xVisibleMax

      Rectangle {
        width: 1
        opacity: 0.8
        height: parent.height
        color: Cpp_ThemeManager.colors["widget_highlight"]
      }

      Label {
        padding: 3
        text: _markerItem.modelData.name
        color: Cpp_ThemeManager.colors["widget_highlighted_text"]
        font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.8, true))
        background: Rectangle {
          radius: 3
          opacity: 0.9
          color: Cpp_ThemeManager.colors["widget_highlight"]
        }
        anchors {
          topMargin: 4
          leftMargin: 3
          top: parent.top
          left: parent.right
        }
      }
    }
  }

  //
  // Hover marker: tracks the pointer with an X readout while enabled
  //
  Item {
    id: _hoverMarker

    width: 1
    x: root.pointerX
    height: parent.height
    visible: root.plot.hoverMarkerEnabled && root.pointerActive

    Rectangle {
      width: 1
      opacity: 0.5
      height: parent.height
      color: Cpp_ThemeManager.colors["widget_text"]
    }

    Label {
      padding: 3
      color: Cpp_ThemeManager.colors["widget_base"]
      text: root.plot.displayValueX(root.plot.pixelToWorldX(root.pointerX))
      font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.8))
      background: Rectangle {
        radius: 3
        opacity: 0.85
        color: Cpp_ThemeManager.colors["widget_text"]
      }
      anchors {
        leftMargin: 3
        bottomMargin: 4
        left: parent.right
        bottom: parent.bottom
      }
    }
  }
}
