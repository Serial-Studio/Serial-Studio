/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Window {
  id: root

  //
  // Custom properties
  //
  property int titlebarHeight: 0

  width: 750
  height: 500
  minimumWidth: 650
  minimumHeight: 400 + titlebarHeight
  title: qsTr("Modbus Register Map Preview")

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
    target: Cpp_JSON_ModbusMapImporter
    function onPreviewReady() {
      _registersTable.model = Cpp_JSON_ModbusMapImporter.registerCount
      root.show()
    }
  }

  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: {
      Cpp_JSON_ModbusMapImporter.cancelImport()
      root.close()
    }
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
      spacing: 4
      anchors.margins: 16
      anchors.fill: parent

      Label {
        color: palette.text
        Layout.fillWidth: true
        font: Cpp_Misc_CommonFonts.customUiFont(1.1, true)
        text: qsTr("File: %1").arg(Cpp_JSON_ModbusMapImporter.fileName)
      }

      Label {
        opacity: 0.7
        color: palette.text
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        text: qsTr("Review the registers to import into a new Serial Studio project.")
      }

      Item {
        implicitHeight: 4
      }

      Label {
        text: qsTr("Registers")
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
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
          color: Cpp_ThemeManager.colors["groupbox_background"]
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
                Layout.preferredWidth: 40
                font: Cpp_Misc_CommonFonts.boldUiFont
              }

              Label {
                text: qsTr("Name")
                color: palette.text
                Layout.fillWidth: true
                Layout.minimumWidth: 120
                font: Cpp_Misc_CommonFonts.boldUiFont
              }

              Label {
                color: palette.text
                text: qsTr("Address")
                Layout.preferredWidth: 60
                font: Cpp_Misc_CommonFonts.boldUiFont
              }

              Label {
                text: qsTr("Type")
                color: palette.text
                Layout.preferredWidth: 120
                font: Cpp_Misc_CommonFonts.boldUiFont
              }

              Label {
                color: palette.text
                text: qsTr("Data Type")
                Layout.preferredWidth: 70
                font: Cpp_Misc_CommonFonts.boldUiFont
              }

              Label {
                color: palette.text
                text: qsTr("Units")
                Layout.preferredWidth: 50
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
            id: _registersTable

            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: Cpp_JSON_ModbusMapImporter.registerCount

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
                  color: palette.text
                  Layout.preferredWidth: 40
                  text: (index + 1).toString()
                  font: Cpp_Misc_CommonFonts.monoFont
                }

                Label {
                  color: palette.text
                  Layout.fillWidth: true
                  elide: Text.ElideRight
                  Layout.minimumWidth: 120
                  text: {
                    var info = Cpp_JSON_ModbusMapImporter.registerInfo(index)
                    var match = info.match(/:\s*(.+?)\s*@/)
                    return match ? match[1].trim() : ""
                  }
                }

                Label {
                  color: palette.text
                  Layout.preferredWidth: 60
                  font: Cpp_Misc_CommonFonts.monoFont
                  text: {
                    var info = Cpp_JSON_ModbusMapImporter.registerInfo(index)
                    var match = info.match(/@\s*(\d+)/)
                    return match ? match[1] : ""
                  }
                }

                Label {
                  color: palette.text
                  Layout.preferredWidth: 120
                  text: {
                    var info = Cpp_JSON_ModbusMapImporter.registerInfo(index)
                    var match = info.match(/\(([^,]+),/)
                    return match ? match[1].trim() : ""
                  }
                }

                Label {
                  color: palette.text
                  Layout.preferredWidth: 70
                  font: Cpp_Misc_CommonFonts.monoFont
                  text: {
                    var info = Cpp_JSON_ModbusMapImporter.registerInfo(index)
                    var match = info.match(/,\s*([^)]+)\)/)
                    return match ? match[1].trim() : ""
                  }
                }

                Label {
                  color: palette.text
                  Layout.preferredWidth: 50
                  text: {
                    var info = Cpp_JSON_ModbusMapImporter.registerInfo(index)
                    var match = info.match(/\[([^\]]+)\]/)
                    return match ? match[1] : ""
                  }
                }
              }
            }

            Label {
              opacity: 0.5
              color: palette.text
              anchors.centerIn: parent
              visible: _registersTable.count === 0
              horizontalAlignment: Text.AlignHCenter
              text: qsTr("No registers found in file.")
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
          text: qsTr("Total: %1 registers in %2 groups")
            .arg(Cpp_JSON_ModbusMapImporter.registerCount)
            .arg(Cpp_JSON_ModbusMapImporter.groupCount)
        }

        Button {
          icon.width: 18
          icon.height: 18
          text: qsTr("Cancel")
          horizontalPadding: 8
          icon.color: Cpp_ThemeManager.colors["button_text"]
          icon.source: "qrc:/icons/buttons/close.svg"
          onClicked: {
            Cpp_JSON_ModbusMapImporter.cancelImport()
            root.close()
          }
        }

        Button {
          icon.width: 18
          icon.height: 18
          highlighted: true
          horizontalPadding: 8
          text: qsTr("Create Project")
          icon.color: Cpp_ThemeManager.colors["button_text"]
          icon.source: "qrc:/icons/buttons/apply.svg"
          enabled: Cpp_JSON_ModbusMapImporter.registerCount > 0
          onClicked: {
            Cpp_JSON_ModbusMapImporter.confirmImport()
            root.close()
          }
        }
      }
    }
  }
}
