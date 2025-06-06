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
  property bool horizontalLayout: false
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
  implicitHeight: horizontalLayout ?
                    Math.max(root.iconSize, _label.implicitHeight)  :
                    root.iconSize + _label.implicitHeight + 20
  implicitWidth: horizontalLayout ?
                   root.iconSize + Math.ceil(metrics.width + 16) :
                   Math.max(Math.ceil(metrics.width + 16), icon.width / 32 * 72)

  //
  // Animations
  //
  opacity: enabled ? 1 : 0.5
  Behavior on opacity { NumberAnimation {} }

  //
  // Checked background (toolbar)
  //
  Rectangle {
    id: _background

    radius: 3
    border.width: 1
    anchors.fill: parent
    visible: root.toolbarButton && !root.horizontalLayout
    color: Cpp_ThemeManager.colors["toolbar_checked_button_background"]
    border.color: Cpp_ThemeManager.colors["toolbar_checked_button_border"]
    opacity: (root.checked || _mouseArea.pressed) ? Cpp_ThemeManager.colors["toolbar_checked_button_opacity"] : 0.0

    Behavior on opacity { NumberAnimation {} }
  }

  //
  // Checked background (non-toolbar)
  //
  ToolButton {
    checked: true
    anchors.fill: parent
    visible: !root.toolbarButton
    opacity: (root.checked || _mouseArea.pressed) ? 1 : 0

    Behavior on opacity { NumberAnimation {} }
  }

  //
  // Button layout (Grid-based dynamic layout)
  //
  GridLayout {
    id: _layout
    anchors.fill: parent
    flow: GridLayout.LeftToRight
    rows: horizontalLayout ? 1 : 2
    columns: horizontalLayout ? 2 : 1
    rowSpacing: horizontalLayout ? 0 : 4
    columnSpacing: horizontalLayout ? 4 : 0
    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

    Item {
      id: _iconContainer
      Layout.row: 0
      Layout.column: 0
      implicitWidth: root.iconSize
      implicitHeight: root.iconSize
      Layout.alignment: horizontalLayout ? Qt.AlignLeft | Qt.AlignVCenter :
                                           Qt.AlignCenter

      Image {
        id: _icon
        anchors.fill: parent
        sourceSize.width: root.iconSize
        sourceSize.height: root.iconSize
        visible: Cpp_Misc_ModuleManager.softwareRendering
      }

      MultiEffect {
        source: _icon
        anchors.fill: _icon
        enabled: !Cpp_Misc_ModuleManager.softwareRendering
        visible: !Cpp_Misc_ModuleManager.softwareRendering
        saturation: _mouseArea.containsMouse && root.enabled ? 0.07 : 0
        brightness: _mouseArea.containsMouse && root.enabled ?
                      (_mouseArea.containsPress ? -0.07 : 0.07) : 0
      }
    }

    Label {
      id: _label
      elide: Qt.ElideRight
      Layout.row: horizontalLayout ? 0 : 1
      Layout.column: horizontalLayout ? 1 : 0
      Layout.alignment: horizontalLayout ? Qt.AlignLeft : Qt.AlignCenter
      horizontalAlignment: horizontalLayout ? Qt.AlignLeft : Qt.AlignHCenter
      color: root.toolbarButton ? Cpp_ThemeManager.colors["toolbar_text"] :
                                  Cpp_ThemeManager.colors["button_text"]
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
}
