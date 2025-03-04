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
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Dashboard" as DashboardItems

RowLayout {
  id: root
  spacing: 0

  //
  // Custom properties
  //
  property int structureMargin: 0
  readonly property int structureWidth: 280
  readonly property bool structureVisible: structureMargin > -1 * structureWidth

  //
  // Hides/shows the structure tree
  //
  function toggleStructureTree() {
    if (structureMargin < 0)
      structureMargin = 0
    else
      structureMargin = -1 * root.structureWidth
  }

  //
  // Animations
  //
  Behavior on structureMargin {NumberAnimation{}}

  //
  // View options window
  //
  DashboardItems.ViewOptions {
    id: viewOptions
    Layout.fillHeight: true
    Layout.minimumWidth: 280
    visible: root.structureVisible
    Layout.leftMargin: root.structureMargin
  }

  //
  // Panel border rectangle
  //
  Rectangle {
    z: 2
    implicitWidth: 1
    Layout.fillHeight: true
    visible: root.structureVisible
    color: Cpp_ThemeManager.colors["setup_border"]

    Rectangle {
      width: 1
      height: 32
      anchors.top: parent.top
      anchors.left: parent.left
      color: Cpp_ThemeManager.colors["pane_caption_border"]
    }
  }

  //
  // Widget grid
  //
  DashboardItems.WidgetGrid {
    id: widgetGrid
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.minimumWidth: 240
  }
}
