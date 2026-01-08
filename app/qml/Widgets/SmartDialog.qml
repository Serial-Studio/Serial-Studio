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

import QtQuick
import QtQuick.Window
import QtQuick.Controls

Window {
  id: root

  //
  // Custom properties
  //
  property int titlebarHeight: 0
  property bool staysOnTop: false
  property int contentPadding: 16
  property alias contentItem: contentArea.contentItem
  property color titleColor: Cpp_ThemeManager.colors["text"]
  property color backgroundColor: Cpp_ThemeManager.colors["window"]

  //
  // Window sizing
  //
  width: minimumWidth
  height: minimumHeight
  minimumWidth: contentArea.implicitWidth + (contentPadding * 2)
  maximumWidth: contentArea.implicitWidth + (contentPadding * 2)
  minimumHeight: contentArea.implicitHeight + (contentPadding * 1.50) + titlebarHeight
  maximumHeight: contentArea.implicitHeight + (contentPadding * 1.50) + titlebarHeight

  //
  // CSD-safety properties
  //
  property int preferredWidth: contentArea.implicitWidth + (contentPadding * 2)
  property int preferredHeight: contentArea.implicitHeight + (contentPadding * 1.50) + titlebarHeight

  //
  // Configure window flags
  //
  Component.onCompleted: {
    if (root.staysOnTop) {
      root.flags = Qt.Dialog |
          Qt.CustomizeWindowHint |
          Qt.WindowTitleHint |
          Qt.WindowStaysOnTopHint |
          Qt.WindowCloseButtonHint
    } else {
      root.flags = Qt.Dialog |
          Qt.CustomizeWindowHint |
          Qt.WindowTitleHint |
          Qt.WindowCloseButtonHint
    }
  }

  //
  // Native window integration
  //
  onVisibilityChanged: {
    if (visible)
      Cpp_NativeWindow.addWindow(root, root.backgroundColor)
    else
      Cpp_NativeWindow.removeWindow(root)

    root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
  }

  //
  // Update window colors when theme changes
  //
  Connections {
    target: Cpp_ThemeManager
    function onThemeChanged() {
      Cpp_NativeWindow.removeWindow(root)
      if (root.visible)
        Cpp_NativeWindow.addWindow(root, root.backgroundColor)
    }
  }

  //
  // Enable window dragging via titlebar
  //
  DragHandler {
    target: null
    onActiveChanged: {
      if (active)
        root.startSystemMove()
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
