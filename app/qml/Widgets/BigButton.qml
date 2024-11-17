/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
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
  // Checked background (toolbar)
  //
  Rectangle {
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
    brightness: _mouseArea.containsMouse && root.enabled ? 0.07 : 0

    Behavior on saturation {NumberAnimation{}}
    Behavior on brightness {NumberAnimation{}}
  }
}
