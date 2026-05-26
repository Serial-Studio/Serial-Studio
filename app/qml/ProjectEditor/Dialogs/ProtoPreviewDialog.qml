/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

import QtCore
import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets

Window {
  id: root

  Widgets.WindowMirror {}

  property int titlebarHeight: 0

  width: 720
  height: 540
  minimumWidth: 620
  minimumHeight: 440 + titlebarHeight
  title: qsTr("Protocol Buffers File Preview")

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
  // Theme tracking
  //
  Connections {
    target: Cpp_ThemeManager
    function onThemeChanged() {
      if (root.visible)
        Cpp_NativeWindow.addWindow(root, Cpp_ThemeManager.colors["window"])
    }
  }

  //
  // Top titlebar fill
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
  // Drag/move window via toolbar
  //
  DragHandler {
    target: null
    onActiveChanged: {
      if (active)
        root.startSystemMove()
    }
  }

  Connections {
    target: Cpp_JSON_ProtoImporter
    function onPreviewReady() {
      _picker.currentIndex = 0
      root.show()
    }
  }

  Shortcut {
    sequences: [StandardKey.Close]
    onActivated: {
      Cpp_JSON_ProtoImporter.cancelImport()
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
      id: column

      spacing: 4
      anchors.margins: 16
      anchors.fill: parent

      Label {
        color: palette.text
        Layout.fillWidth: true
        font: Cpp_Misc_CommonFonts.customUiFont(1.1, true)
        text: qsTr("Proto File: %1").arg(Cpp_JSON_ProtoImporter.protoFileName)
      }

      Label {
        opacity: 0.7
        color: palette.text
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        text: qsTr("Browse the messages below, then create the project. Every message " +
                   "becomes a dashboard group; matching-type channel blocks get a MultiPlot " +
                   "and mixed-type messages get a DataGrid.")
      }

      Item {
        implicitHeight: 4
      }

      Label {
        text: qsTr("Show fields for")
        color: Cpp_ThemeManager.colors["pane_section_label"]
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
        Component.onCompleted: font.capitalization = Font.AllUppercase
      }

      Widgets.Combo {
        id: _picker

        Layout.fillWidth: true
        model: Cpp_JSON_ProtoImporter.messageCount

        delegate: ItemDelegate {
          width: _picker.width
          text: Cpp_JSON_ProtoImporter.messageInfo(index)
          highlighted: _picker.highlightedIndex === index
        }

        displayText: Cpp_JSON_ProtoImporter.messageInfo(currentIndex)
      }

      Item {
        implicitHeight: 4
      }

      Label {
        text: qsTr("Fields")
        color: Cpp_ThemeManager.colors["pane_section_label"]
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
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
                text: qsTr("Tag")
                color: palette.text
                Layout.preferredWidth: 60
                font: Cpp_Misc_CommonFonts.boldUiFont
              }

              Label {
                color: palette.text
                Layout.fillWidth: true
                Layout.minimumWidth: 150
                text: qsTr("Field Name")
                font: Cpp_Misc_CommonFonts.boldUiFont
              }

              Label {
                text: qsTr("Type")
                color: palette.text
                Layout.preferredWidth: 200
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
            id: _fieldsTable

            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: {
              // ComboBox currentIndex change triggers a rebind via the messagesChanged signal.
              if (Cpp_JSON_ProtoImporter.messageCount <= 0)
                return 0

              // Probe field count by scanning until fieldInfo returns empty.
              let n = 0
              while (Cpp_JSON_ProtoImporter.fieldInfo(_picker.currentIndex, n) !== "")
                n++

              return n
            }

            delegate: Rectangle {
              height: 32
              width: ListView.view.width
              color: index % 2 === 0 ? "transparent" : palette.alternateBase

              RowLayout {
                spacing: 8
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8

                Label {
                  color: palette.text
                  Layout.preferredWidth: 60
                  font: Cpp_Misc_CommonFonts.monoFont
                  text: {
                    const info = Cpp_JSON_ProtoImporter.fieldInfo(_picker.currentIndex, index)
                    const match = info.match(/^\[(\d+)\]/)
                    return match ? match[1] : ""
                  }
                }

                Label {
                  color: palette.text
                  Layout.fillWidth: true
                  elide: Text.ElideRight
                  Layout.minimumWidth: 150
                  text: {
                    const info = Cpp_JSON_ProtoImporter.fieldInfo(_picker.currentIndex, index)
                    const parts = info.split(" ")
                    return parts.length > 0 ? parts[parts.length - 1] : ""
                  }
                }

                Label {
                  color: palette.text
                  elide: Text.ElideRight
                  Layout.preferredWidth: 200
                  font: Cpp_Misc_CommonFonts.monoFont
                  text: {
                    const info = Cpp_JSON_ProtoImporter.fieldInfo(_picker.currentIndex, index)
                    const match = info.match(/^\[\d+\]\s+(.+)\s+\S+$/)
                    return match ? match[1] : ""
                  }
                }
              }
            }

            Label {
              opacity: 0.5
              color: palette.text
              anchors.centerIn: parent
              visible: _fieldsTable.count === 0
              horizontalAlignment: Text.AlignHCenter
              text: qsTr("No fields in the selected message.")
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
          text: qsTr("Total: %1 messages, %2 fields")
            .arg(Cpp_JSON_ProtoImporter.messageCount)
            .arg(Cpp_JSON_ProtoImporter.fieldCount)
        }

        Widgets.IconButton {
          horizontalPadding: 8
          text: qsTr("Cancel")
          icon.source: "qrc:/icons/buttons/close.svg"
          onClicked: {
            Cpp_JSON_ProtoImporter.cancelImport()
            root.close()
          }
        }

        Widgets.IconButton {
          highlighted: true
          horizontalPadding: 8
          text: qsTr("Create Project")
          icon.source: "qrc:/icons/buttons/apply.svg"
          enabled: Cpp_JSON_ProtoImporter.messageCount > 0
          onClicked: {
            Cpp_JSON_ProtoImporter.confirmImport()
            root.close()
          }
        }
      }
    }
  }
}
