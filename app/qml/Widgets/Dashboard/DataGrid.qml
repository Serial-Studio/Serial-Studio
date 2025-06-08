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
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../"

Item {
  id: root

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property DataGridWidget model

  //
  // Window flags
  //
  readonly property bool hasToolbar: root.height >= 220

  //
  // Custom properties
  //
  property bool running: true
  onRunningChanged: {
    if (model)
      model.paused = !root.running
  }

  //
  // Place widget in container when its initialized
  //
  onModelChanged: {
    if (model) {
      model.parent = innerContainer
      model.anchors.fill = innerContainer
      model.anchors.margins = -1
      model.anchors.rightMargin = -2
      model.visible = true
    }
  }

  //
  // Add toolbar
  //
  RowLayout {
    id: toolbar

    spacing: 4
    visible: root.hasToolbar
    height: root.hasToolbar ? 48 : 0

    anchors {
      leftMargin: 8
      top: parent.top
      left: parent.left
      right: parent.right
    }

    ToolButton {
      height: 24
      icon.width: 18
      icon.height: 18
      checked: !root.running
      icon.color: "transparent"
      text: root.running ? qsTr("Pause") : qsTr("Resume")
      onClicked: root.running = !root.running
      icon.source: root.running?
                     "qrc:/rcc/icons/dashboard-buttons/pause.svg" :
                     "qrc:/rcc/icons/dashboard-buttons/resume.svg"
    }

    Item {
      Layout.fillWidth: true
    }
  }

  //
  // Widget view
  //
  ScrollView {
    id: container
    clip: true
    contentHeight: innerContainer.height
    ScrollBar.vertical.policy: innerContainer.height > container.height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

    anchors {
      left: parent.left
      right: parent.right
      top: toolbar.bottom
      bottom: parent.bottom
    }

    Item {
      id: innerContainer
      width: container.width
      height: model ? model.contentHeight : 0
    }
  }
}
