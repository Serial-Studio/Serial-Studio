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

import "../Widgets"

SmartDialog {
  id: root

  //
  // Custom properties
  //
  signal iconSelected(var icon)
  property string selectedIcon: ""

  //
  // Window options
  //
  staysOnTop: true
  title: qsTr("Select Icon")

  //
  // Window controls
  //
  contentItem: ColumnLayout {
    spacing: 4

    Rectangle {
      radius: 2
      border.width: 1
      implicitWidth: 320
      implicitHeight: 480
      Layout.fillWidth: true
      Layout.fillHeight: true
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: Cpp_ThemeManager.colors["groupbox_border"]

      GridView {
        clip: true
        cellWidth: 48
        cellHeight: 48
        anchors.margins: 4
        anchors.fill: parent
        model: Cpp_JSON_ProjectModel.availableActionIcons
        ScrollBar.vertical: ScrollBar {}

        delegate: Item {
          width: 48
          height: 48

          Rectangle {
            width: 42
            height: 42
            anchors.centerIn: parent
            color: root.selectedIcon === modelData ?
                     Cpp_ThemeManager.colors["highlight"] : "transparent"
          }

          Image {
            anchors.centerIn: parent
            sourceSize: Qt.size(32, 32)
            source: "qrc:/rcc/actions/" + modelData + ".svg"

            MouseArea {
              anchors.fill: parent
              onClicked: root.selectedIcon = modelData
              onDoubleClicked: {
                root.selectedIcon = modelData
                root.iconSelected(root.selectedIcon)
                root.close()
              }
            }
          }
        }
      }
    }

    Item {
      implicitHeight: 4
    }

    RowLayout {
      spacing: 4
      Layout.fillWidth: true

      Item {
        Layout.fillWidth: true
      }

      Button {
        text: qsTr("OK")
        highlighted: true
        onClicked: {
          root.iconSelected(root.selectedIcon)
          root.close()
        }
      }

      Button {
        text: qsTr("Cancel")
        onClicked: root.close()
      }
    }
  }
}
