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
