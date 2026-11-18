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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Window

Window {
  id: root

  //
  // Custom properties
  //
  property real previousX: 0
  property real previousY: 0
  property string category: ""
  property real previousWidth: 0
  property real previousHeight: 0
  property bool isMaximized: false
  property bool isChangingSize: false
  property string previousScreenName: ""
  property int preFullscreenVisibility: Window.AutomaticVisibility

  //
  // Toggle real OS fullscreen, remembering prior visibility for restore
  //
  function toggleFullScreen() {
    if (root.kioskFill)
      return

    if (root.visibility === Window.FullScreen) {
      if (root.preFullscreenVisibility === Window.Maximized)
        root.showMaximized()
      else
        root.showNormal()
    } else {
      root.preFullscreenVisibility = root.visibility
      root.showFullScreen()
    }
  }

  //
  // Kiosk display (--kiosk): a frameless window filling its screen. kioskGeometry is the whole
  // screen it took; kioskConfined (spec 0125) switches the main window's settings category.
  //
  property bool kioskFill: false
  property bool kioskConfined: false
  property rect kioskGeometry: Qt.rect(0, 0, 0, 0)
  readonly property bool kioskMode: typeof CLI_KIOSK !== "undefined" && CLI_KIOSK

  //
  // Geometry of the screen holding a point; by position, not by name, since two monitors of
  // the same model report the same name
  //
  function screenGeometryAt(px, py) {
    for (let i = 0; i < Cpp_ScreenList.length; ++i) {
      let g = Cpp_ScreenList[i].geometry
      if (px >= g.x && px < g.x + g.width && py >= g.y && py < g.y + g.height)
        return g
    }

    return Cpp_PrimaryScreen.geometry
  }

  //
  // Fills the screen under the saved position, or the session's screen when confined. False
  // outside kiosk mode, when `taken` fills that screen, or for a confined kiosk's other windows.
  //
  function showKiosk(taken) {
    if (!root.kioskMode)
      return false

    const confined = CLI_KIOSK_SESSION.confined
    if (confined && taken)
      return false

    const g = confined
            ? CLI_KIOSK_SESSION.screenGeometry
            : root.screenGeometryAt(root.x + root.width / 2, root.y + root.height / 2)
    if (taken && taken.width > 0 && taken.x === g.x && taken.y === g.y)
      return false

    root.kioskConfined = confined
    root.kioskFill = true
    root.kioskGeometry = Qt.rect(g.x, g.y, g.width, g.height)
    root.flags = Qt.Window | Qt.FramelessWindowHint
    root.applyKioskRect(g)
    root.showNormal()
    root.raise()
    root.requestActivate()
    return true
  }

  //
  // Takes the part of screen `g` a kiosk window may cover: all of it, minus the strip reserved
  // on the leftmost screen
  //
  function applyKioskRect(g) {
    const f = CLI_KIOSK_SESSION.fillRect(g)
    root.x = f.x
    root.y = f.y
    root.width = f.width
    root.height = f.height
  }

  //
  // Follows the session after a screen command; the settings category switches first, so the
  // new geometry never lands in the category an unconfined kiosk reads
  //
  function moveKiosk() {
    if (!root.kioskFill || !CLI_KIOSK_SESSION.confined)
      return

    const g = CLI_KIOSK_SESSION.screenGeometry
    root.kioskConfined = true
    root.kioskGeometry = Qt.rect(g.x, g.y, g.width, g.height)
    root.applyKioskRect(g)
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
  // Mirror entire scene graph when active language is right-to-left
  //
  WindowMirror {}

  //
  // HDR output stage (spec 0089); dormant unless HDR is enabled and granted. The two window
  // properties are the discovery surface HdrBoost reads through Window.window.
  //
  readonly property bool hdrActive: _hdr.active
  readonly property real hdrHeadroom: _hdr.headroom
  readonly property real hdrBoost: hdrActive
                                   ? Math.min(Cpp_Misc_GraphicsBackend.hdrAutoIntensity,
                                              hdrHeadroom)
                                   : 1

  HdrSurface {
    id: _hdr

    hostWindow: root
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
  onXChanged: {
    savePreviousDimensions()
    updateScreenName()
  }
  onYChanged: {
    savePreviousDimensions()
    updateScreenName()
  }
  onWidthChanged: savePreviousDimensions()
  onHeightChanged: savePreviousDimensions()

  //
  // React to maximize/unmaximize event
  //
  onVisibilityChanged: {
    if (root.visible) {
      if (root.visibility === Window.Maximized) {
        root.isMaximized = true
      } else if (root.isMaximized
                 && !root.kioskFill
                 && root.visibility !== Window.Minimized
                 && root.visibility !== Window.FullScreen) {
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
      if (!root.isMaximized && root.visible
          && !root.kioskFill
          && root.visibility !== Window.Maximized
          && root.visibility !== Window.FullScreen) {
        root.previousX = root.x
        root.previousY = root.y
        root.previousWidth = root.width
        root.previousHeight = root.height
      }

      root.isChangingSize = false
    }
  }

  //
  // Set by a CommandPalette hosted in this window. Window-local on purpose: a palette open in
  // another window must not disable this one's shortcuts.
  //
  property bool paletteOpen: false

  //
  // Close shortcut; yields to an open command palette in this window, whose own Cmd+W/Ctrl+W
  // handler would otherwise turn the sequence into an ambiguous no-op for both; off on a kiosk
  //
  Shortcut {
    enabled: !root.paletteOpen && !root.kioskFill
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  //
  // Save settings
  //
  Settings {
    property alias ax: root.x
    property alias ay: root.y
    property alias aw: root.width
    property alias ah: root.height
    property alias px: root.previousX
    property alias py: root.previousY
    property alias am: root.isMaximized
    property alias pw: root.previousWidth
    property alias ph: root.previousHeight
    property alias pn: root.previousScreenName
    category: root.category + (root.kioskConfined ? "_KioskConfined" : "") + app.settingsSuffix
  }
}
