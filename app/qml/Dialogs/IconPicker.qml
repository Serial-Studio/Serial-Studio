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
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

Window {
  id: root

  //
  // Custom properties
  //
  signal iconSelected(var icon)
  property string selectedIcon: ""

  //
  // Window options
  //
  width: minimumWidth
  height: minimumHeight
  minimumWidth: 320 + 32
  maximumWidth: 320 + 32
  minimumHeight: 480 + 32
  maximumHeight: 480 + 32
  title: qsTr("Select Icon")
  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.WindowTitleHint |
        Qt.WindowStaysOnTopHint |
        Qt.WindowCloseButtonHint
  }

  //
  // Close shortcut
  //
  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
  }

  //
  // Use page item to set application palette
  //
  Page {
    anchors.fill: parent
    palette.mid: Cpp_ThemeManager.colors["mid"]
    palette.dark: Cpp_ThemeManager.colors["dark"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.link: Cpp_ThemeManager.colors["link"]
    palette.light: Cpp_ThemeManager.colors["light"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.shadow: Cpp_ThemeManager.colors["shadow"]
    palette.accent: Cpp_ThemeManager.colors["accent"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.midlight: Cpp_ThemeManager.colors["midlight"]
    palette.highlight: Cpp_ThemeManager.colors["highlight"]
    palette.windowText: Cpp_ThemeManager.colors["window_text"]
    palette.brightText: Cpp_ThemeManager.colors["bright_text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.toolTipBase: Cpp_ThemeManager.colors["tooltip_base"]
    palette.toolTipText: Cpp_ThemeManager.colors["tooltip_text"]
    palette.linkVisited: Cpp_ThemeManager.colors["link_visited"]
    palette.alternateBase: Cpp_ThemeManager.colors["alternate_base"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    //
    // Window controls
    //
    ColumnLayout {
      spacing: 4
      anchors.fill: parent
      anchors.margins: 16

      Rectangle {
        radius: 2
        border.width: 1
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
}
