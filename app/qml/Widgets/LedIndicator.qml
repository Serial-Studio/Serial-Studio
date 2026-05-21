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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Effects

import SerialStudio

//
// LED indicator with the same bloom-glow effect as the dashboard LED panel.
// Use `on` to switch between the lit color and a darkened off color.
//
Item {
  id: root

  property bool on: false
  property real diameter: 18
  property color onColor: Cpp_ThemeManager.colors["highlight"]
  property color offColor: Cpp_ThemeManager.colors["alarm"]

  implicitWidth: diameter
  implicitHeight: diameter

  Rectangle {
    id: led

    anchors.centerIn: parent
    width: root.diameter
    height: root.diameter
    radius: width / 2
    border.width: 1
    border.color: Cpp_ThemeManager.colors["widget_border"]
    color: root.on ? root.onColor : Qt.darker(root.offColor, 1.2)

    Behavior on color {
      ColorAnimation { duration: 180 }
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
