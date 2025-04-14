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

import "../../Widgets" as Widgets

Widgets.Pane {
  id: root
  implicitWidth: 0
  implicitHeight: 0
  icon: Cpp_JSON_ProjectModel.selectedIcon
  title: Cpp_JSON_ProjectModel.selectedText
  Component.onCompleted: Cpp_JSON_ProjectModel.buildProjectModel()

  TableDelegate {
    id: delegate
    anchors.fill: parent
    anchors.topMargin: -16
    anchors.leftMargin: -10
    anchors.rightMargin: -10
    anchors.bottomMargin: -9
    modelPointer: Cpp_JSON_ProjectModel.projectModel

    footerItem: ColumnLayout {
      spacing: 0

      Image {
        sourceSize: Qt.size(128, 128)
        Layout.alignment: Qt.AlignHCenter
        source: "qrc:/rcc/images/soldering-iron.svg"
      }

      Item {
        implicitHeight: 16
      }

      Label {
        Layout.alignment: Qt.AlignHCenter
        text: qsTr("Start Building Now!")
        horizontalAlignment: Label.AlignHCenter
        font: Cpp_Misc_CommonFonts.customUiFont(2, true)
      }

      Item {
        implicitHeight: 8
      }

      Label {
        opacity: 0.8
        Layout.alignment: Qt.AlignHCenter
        horizontalAlignment: Label.AlignHCenter
        Layout.maximumWidth: delegate.width * 0.9
        wrapMode: Label.WrapAtWordBoundaryOrAnywhere
        font: Cpp_Misc_CommonFonts.customUiFont(1.5, false)
        text: qsTr("Get started by adding a group with the toolbar buttons above.")
      }
    }
  }
}
