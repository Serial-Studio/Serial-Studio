/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick

//
// Blink phase behind the Bar, Gauge and Meter readouts: only a Critical band flashes, a Warning
// band holds a steady tint, so a persistent out-of-normal reading never strobes the dashboard.
//
SequentialAnimation {
  id: root

  required property bool hasData
  required property int severity
  required property bool triggered

  property bool flashOn: false

  readonly property bool blinking: severity === 3 && hasData
  readonly property bool filled: triggered && (flashOn || !blinking)
  readonly property color fill: Cpp_ThemeManager.alarmColorForSeverity(root.severity)

  //
  // Text drawn on the flash fill: dark only on a light fill such as a caution yellow, else white;
  // WCAG luminance put dark ink on the reds and dark ambers, where white reads better
  //
  readonly property color ink: {
    const light = 0.2126 * root.fill.r + 0.7152 * root.fill.g + 0.0722 * root.fill.b
    return light > 0.55 ? "#1a1a1a" : "#ffffff"
  }

  running: root.blinking
  loops: Animation.Infinite
  onRunningChanged: if (!running) root.flashOn = false

  PropertyAction { target: root; property: "flashOn"; value: true }
  PauseAnimation { duration: 450 }
  PropertyAction { target: root; property: "flashOn"; value: false }
  PauseAnimation { duration: 450 }
}
