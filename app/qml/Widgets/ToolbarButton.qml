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
  // Constants
  //
  property int maxButtonWidth: 128

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
  property bool checkBgVisible: true
  property bool horizontalLayout: false
  property alias background: _background

  //
  // State properties
  //
  property bool checked: false
  property bool checkable: false

  //
  // Layout properties
  //
  Layout.minimumWidth: implicitWidth
  Layout.maximumWidth: maxButtonWidth
  implicitHeight: horizontalLayout ?
                    Math.max(root.iconSize, _label.implicitHeight) :
                    root.iconSize + _label.implicitHeight + 20
  implicitWidth: Math.min(
                   maxButtonWidth,
                   horizontalLayout
                   ? root.iconSize + Math.ceil(_metrics.width + 16)
                   : Math.max(Math.ceil(_metrics.width + 16), icon.width / 32 * 72)
                   )

  //
  // Tooltip
  //
  ToolTip.delay: 700
  ToolTip.visible: _mouseArea.containsMouse && ToolTip.text !== ""

  //
  // Enabled/disabled opacity effect
  //
  opacity: enabled ? 1 : 0.5
  Behavior on opacity { NumberAnimation {} }

  //
  // Checked toolbar button background
  //
  Rectangle {
    id: _background
    radius: 3
    border.width: 1
    anchors.fill: parent
    visible: root.toolbarButton && !root.horizontalLayout
    color: Cpp_ThemeManager.colors["toolbar_checked_button_background"]
    border.color: Cpp_ThemeManager.colors["toolbar_checked_button_border"]
    opacity: (root.checked || _mouseArea.pressed)
             ? Cpp_ThemeManager.colors["toolbar_checked_button_opacity"]
             : 0.0
    Behavior on opacity { NumberAnimation {} }
  }

  //
  // Non-toolbar button background
  //
  ToolButton {
    checked: true
    anchors.fill: parent
    visible: !root.toolbarButton && root.checkBgVisible
    opacity: (root.checked || _mouseArea.pressed) ? 1 : 0
    Behavior on opacity { NumberAnimation {} }
  }

  //
  // Icon & text, add in scalable container for button press effect
  //
  Item {
    id: _scaleContainer
    anchors.fill: parent
    transform: Scale {
      id: _pressScale
      origin.x: _scaleContainer.width / 2
      origin.y: _scaleContainer.height / 2
      xScale: _mouseArea.pressed ? 0.95 : 1.0
      yScale: _mouseArea.pressed ? 0.95 : 1.0
    }
    Behavior on transform {
      NumberAnimation { properties: "xScale,yScale"; duration: 100; easing.type: Easing.OutQuad }
    }

    GridLayout {
      id: _layout
      anchors.fill: parent
      flow: GridLayout.LeftToRight
      rows: horizontalLayout ? 1 : 2
      columns: horizontalLayout ? 2 : 1
      rowSpacing: horizontalLayout ? 0 : 4
      columnSpacing: horizontalLayout ? 4 : 0
      anchors.leftMargin: horizontalLayout ? 8 : 0
      Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

      Item {
        id: _iconContainer
        Layout.row: 0
        Layout.column: 0
        implicitWidth: root.iconSize
        implicitHeight: root.iconSize
        Layout.alignment: horizontalLayout ? Qt.AlignLeft | Qt.AlignVCenter : Qt.AlignCenter

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
        Layout.fillWidth: horizontalLayout
        Layout.row: horizontalLayout ? 0 : 1
        Layout.column: horizontalLayout ? 1 : 0
        Layout.maximumWidth: root.maxButtonWidth - (horizontalLayout ? root.iconSize + 4 : 0)
        Layout.alignment: horizontalLayout ? Qt.AlignLeft : Qt.AlignCenter
        horizontalAlignment: horizontalLayout ? Qt.AlignLeft : Qt.AlignHCenter
        color: root.toolbarButton
               ? Cpp_ThemeManager.colors["toolbar_text"]
               : Cpp_ThemeManager.colors["button_text"]
      }
    }
  }

  //
  // Text metrics for calculating button width
  //
  TextMetrics {
    id: _metrics
    font: _label.font
    text: " " + root.text + " "
  }

  //
  // Mouse area for detecting clicks
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

