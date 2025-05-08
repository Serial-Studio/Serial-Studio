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

import SerialStudio

Item {
  id: root

  //
  // Widget data inputs
  //
  property DataGridModel model
  property color color: "transparent"

  //
  // Responsive design stuff
  //
  readonly property bool unitsVisible: root.height >= 120

  //
  // Create scrollable grid view with LED states
  //
  Flickable {
    contentWidth: width
    anchors.fill: parent
    anchors.topMargin: 8
    anchors.leftMargin: 8
    anchors.bottomMargin: 8
    contentHeight: Math.max(grid.implicitHeight, height)

    ScrollBar.vertical: ScrollBar {
      id: scroll
    }

    GridLayout {
      id: grid
      columns: 2
      rowSpacing: 4
      columnSpacing: 4
      width: parent.width - 8
      anchors.centerIn: parent
      anchors.horizontalCenterOffset: -4

      Repeater {
        model: root.model.count
        delegate: Rectangle {
          border.width: 1
          Layout.fillWidth: true
          Layout.minimumHeight: 32
          Layout.maximumHeight: 32
          color: Cpp_ThemeManager.colors["widget_base"]
          border.color: Cpp_ThemeManager.colors["widget_border"]

          RowLayout {
            id: layout
            spacing: 0
            anchors.margins: 4
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            Label {
              elide: Qt.ElideRight
              Layout.fillWidth: true
              text: root.model.titles[index] + ":"
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
              horizontalAlignment: Label.AlignRight
              visible: text !== "" && root.unitsVisible
              color: root.model.alarms[index] ? Cpp_ThemeManager.colors["alarm"] :
                                                root.model.colors[index]

              Behavior on color {ColorAnimation{}}
            }

            Item {
              Layout.fillWidth: true
              Layout.minimumWidth: 4
            }

            Label {
              elide: Qt.ElideRight
              Layout.fillWidth: true
              text: root.model.values[index]
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
              Layout.maximumWidth: layout.width - 8
              horizontalAlignment: Label.AlignHCenter
              color: root.unitsVisible ? Cpp_ThemeManager.colors["widget_text"] :
                                         root.model.colors[index]
            }

            Item {
              Layout.fillWidth: true
              Layout.minimumWidth: 4
            }

            Label {
              elide: Qt.ElideRight
              Layout.fillWidth: true
              text: root.model.units[index]
              color: root.model.colors[index]
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.monoFont
              horizontalAlignment: Label.AlignLeft
              visible: text !== "" && root.unitsVisible
            }
          }
        }
      }
    }
  }
}
