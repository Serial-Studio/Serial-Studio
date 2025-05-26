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

  //
  // Opacity control properties
  //
  property real hoverOpacity: 0.5
  property real focusedOpacity: 1.0

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
  opacity: enabled ? 1 : 0.5
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
      PropertyChanges { target: _label; opacity: 0.3 }
      PropertyChanges { target: _background; border.color: "transparent" }
    },
    State {
      name: "minimized"
      when: root.open && root.minimized
      PropertyChanges { target: _label; opacity: 0.5 }
      PropertyChanges { target: _background; border.color: "transparent" }
    },
    State {
      name: "focused"
      when: root.open && !root.minimized && root.focused
      PropertyChanges { target: _label; font: Cpp_Misc_CommonFonts.boldUiFont }
      PropertyChanges { target: _label; color: Cpp_ThemeManager.colors["taskbar_indicator_active"] }
      PropertyChanges { target: _background; border.color: Cpp_ThemeManager.colors["taskbar_checked_button_border"] }
    },
    State {
      name: "idle"
      when: root.open && !root.minimized && !root.focused
      PropertyChanges { target: _label; font: Cpp_Misc_CommonFonts.uiFont }
      PropertyChanges { target: _label; color: Cpp_ThemeManager.colors["taskbar_text"] }
      PropertyChanges { target: _background; border.color: "transparent" }
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
    visible: false
    border.width: 1
    anchors.fill: parent
    border.color: Cpp_ThemeManager.colors["taskbar_checked_button_border"]

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

    property real baseVisibility: root.startMenu ? 0 : (root.enabled ? 1 : 0.5)
    property real hoverStateOpacity: root.focused ? root.focusedOpacity : (_mouseArea.containsMouse ? root.hoverOpacity : 0)

    opacity: baseVisibility * hoverStateOpacity
    saturation: root.enabled && _mouseArea.containsMouse ? 0.07 : 0
    brightness: root.enabled && _mouseArea.containsMouse ? (_mouseArea.containsPress ? -0.07 : 0.07) : 0

    Behavior on opacity { NumberAnimation{} }
    Behavior on saturation { NumberAnimation{} }
    Behavior on brightness { NumberAnimation{} }
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
        visible: false
        anchors.fill: parent
        sourceSize.width: root.iconSize
        sourceSize.height: root.iconSize
      }

      MultiEffect {
        id: _effects
        source: _icon
        anchors.fill: _icon
        opacity: root.open ? (root.minimized ? 0.5 : 1) : 0.3
        saturation: !root.open ? -1 : (_mouseArea.containsMouse && root.enabled ? 0.07 : 0)
        brightness: _mouseArea.containsMouse && root.enabled ? (_mouseArea.containsPress ? -0.07 : 0.07) : 0

        Behavior on opacity { NumberAnimation{} }
        Behavior on saturation { NumberAnimation{} }
        Behavior on brightness { NumberAnimation{} }
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
