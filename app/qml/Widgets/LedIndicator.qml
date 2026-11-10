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
import QtQuick.Effects

import SerialStudio

//
// LED indicator with the dashboard LED panel's bloom; the lamp look (spec 0113) adds the
// panel's bezel and faceted lens. `on` switches between the lit and the darkened off color.
//
Item {
  id: root

  property bool on: false
  property real diameter: 18
  property color onColor: Cpp_ThemeManager.colors["highlight"]
  property color offColor: Cpp_ThemeManager.colors["alarm"]

  readonly property bool lampLook: Cpp_ThemeManager.widgetLooks["led"] === "lamp"
  readonly property color lampColor: root.on ? root.onColor : root.offColor

  implicitWidth: diameter
  implicitHeight: diameter

  Loader {
    height: width
    active: root.lampLook
    anchors.centerIn: led
    width: Math.round(led.width * 1.45)

    sourceComponent: PanelBezel {
      halo: root.on
      haloColor: root.onColor
    }
  }

  Rectangle {
    id: led

    anchors.centerIn: parent
    width: root.diameter
    height: root.diameter
    radius: width / 2
    border.width: 1
    border.color: root.lampLook ? Cpp_ThemeManager.colors["window_border"]
                                : Cpp_ThemeManager.colors["widget_border"]
    color: {
      if (!root.lampLook)
        return root.on ? root.onColor : Qt.darker(root.offColor, 1.2)

      const hue = Math.max(0, root.lampColor.hsvHue)
      if (root.on)
        return Qt.hsva(hue, root.lampColor.hsvSaturation, Math.max(0.96, root.lampColor.hsvValue), 1)

      return Qt.hsva(hue, root.lampColor.hsvSaturation * 0.7,
                     Math.min(1, root.lampColor.hsvValue * 0.55 + 0.12), 1)
    }

    Behavior on color {
      ColorAnimation { duration: 180 }
    }
  }

  Loader {
    anchors.fill: led
    active: root.lampLook

    sourceComponent: PanelLensFacets {
      lit: root.on
    }
  }

  MultiEffect {
    source: led
    anchors.fill: led

    blurEnabled: true
    blur: root.on ? 1 : 0
    blurMax: root.on ? 64 : 0
    brightness: root.on ? 0.4 : 0
    saturation: root.on ? 0.2 : 0
    visible: Cpp_Misc_GraphicsBackend.effectsEnabled
    enabled: Cpp_Misc_GraphicsBackend.effectsEnabled
  }

  MultiEffect {
    source: led
    anchors.fill: led

    blurEnabled: true
    blur: root.on ? 0.3 : 0
    blurMax: root.on ? 64 : 0
    brightness: root.on ? 0.4 : 0
    saturation: root.on ? 0.2 : 0
    visible: Cpp_Misc_GraphicsBackend.effectsEnabled
    enabled: Cpp_Misc_GraphicsBackend.effectsEnabled
  }
}
