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
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

Item {
  id: root

  //
  // Signals
  //
  signal clicked()

  //
  // Display properties
  //
  property alias icon: _icon
  property real iconSize: 18
  property bool startMenu: false
  property alias layout: _layout
  property alias font: _label.font
  property alias text: _label.text
  property alias background: _background

  //
  // State flags
  //
  property bool open: true
  property bool focused: false
  property bool minimized: false

  //
  // Control flags
  //
  property bool checkable: false
  property bool forceVisible: false

  //
  // Opacity control properties
  //
  property real hoverOpacity: 0.5
  property real focusedOpacity: 1.0

  //
  // Hide taskbar when not needed
  //
  visible: opacity > 0
  opacity: !open || minimized || startMenu || forceVisible ? 1 : 0

  //
  // Layout options
  //
  implicitHeight: startMenu ? 30 : 24
  implicitWidth: Math.min(128, Math.max(implicitHeight, _layout.implicitWidth))

  //
  // Tooltip
  //
  ToolTip.delay: 500
  ToolTip.visible: _mouseArea.containsMouse && ToolTip.text !== ""

  //
  // General Opacity
  //
  Behavior on opacity { NumberAnimation{} }

  //
  // Visual State Styling
  //
  states: [
    State {
      name: "startMenu"
      when: root.startMenu
      PropertyChanges { target: _label; font: Cpp_Misc_CommonFonts.boldUiFont }
      PropertyChanges { target: _label; color: Cpp_ThemeManager.colors["taskbar_indicator_active"] }
    },
    State {
      name: "closed"
      when: !root.open
      //PropertyChanges { target: _label; opacity: 0.3 }
    },
    State {
      name: "minimized"
      when: root.open && root.minimized
      //PropertyChanges { target: _label; opacity: 0.5 }
    },
    State {
      name: "focused"
      when: root.open && !root.minimized && root.focused
      //PropertyChanges { target: _label; font: Cpp_Misc_CommonFonts.boldUiFont }
      PropertyChanges { target: _label; color: Cpp_ThemeManager.colors["taskbar_indicator_active"] }
    },
    State {
      name: "idle"
      when: root.open && !root.minimized && !root.focused
      PropertyChanges { target: _label; font: Cpp_Misc_CommonFonts.uiFont }
      PropertyChanges { target: _label; color: Cpp_ThemeManager.colors["taskbar_text"] }
    }
  ]

  //
  // Animations
  //
  transitions: [
    Transition {
      NumberAnimation { properties: "opacity"; duration: 150 }
    }
  ]

  //
  // Background
  //
  Rectangle {
    id: _background
    anchors.fill: parent
    visible: Cpp_Misc_ModuleManager.softwareRendering
    border.width: root.focused || _mouseArea.containsMouse ? 1 : 0
    border.color: Cpp_ThemeManager.colors["taskbar_checked_button_border"]

    property real baseVisibility: root.startMenu ? 0 : (root.enabled ? 1 : 0.5)
    property real hoverStateOpacity: root.focused ? root.focusedOpacity : (_mouseArea.containsMouse ? root.hoverOpacity : 0)

    opacity: Cpp_Misc_ModuleManager.softwareRendering ? baseVisibility * hoverStateOpacity : 1

    gradient: Gradient {
      GradientStop {
        position: root.focused ? 0 : 1
        color: Cpp_ThemeManager.colors["taskbar_checked_button_top"]
      }
      GradientStop {
        position: root.focused ? 1 : 0
        color: Cpp_ThemeManager.colors["taskbar_checked_button_bottom"]
      }
    }
  }

  //
  // Background effects
  //
  MultiEffect {
    source: _background
    anchors.fill: _background

    enabled: !Cpp_Misc_ModuleManager.softwareRendering
    visible: !Cpp_Misc_ModuleManager.softwareRendering

    property real baseVisibility: root.startMenu ? 0 : (root.enabled ? 1 : 0.5)
    property real hoverStateOpacity: root.focused ? root.focusedOpacity : (_mouseArea.containsMouse ? root.hoverOpacity : 0)

    opacity: baseVisibility * hoverStateOpacity
    brightness: root.enabled && _mouseArea.containsMouse ? (_mouseArea.containsPress ? -0.07 : 0.07) : 0
  }

  //
  // Content Layout
  //
  RowLayout {
    id: _layout
    spacing: 0
    anchors.fill: parent
    visible: root.startMenu ? !Cpp_ThemeManager.colors["start_menu_button_gradient_enabled"] : true

    Item { implicitWidth: 4 }

    Item {
      implicitWidth: root.iconSize
      implicitHeight: root.iconSize
      Layout.alignment: Qt.AlignVCenter

      Image {
        id: _icon
        anchors.fill: parent
        sourceSize.width: root.iconSize
        sourceSize.height: root.iconSize
        visible: Cpp_Misc_ModuleManager.softwareRendering
      }

      MultiEffect {
        id: _effects
        source: _icon
        anchors.fill: _icon
        saturation: !root.open ? -1 : (_mouseArea.containsMouse && root.enabled ? 0.07 : 0)
        brightness: _mouseArea.containsMouse && root.enabled ? (_mouseArea.containsPress ? -0.07 : 0.07) : 0

        enabled: !Cpp_Misc_ModuleManager.softwareRendering
        visible: !Cpp_Misc_ModuleManager.softwareRendering
      }
    }

    Item { implicitWidth: 4 }

    Label {
      id: _label
      elide: Qt.ElideRight
      Layout.fillWidth: true
      font: Cpp_Misc_CommonFonts.uiFont
      Layout.alignment: Qt.AlignVCenter
      horizontalAlignment: Label.AlignLeft
      color: Cpp_ThemeManager.colors["taskbar_text"]
    }
  }

  //
  // Mouse Interaction
  //
  MouseArea {
    id: _mouseArea
    hoverEnabled: true
    anchors.fill: parent
    onClicked: {
      if (root.checkable)
        root.focused = !root.focused

      root.clicked()
    }
  }
}
