/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
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
