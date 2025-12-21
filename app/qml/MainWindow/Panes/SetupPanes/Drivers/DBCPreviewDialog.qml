/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

Window {
  id: root
  width: 700
  height: 500
  minimumWidth: 600
  minimumHeight: 400
  title: qsTr("DBC File Preview")

  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.WindowTitleHint |
        Qt.WindowCloseButtonHint |
        Qt.WindowStaysOnTopHint
  }

  Connections {
    target: Cpp_JSON_DBCImporter
    function onPreviewReady() {
      _messagesTable.model = Cpp_JSON_DBCImporter.messageCount
      root.show()
    }
  }

  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: {
      Cpp_JSON_DBCImporter.cancelImport()
      root.close()
    }
  }

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

    ColumnLayout {
      id: column
      spacing: 4
      anchors.fill: parent
      anchors.margins: 16

      Label {
        text: qsTr("DBC File: %1").arg(Cpp_JSON_DBCImporter.dbcFileName)
        font: Cpp_Misc_CommonFonts.customUiFont(1.1, true)
        color: palette.text
        Layout.fillWidth: true
      }

      Label {
        text: qsTr("Review the CAN messages and signals that will be imported into a new Serial Studio project.")
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
        color: palette.text
        opacity: 0.7
      }

      Item {
        implicitHeight: 4
      }

      Label {
        text: qsTr("Messages")
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      }

      GroupBox {
        Layout.fillWidth: true
        Layout.fillHeight: true

        background: Rectangle {
          radius: 2
          border.width: 1
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        ColumnLayout {
          spacing: 0
          anchors.fill: parent
          anchors.margins: -7

          Rectangle {
            Layout.fillWidth: true
            height: 32
            color: palette.alternateBase

            RowLayout {
              anchors.fill: parent
              anchors.leftMargin: 8
              anchors.rightMargin: 8
              spacing: 8

              Label {
                text: qsTr("#")
                font.bold: true
                color: palette.text
                Layout.preferredWidth: 40
              }

              Label {
                text: qsTr("Message Name")
                font.bold: true
                color: palette.text
                Layout.fillWidth: true
                Layout.minimumWidth: 150
              }

              Label {
                text: qsTr("CAN ID")
                font.bold: true
                color: palette.text
                Layout.preferredWidth: 80
              }

              Label {
                text: qsTr("Signals")
                font.bold: true
                color: palette.text
                Layout.preferredWidth: 60
              }
            }
          }

          Rectangle {
            height: 1
            color: palette.mid
            Layout.fillWidth: true
          }

          ListView {
            id: _messagesTable

            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: Cpp_JSON_DBCImporter.messageCount

            delegate: Rectangle {
              width: ListView.view.width
              height: 36
              color: index % 2 === 0 ? "transparent" : palette.alternateBase

              RowLayout {
                spacing: 8
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8

                Label {
                  Layout.preferredWidth: 40
                  text: (index + 1).toString()
                  font.family: Cpp_Misc_CommonFonts.monoFont.family
                  color: palette.text
                }

                Label {
                  Layout.fillWidth: true
                  Layout.minimumWidth: 150
                  elide: Text.ElideRight
                  color: palette.text
                  text: {
                    const info = Cpp_JSON_DBCImporter.messageInfo(index)
                    const match = info.match(/:\s*([^@]+)\s*@/)
                    return match ? match[1].trim() : ""
                  }
                }

                Label {
                  Layout.preferredWidth: 80
                  font.family: Cpp_Misc_CommonFonts.monoFont.family
                  color: palette.text
                  text: {
                    const info = Cpp_JSON_DBCImporter.messageInfo(index)
                    const match = info.match(/@\s*(0x[0-9A-F]+)/)
                    return match ? match[1] : ""
                  }
                }

                Label {
                  Layout.preferredWidth: 60
                  font.family: Cpp_Misc_CommonFonts.monoFont.family
                  color: palette.text
                  text: {
                    const info = Cpp_JSON_DBCImporter.messageInfo(index)
                    const match = info.match(/\((\d+)\s+signals?\)/)
                    return match ? match[1] : ""
                  }
                }
              }
            }

            Label {
              opacity: 0.5
              color: palette.text
              anchors.centerIn: parent
              visible: _messagesTable.count === 0
              horizontalAlignment: Text.AlignHCenter
              text: qsTr("No messages found in DBC file.")
            }
          }
        }
      }

      Item {
        implicitHeight: 4
      }

      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Label {
          opacity: 0.7
          color: palette.text
          Layout.fillWidth: true
          text: qsTr("Total: %1 messages, %2 signals")
            .arg(Cpp_JSON_DBCImporter.messageCount)
            .arg(Cpp_JSON_DBCImporter.signalCount)
        }

        Button {
          text: qsTr("Cancel")
          onClicked: {
            Cpp_JSON_DBCImporter.cancelImport()
            root.close()
          }
        }

        Button {
          text: qsTr("Create Project")
          highlighted: true
          enabled: Cpp_JSON_DBCImporter.messageCount > 0
          onClicked: {
            Cpp_JSON_DBCImporter.confirmImport()
            root.close()
          }
        }
      }
    }
  }
}
