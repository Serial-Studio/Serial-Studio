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

ColumnLayout {
  id: root
  spacing: 0
  visible: count > 0

  property int count: 0
  property var colors: [""]
  property var titles: [""]
  property string icon: ""
  property string title: ""

  signal checkedChanged(var index, var checked)

  Connections {
    target: Cpp_UI_Dashboard

    function onDataReset() {
      hideAll.checked = false
    }
  }

  RowLayout {
    spacing: 4
    visible: root.count > 0

    Image {
      source: root.icon
      sourceSize: Qt.size(18, 18)
      Layout.alignment: Qt.AlignVCenter
    }

    Label {
      text: root.title
      Layout.alignment: Qt.AlignVCenter
      opacity: hideAll.checked ? 0.5 : 1
      font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
      color: Cpp_ThemeManager.colors["pane_section_label"]
      Component.onCompleted: font.capitalization = Font.AllUppercase
    }

    Item {
      Layout.fillWidth: true
    }

    RoundButton {
      id: hideAll
      flat: true
      checkable: true
      icon.width: 18
      icon.height: 18
      implicitWidth: 24
      implicitHeight: 24
      Layout.rightMargin: -6
      icon.color: "transparent"
      icon.source: !checked ? "qrc:/rcc/icons/dashboard/show-all.svg" :
                              "qrc:/rcc/icons/dashboard/hide-all.svg"
      onCheckedChanged: {
        for (var i = 0; i < root.count; ++i)
          root.checkedChanged(i, !checked)
      }
    }
  }

  Repeater {
    model: hideAll.checked ? 0 : root.count
    delegate: Switch {
      checked: true
      Layout.leftMargin: -6
      Layout.fillWidth: true
      text: root.titles[index]
      Layout.maximumHeight: 24
      onCheckedChanged: root.checkedChanged(index, checked)
      palette.highlight: root.colors.length > index ? root.colors[index] : Cpp_ThemeManager.colors["switch_highlight"]
    }
  }

  Item {
    implicitHeight: 8
    visible: !hideAll.checked && count > 0
  }
}
