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
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
  id: root

  property alias icon: _icon
  property alias label: _label
  property alias text: _label.text

  opacity: enabled ? 1 : 0.5
  implicitWidth: _layout.implicitWidth
  implicitHeight: _layout.implicitHeight

  property bool checked: false
  property bool checkable: false
  property bool expandable: true
  property alias containsMouse: _mouseArea.containsMouse

  signal clicked()

  Rectangle {
    anchors.fill: parent
    anchors.leftMargin: -6
    visible: _mouseArea.containsMouse
    color: Cpp_ThemeManager.colors["start_menu_highlight"]
  }

  RowLayout {
    id: _layout

    spacing: 0
    anchors.fill: parent

    Image {
      id: _icon
      sourceSize.width: 27
      sourceSize.height: 27
      Layout.alignment: Qt.AlignVCenter
    }

    Item {
      implicitWidth: 4
    }

    Label {
      id: _label
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignVCenter
      color: _mouseArea.containsMouse ? Cpp_ThemeManager.colors["start_menu_highlighted_text"] :
                                        Cpp_ThemeManager.colors["start_menu_text"]
    }

    Item {
      implicitWidth: 4
    }

    ToolButton {
      id: _expandButton

      icon.width: 16
      icon.height: 16
      background: Item{}
      Layout.alignment: Qt.AlignVCenter
      opacity: root.expandable || root.checked ? 1 : 0
      icon.source: root.checked ? "qrc:/rcc/icons/buttons/apply.svg" :
                                  "qrc:/rcc/icons/buttons/forward.svg"
      icon.color: _mouseArea.containsMouse ? Cpp_ThemeManager.colors["start_menu_highlighted_text"] :
                                             Cpp_ThemeManager.colors["start_menu_text"]
    }
  }

  MouseArea {
    id: _mouseArea
    hoverEnabled: true
    anchors.fill: parent
    onClicked: {
      root.clicked()
      if (root.checkable)
        root.checked = !root.checked
    }
  }
}
