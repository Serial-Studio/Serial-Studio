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
  width: 600
  height: 500
  minimumWidth: 500
  minimumHeight: 400
  title: qsTr("Modbus Register Groups")

  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.WindowTitleHint |
        Qt.WindowCloseButtonHint |
        Qt.WindowStaysOnTopHint
  }

  Connections {
    target: Cpp_IO_Modbus
    function onRegisterGroupsChanged() {
      _groupsTable.model = Cpp_IO_Modbus.registerGroupCount
    }
  }

  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: root.close()
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
        text: qsTr("Configure multiple register groups to poll different register types in sequence.")
        wrapMode: Text.WordWrap
        Layout.fillWidth: true
        color: palette.text
        opacity: 0.7
      }

      Item {
        implicitHeight: 4
      }

      Label {
        text: qsTr("Add New Group")
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      }

      GroupBox {
        Layout.fillWidth: true

        background: Rectangle {
          radius: 2
          border.width: 1
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        GridLayout {
          columns: 2
          rowSpacing: 4
          columnSpacing: 8
          anchors.fill: parent

          Label {
            text: qsTr("Register Type:")
            color: palette.text
          }

          ComboBox {
            id: _typeCombo
            Layout.fillWidth: true
            model: Cpp_IO_Modbus.registerTypeList
          }

          Label {
            text: qsTr("Start Address:")
            color: palette.text
          }

          TextField {
            id: _startField
            Layout.fillWidth: true
            placeholderText: qsTr("0-65535")
            validator: IntValidator { bottom: 0; top: 65535 }
          }

          Label {
            text: qsTr("Register Count:")
            color: palette.text
          }

          RowLayout {
            spacing: 8
            Layout.fillWidth: true

            TextField {
              id: _countField
              Layout.fillWidth: true
              placeholderText: qsTr("1-125")
              validator: IntValidator { bottom: 1; top: 125 }
            }

            Button {
              text: qsTr("Add Group")
              enabled: _startField.text.length > 0 && _countField.text.length > 0
              onClicked: {
                const type = _typeCombo.currentIndex
                const start = parseInt(_startField.text)
                const count = parseInt(_countField.text)

                if (!isNaN(start) && !isNaN(count) && count > 0 && count <= 125) {
                  Cpp_IO_Modbus.addRegisterGroup(type, start, count)
                  _startField.text = ""
                  _countField.text = ""
                }
              }
            }
          }
        }
      }

      Item {
        implicitHeight: 4
      }

      Label {
        text: qsTr("Configured Groups")
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
                Layout.preferredWidth: 30
              }

              Label {
                text: qsTr("Type")
                font.bold: true
                color: palette.text
                Layout.preferredWidth: 180
              }

              Label {
                text: qsTr("Start")
                font.bold: true
                color: palette.text
                Layout.preferredWidth: 60
              }

              Label {
                text: qsTr("Count")
                font.bold: true
                color: palette.text
                Layout.preferredWidth: 60
              }

              Label {
                text: qsTr("Action")
                font.bold: true
                color: palette.text
                Layout.fillWidth: true
              }
            }
          }

          Rectangle {
            height: 1
            color: palette.mid
            Layout.fillWidth: true
          }

          ListView {
            id: _groupsTable

            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: Cpp_IO_Modbus.registerGroupCount

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
                  Layout.preferredWidth: 30
                  text: (index + 1).toString()
                  font.family: Cpp_Misc_CommonFonts.monoFont.family
                }

                Label {
                  Layout.preferredWidth: 180
                  elide: Text.ElideRight
                  text: {
                    const types = Cpp_IO_Modbus.registerTypeList
                    const info = Cpp_IO_Modbus.registerGroupInfo(index)
                    const match = info.match(/^[^:]+:\s*([^@]+)/)
                    return match ? match[1].trim() : ""
                  }
                }

                Label {
                  Layout.preferredWidth: 60
                  font.family: Cpp_Misc_CommonFonts.monoFont.family
                  text: {
                    const info = Cpp_IO_Modbus.registerGroupInfo(index)
                    const match = info.match(/@\s*(\d+)/)
                    return match ? match[1] : ""
                  }
                }

                Label {
                  Layout.preferredWidth: 60
                  font.family: Cpp_Misc_CommonFonts.monoFont.family
                  text: {
                    const info = Cpp_IO_Modbus.registerGroupInfo(index)
                    const match = info.match(/count:\s*(\d+)/)
                    return match ? match[1] : ""
                  }
                }

                Item {
                  Layout.fillWidth: true
                }

                Button {
                  text: qsTr("Remove")
                  implicitHeight: 28
                  onClicked: Cpp_IO_Modbus.removeRegisterGroup(index)
                }
              }
            }

            Label {
              opacity: 0.5
              color: palette.text
              anchors.centerIn: parent
              visible: _groupsTable.count === 0
              horizontalAlignment: Text.AlignHCenter
              text: qsTr("No groups configured.\nAdd groups above to poll multiple register types.")
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
          text: qsTr("Total groups: %1").arg(Cpp_IO_Modbus.registerGroupCount)
        }

        Button {
          text: qsTr("Clear All")
          visible: Cpp_IO_Modbus.registerGroupCount > 0
          onClicked: Cpp_IO_Modbus.clearRegisterGroups()
        }

        Button {
          text: qsTr("Close")
          onClicked: root.close()
        }
      }
    }
  }
}
