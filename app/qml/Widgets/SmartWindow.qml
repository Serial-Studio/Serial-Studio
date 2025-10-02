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
  property string previousScreenName: ""

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
  // Called whenever window moves, finds which screen has the top-left corner
  //
  function updateScreenName() {
    for (let i = 0; i < Cpp_ScreenList.length; ++i) {
      let g = Cpp_ScreenList[i].geometry
      if (root.x >= g.x && root.x < g.x + g.width &&
          root.y >= g.y && root.y < g.y + g.height) {
        if (root.previousScreenName !== Cpp_ScreenList[i].name)
          root.previousScreenName = Cpp_ScreenList[i].name

        return
      }
    }

    root.previousScreenName = Cpp_PrimaryScreen.name
  }

  //
  // Find screen information based on screen name
  //
  function findScreenByName(screenName) {
    for (let i = 0; i < Cpp_ScreenList.length; ++i) {
      if (Cpp_ScreenList[i].name === screenName)
        return Cpp_ScreenList[i]
    }

    return Cpp_PrimaryScreen
  }

  //
  // Ensure that window is visible
  //
  function displayWindow() {
    if (root.isMaximized) {
      root.showMaximized()
    } else {
      let targetScreen = findScreenByName(root.previousScreenName)
      let g = targetScreen.geometry

      if (root.x > g.x + g.width - root.minimumWidth || root.x < g.x)
        root.x = g.x + (g.width - root.minimumWidth) / 2
      if (root.y > g.y + g.height - root.minimumHeight || root.y < g.y)
        root.y = g.y + (g.height - root.minimumHeight) / 2
      if (root.width > g.width)
        root.width = root.minimumWidth
      if (root.height > g.height)
        root.height = root.minimumHeight

      root.showNormal()
    }

    root.raise()
    root.requestActivate()
  }

  //
  // Ensure that window size stays within minimum size
  //
  Component.onCompleted: {
    // Try to restore to the previous screen
    let targetScreen = findScreenByName(root.previousScreenName)
    let g = targetScreen.geometry

    // Snap inside screen bounds if needed
    if (root.x < g.x || root.x > g.x + g.width - root.width)
      root.x = g.x + (g.width - root.width) / 2
    if (root.y < g.y || root.y > g.y + g.height - root.height)
      root.y = g.y + (g.height - root.height) / 2

    if (root.width < root.minimumWidth)
      root.width = root.minimumWidth
    if (root.height < root.minimumHeight)
      root.height = root.minimumHeight

    // Make sure window is fully visible
    if (root.x < g.x)
      root.x = g.x
    if (root.y < g.y)
      root.y = g.y
  }

  //
  // Save previous values for maximize/unmaximize cycle with delay
  //
  x: 100
  y: 100
  onWidthChanged: savePreviousDimensions()
  onHeightChanged: savePreviousDimensions()
  onXChanged: {
    savePreviousDimensions()
    updateScreenName()
  }
  onYChanged: {
    savePreviousDimensions()
    updateScreenName()
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
    property alias pn: root.previousScreenName
  }
}
