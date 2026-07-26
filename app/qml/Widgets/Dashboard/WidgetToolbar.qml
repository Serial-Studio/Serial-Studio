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
import QtQuick.Layouts

Item {
  id: root

  //
  // Host API: buttons as children; windowRoot supplies frozen (undefined on external
  // pop-outs reads not-frozen); minWidgetHeight hides the bar, available gates it
  //
  default property alias content: row.data
  property var windowRoot: null
  property bool available: true
  property int minWidgetHeight: 220

  //
  // Visibility policy owner: frozen dashboards and too-short widgets hide the bar;
  // a too-narrow widget never hides it, the content scrolls horizontally instead
  //
  readonly property bool frozen: root.windowRoot ? root.windowRoot.frozen === true : false
  readonly property bool shown: root.available && !root.frozen
                                && (root.parent ? root.parent.height >= root.minWidgetHeight
                                                : false)

  visible: root.shown
  height: implicitHeight
  implicitHeight: root.shown ? 48 : 0

  //
  // Horizontally scrollable button strip
  //
  Flickable {
    id: flick

    clip: true
    anchors.fill: parent
    contentHeight: height
    contentWidth: row.implicitWidth
    interactive: contentWidth > width
    boundsBehavior: Flickable.StopAtBounds
    flickableDirection: Flickable.HorizontalFlick

    RowLayout {
      id: row

      spacing: 4
      height: 48
      LayoutMirroring.childrenInherit: true
      width: Math.max(implicitWidth, flick.width)
      LayoutMirroring.enabled: Cpp_Misc_Translator.rtl
    }

    WheelHandler {
      onWheel: (wheel) => {
        const max = Math.max(0, flick.contentWidth - flick.width)
        const next = flick.contentX - (wheel.angleDelta.y + wheel.angleDelta.x) / 2
        flick.contentX = Math.max(0, Math.min(max, next))
      }
    }
  }

  //
  // Edge fades hinting at scrollable overflow
  //
  Rectangle {
    width: 16
    visible: flick.interactive && flick.contentX > 0

    anchors {
      top: parent.top
      left: parent.left
      bottom: parent.bottom
    }

    gradient: Gradient {
      orientation: Gradient.Horizontal

      GradientStop {
        position: 0
        color: Cpp_ThemeManager.colors["window_toolbar_background"]
      }

      GradientStop {
        position: 1
        color: "transparent"
      }
    }
  }

  Rectangle {
    width: 16
    visible: flick.interactive && flick.contentX < flick.contentWidth - flick.width

    anchors {
      top: parent.top
      right: parent.right
      bottom: parent.bottom
    }

    gradient: Gradient {
      orientation: Gradient.Horizontal

      GradientStop {
        position: 0
        color: "transparent"
      }

      GradientStop {
        position: 1
        color: Cpp_ThemeManager.colors["window_toolbar_background"]
      }
    }
  }
}
