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

  //
  // Custom properties
  //
  property int titlebarHeight: 0

  width: 600
  height: 500
  minimumWidth: 500
  minimumHeight: 400 + titlebarHeight
  title: qsTr("Modbus Register Groups")

  Component.onCompleted: {
    root.flags = Qt.Dialog |
        Qt.CustomizeWindowHint |
        Qt.WindowTitleHint |
        Qt.WindowCloseButtonHint
  }

  //
  // Native window integration
  //
  onVisibleChanged: {
    if (visible)
      Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
    else
      Cpp_NativeWindow.removeWindow(root)

    root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(root)
  }

  //
  // Update window colors when theme changes
  //
  Connections {
    target: Cpp_ThemeManager
    function onThemeChanged() {
      if (root.visible)
        Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
    }
  }

  //
  // Top section
  //
  Rectangle {
    height: root.titlebarHeight
    color: Cpp_ThemeManager.colors["window"]
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
  }

  //
  // Titlebar text
  //
  Label {
    text: root.title
    visible: root.titlebarHeight > 0
    color: Cpp_ThemeManager.colors["text"]
    font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)

    anchors {
      topMargin: 6
      top: parent.top
      horizontalCenter: parent.horizontalCenter
    }
  }

  //
  // Be able to drag/move the window
  //
  DragHandler {
    target: null
    onActiveChanged: {
      if (active)
        root.startSystemMove()
    }
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
    anchors.topMargin: root.titlebarHeight
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
      anchors.margins: 16
      anchors.fill: parent

      Label {
        opacity: 0.7
        color: palette.text
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        text: qsTr("Configure multiple register groups to poll different register types in sequence.")
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
            color: palette.text
            text: qsTr("Register Type:")
          }

          ComboBox {
            id: _typeCombo

            Layout.fillWidth: true
            model: Cpp_IO_Modbus.registerTypeList
          }

          Label {
            color: palette.text
            text: qsTr("Start Address:")
          }

          TextField {
            id: _startField

            Layout.fillWidth: true
            placeholderText: qsTr("0-65535")
            validator: IntValidator { bottom: 0; top: 65535 }
          }

          Label {
            color: palette.text
            text: qsTr("Register Count:")
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
          anchors.margins: -7
          anchors.fill: parent

          Rectangle {
            height: 32
            Layout.fillWidth: true
            color: palette.alternateBase

            RowLayout {
              spacing: 8
              anchors.fill: parent
              anchors.leftMargin: 8
              anchors.rightMargin: 8

              Label {
                text: qsTr("#")
                color: palette.text
                Layout.preferredWidth: 30
                font: Cpp_Misc_CommonFonts.boldUiFont
              }

              Label {
                text: qsTr("Type")
                color: palette.text
                Layout.preferredWidth: 180
                font: Cpp_Misc_CommonFonts.boldUiFont
              }

              Label {
                color: palette.text
                text: qsTr("Start")
                Layout.preferredWidth: 60
                font: Cpp_Misc_CommonFonts.boldUiFont
              }

              Label {
                color: palette.text
                text: qsTr("Count")
                Layout.preferredWidth: 60
                font: Cpp_Misc_CommonFonts.boldUiFont
              }

              Label {
                color: palette.text
                text: qsTr("Action")
                Layout.fillWidth: true
                font: Cpp_Misc_CommonFonts.boldUiFont
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
              height: 36
              width: ListView.view.width
              color: index % 2 === 0 ? "transparent" : palette.alternateBase

              RowLayout {
                spacing: 8
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8

                Label {
                  Layout.preferredWidth: 30
                  text: (index + 1).toString()
                  font: Cpp_Misc_CommonFonts.monoFont
                }

                Label {
                  elide: Text.ElideRight
                  Layout.preferredWidth: 180
                  text: {
                    const types = Cpp_IO_Modbus.registerTypeList
                    const info = Cpp_IO_Modbus.registerGroupInfo(index)
                    const match = info.match(/^[^:]+:\s*([^@]+)/)
                    return match ? match[1].trim() : ""
                  }
                }

                Label {
                  Layout.preferredWidth: 60
                  font: Cpp_Misc_CommonFonts.monoFont
                  text: {
                    const info = Cpp_IO_Modbus.registerGroupInfo(index)
                    const match = info.match(/@\s*(\d+)/)
                    return match ? match[1] : ""
                  }
                }

                Label {
                  Layout.preferredWidth: 60
                  font: Cpp_Misc_CommonFonts.monoFont
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
                  implicitHeight: 28
                  text: qsTr("Remove")
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
          text: qsTr("Generate Project")
          visible: Cpp_IO_Modbus.registerGroupCount > 0
          onClicked: Cpp_IO_Modbus.generateProject()
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
