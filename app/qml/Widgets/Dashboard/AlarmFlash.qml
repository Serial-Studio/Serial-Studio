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

  running: root.blinking
  loops: Animation.Infinite
  onRunningChanged: if (!running) root.flashOn = false

  PropertyAction { target: root; property: "flashOn"; value: true }
  PauseAnimation { duration: 450 }
  PropertyAction { target: root; property: "flashOn"; value: false }
  PauseAnimation { duration: 450 }
}
