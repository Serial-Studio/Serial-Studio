/*
 * Copyright (c) 2021-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
    columns: viewOptions.widgetColumns
  }
}
