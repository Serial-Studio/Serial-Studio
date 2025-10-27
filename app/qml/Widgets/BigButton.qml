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
  property real iconSize: 32
  property alias font: _label.font
  property alias text: _label.text
  property bool toolbarButton: true
  property alias background: _background

  //
  // State properties
  //
  property bool checked: false
  property bool checkable: false

  //
  // Layout preferences
  //
  Layout.minimumWidth: implicitWidth
  Layout.maximumWidth: implicitWidth
  implicitHeight: iconSize + _label.implicitHeight + 20
  implicitWidth: Math.max(Math.ceil(metrics.width + 16), icon.width / 32 * 72)

  //
  // Animations
  //
  opacity: enabled ? 1 : 0.5
  Behavior on opacity {NumberAnimation{}}

  //
  // Enable tab focus
  //
  activeFocusOnTab: true

  //
  // Checked background (toolbar)
  //
  Rectangle {
    id: _background

    radius: 3
    border.width: 1
    anchors.fill: parent
    visible: root.toolbarButton
    color: Cpp_ThemeManager.colors["toolbar_checked_button_background"]
    border.color: Cpp_ThemeManager.colors["toolbar_checked_button_border"]
    opacity: (root.checked || _mouseArea.pressed) ? Cpp_ThemeManager.colors["toolbar_checked_button_opacity"] : 0.0

    Behavior on opacity {NumberAnimation{}}
  }

  //
  // Checked background (non-toolbar)
  //
  ToolButton {
    checked: true
    anchors.fill: parent
    visible: !root.toolbarButton
    opacity: (root.checked || _mouseArea.pressed) ? 1 : 0

    Behavior on opacity {NumberAnimation{}}
  }

  //
  // Button display
  //
  ColumnLayout {
    id: _layout
    visible: false

    spacing: 0
    anchors.fill: parent

    Item {
      Layout.fillHeight: true
    }

    Image {
      id: _icon
      sourceSize.width: root.iconSize
      sourceSize.height: root.iconSize
      Layout.alignment: Qt.AlignHCenter
    }

    Item {
      Layout.fillHeight: true
    }

    Label {
      id: _label
      elide: Qt.ElideRight
      Layout.maximumWidth: root.width
      Layout.alignment: Qt.AlignHCenter
      horizontalAlignment: Qt.AlignHCenter
      color: root.toolbarButton ? Cpp_ThemeManager.colors["toolbar_text"] :
                                  Cpp_ThemeManager.colors["button_text"]
    }

    Item {
      Layout.fillHeight: true
    }
  }

  //
  // Button width calculation
  //
  TextMetrics {
    id: metrics
    font: _label.font
    text: " " + root.text + " "
  }

  //
  // Mouse Area
  //
  MouseArea {
    id: _mouseArea
    hoverEnabled: true
    anchors.fill: parent
    onClicked: {
      if (root.checkable)
        root.checked = !root.checked

      root.clicked()
    }
  }

  //
  // Visual effects
  //
  MultiEffect {
    source: _layout
    anchors.fill: _layout
    saturation: _mouseArea.containsMouse && root.enabled ? 0.07 : 0
    brightness: _mouseArea.containsMouse && root.enabled ?
                  (_mouseArea.containsPress ? -0.07 : 0.07) : 0

    Behavior on saturation {NumberAnimation{}}
    Behavior on brightness {NumberAnimation{}}
  }
}
