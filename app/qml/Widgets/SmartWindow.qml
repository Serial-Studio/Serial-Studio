/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

import QtCore
import QtQuick
import QtQuick.Window

Window {
  id: root

  //
  // Custom properties
  //
  property string category: ""
  property real previousX: 0
  property real previousY: 0
  property real previousWidth: 0
  property real previousHeight: 0
  property bool isMaximized: false
  property bool isChangingSize: false

  //
  // Timer to delay saving until animations are complete
  //
  Timer {
    id: saveTimer
    interval: 300
    repeat: false
    onTriggered: {
      if (!root.isMaximized && root.visible && root.visibility !== Window.Maximized) {
        root.previousX = root.x
        root.previousY = root.y
        root.previousWidth = root.width
        root.previousHeight = root.height
      }

      root.isChangingSize = false
    }
  }

  //
  // Save previous values for maximize/unmaximize cycle with delay
  //
  function savePreviousDimensions() {
    if (!isChangingSize) {
      isChangingSize = true
      saveTimer.restart()
    }
  }

  //
  // Save previous values for maximize/unmaximize cycle with delay
  //
  x: 100
  y: 100
  onXChanged: savePreviousDimensions()
  onYChanged: savePreviousDimensions()
  onWidthChanged: savePreviousDimensions()
  onHeightChanged: savePreviousDimensions()

  //
  // Ensure that window is visible
  //
  function displayWindow() {
    if (root.isMaximized) {
      root.showMaximized()
    } else {
      if (root.x > Screen.desktopAvailableWidth - root.minimumWidth || root.x <= 0)
        root.x = (Screen.desktopAvailableWidth - root.minimumWidth) / 2
      if (root.y > Screen.desktopAvailableHeight - root.minimumHeight || root.y <= 0)
        root.y = (Screen.desktopAvailableHeight - root.minimumHeight) / 2
      if (root.width >= Screen.desktopAvailableWidth - 100)
        root.width = root.minimumWidth
      if (root.height >= Screen.desktopAvailableHeight - 100)
        root.height = root.minimumHeight

      root.showNormal()
    }

    root.raise()
    root.requestActivate()
  }

  //
  // React to maximize/unmaximize event
  //
  onVisibilityChanged: {
    if (root.visible) {
      if (root.visibility === Window.Maximized) {
        root.isMaximized = true
      } else if (root.isMaximized && root.visibility !== Window.Minimized) {
        root.isMaximized = false
        root.x = root.previousX
        root.y = root.previousY
        root.width = root.previousWidth
        root.height = root.previousHeight
      }
    }
  }

  //
  // Close shortcut
  //
  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  //
  // Quit shortcut
  //
  Shortcut {
    sequences: [StandardKey.Quit]
    onActivated: root.close()
  }

  //
  // Ensure that window size stays within minimum size
  //
  Component.onCompleted: {
    // Set initial window position within the primary screen bounds
    const screenGeometry = Cpp_PrimaryScreen.geometry
    if (root.x < screenGeometry.x || root.x > screenGeometry.x + screenGeometry.width - root.width)
      root.x = screenGeometry.x + (screenGeometry.width - root.width) / 2
    if (root.y < screenGeometry.y || root.y > screenGeometry.y + screenGeometry.height - root.height)
      root.y = screenGeometry.y + (screenGeometry.height - root.height) / 2

    // Ensure minimum width and height
    if (width < minimumWidth)
      width = minimumWidth
    if (height < minimumHeight)
      height = minimumHeight

    if (width < minimumWidth)
      width = minimumWidth
    if (height < minimumHeight)
      height = minimumHeight
  }

  //
  // Save settings
  //
  Settings {
    category: root.category
    property alias ax: root.x
    property alias ay: root.y
    property alias aw: root.width
    property alias ah: root.height
    property alias am: root.isMaximized
    property alias px: root.previousX
    property alias py: root.previousY
    property alias pw: root.previousWidth
    property alias ph: root.previousHeight
  }
}
