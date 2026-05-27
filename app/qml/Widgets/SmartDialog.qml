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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Window
import QtQuick.Controls

Window {
  id: root

  //
  // Custom properties
  //
  property int titlebarHeight: 0
  property int frameMargin: 0
  property int frameTopInset: 0
  property bool staysOnTop: false
  property bool fixedSize: true
  property bool nativeWindow: true
  property int contentPadding: 16
  property alias dialogContent: contentArea.contentItem
  property color titleColor: Cpp_ThemeManager.colors["text"]
  property color backgroundColor: Cpp_ThemeManager.colors["window"]

  //
  // Bare content size (derived dialogs alias their own layout)
  //
  property int preferredWidth: contentArea.implicitWidth
  property int preferredHeight: contentArea.implicitHeight

  //
  // Window sizing: internal padding + titlebar/CSD frame wrap the preferred size
  //
  width: preferredWidth + 2 * contentPadding + 2 * frameMargin
  minimumWidth: preferredWidth + 2 * contentPadding + 2 * frameMargin
  maximumWidth: fixedSize ? preferredWidth + 2 * contentPadding + 2 * frameMargin : 16777215
  height: preferredHeight + contentPadding * 1.50 + titlebarHeight + frameTopInset + frameMargin
  minimumHeight: preferredHeight + contentPadding * 1.50 + titlebarHeight + frameTopInset + frameMargin
  maximumHeight: fixedSize ? preferredHeight + contentPadding * 1.50 + titlebarHeight + frameTopInset + frameMargin : 16777215

  //
  // Configure window flags and seed CSD chrome insets before first show
  //
  Component.onCompleted: {
    var baseType = root.fixedSize ? Qt.Dialog : Qt.Window
    var sizeButtons = root.fixedSize ? 0 : Qt.WindowMinMaxButtonsHint
    var staysOnTopFlag = root.staysOnTop ? Qt.WindowStaysOnTopHint : 0
    root.flags = baseType
        | Qt.CustomizeWindowHint
        | Qt.WindowTitleHint
        | Qt.WindowCloseButtonHint
        | sizeButtons
        | staysOnTopFlag

    if (root.nativeWindow) {
      root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
      root.frameMargin = Cpp_NativeWindow.frameMargin(root)
      root.frameTopInset = Cpp_NativeWindow.frameTopInset(root)
    }
  }

  //
  // Native window integration
  //
  onVisibilityChanged: {
    if (!root.nativeWindow)
      return

    if (visible)
      Cpp_NativeWindow.addWindow(root, root.backgroundColor)
    else
      Cpp_NativeWindow.removeWindow(root)

    root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
    root.frameMargin = Cpp_NativeWindow.frameMargin(root)
    root.frameTopInset = Cpp_NativeWindow.frameTopInset(root)
  }

  //
  // Update window colors when theme changes
  //
  Connections {
    target: Cpp_ThemeManager
    function onThemeChanged() {
      if (!root.nativeWindow)
        return

      if (root.visible)
        Cpp_NativeWindow.addWindow(root, root.backgroundColor)
    }
  }

  //
  // Mirror entire scene graph when active language is right-to-left
  //
  WindowMirror {}

  //
  // Enable window dragging via titlebar
  //
  Item {
    height: root.titlebarHeight
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }

    DragHandler {
      target: null
      onActiveChanged: {
        if (active)
          root.startSystemMove()
      }
    }
  }

  //
  // Close shortcut
  //
  Shortcut {
    onActivated: root.close()
    sequences: [StandardKey.Close]
  }

  //
  // Background
  //
  Rectangle {
    anchors.fill: parent
    color: root.backgroundColor
  }

  //
  // Titlebar text
  //
  Label {
    text: root.title
    anchors.topMargin: 6
    color: root.titleColor
    anchors.top: parent.top
    visible: root.titlebarHeight > 0
    anchors.horizontalCenter: parent.horizontalCenter
    font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)
  }

  //
  // Page container with theme palette
  //
  Page {
    id: contentArea

    padding: 0
    anchors.fill: parent
    anchors.margins: root.contentPadding
    anchors.topMargin: (root.contentPadding / 2) + root.titlebarHeight

    palette.window: root.backgroundColor
    palette.mid: Cpp_ThemeManager.colors["mid"]
    palette.dark: Cpp_ThemeManager.colors["dark"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.link: Cpp_ThemeManager.colors["link"]
    palette.light: Cpp_ThemeManager.colors["light"]
    palette.shadow: Cpp_ThemeManager.colors["shadow"]
    palette.accent: Cpp_ThemeManager.colors["accent"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.midlight: Cpp_ThemeManager.colors["midlight"]
    palette.highlight: Cpp_ThemeManager.colors["highlight"]
    palette.windowText: Cpp_ThemeManager.colors["window_text"]
    palette.brightText: Cpp_ThemeManager.colors["bright_text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.toolTipBase: Cpp_ThemeManager.colors["tooltip_base"]
    palette.toolTipText: Cpp_ThemeManager.colors["tooltip_text"]
    palette.linkVisited: Cpp_ThemeManager.colors["link_visited"]
    palette.alternateBase: Cpp_ThemeManager.colors["alternate_base"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]
  }
}
