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
