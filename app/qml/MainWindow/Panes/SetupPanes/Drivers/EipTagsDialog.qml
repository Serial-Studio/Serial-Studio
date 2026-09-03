/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
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

import "../../../../Widgets" as Widgets

DriverTagPickerDialog {
  id: root

  width: 640
  height: 520
  minimumWidth: 540
  title: qsTr("EtherNet/IP Tags")
  minimumHeight: 420 + titlebarHeight

  Connections {
    target: Cpp_IO_Eip
    function onTagsChanged() {
      _tagsTable.model = Cpp_IO_Eip.tagCount
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
        text: qsTr("Name tags exactly as the controller does, for example MotorSpeed or Program:MainProgram.Counter. Leave the element index empty for a scalar tag.")
      }

      Item {
        implicitHeight: 4
      }

      Label {
        text: qsTr("Add New Tag")
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
            text: qsTr("Name:")
          }

          Widgets.LineField {
            id: _nameField

            Layout.fillWidth: true
            placeholderText: qsTr("Channel name")
          }

          Label {
            text: qsTr("Tag:")
            color: palette.text
          }

          Widgets.LineField {
            id: _tagField

            Layout.fillWidth: true
            placeholderText: qsTr("Controller tag name")
          }

          Label {
            color: palette.text
            text: qsTr("Type:")
          }

          RowLayout {
            spacing: 8
            Layout.fillWidth: true

            Widgets.Combo {
              id: _typeCombo

              currentIndex: 9
              Layout.fillWidth: true
              model: Cpp_IO_Eip.tagTypeList
            }

            Widgets.LineField {
              id: _elementField

              Layout.preferredWidth: 90
              placeholderText: qsTr("Element")
              validator: IntValidator { bottom: 0; top: 65535 }
            }

            Button {
              text: qsTr("Add Tag")
              enabled: _tagField.text.length > 0
              onClicked: {
                const element = _elementField.text.length > 0
                                ? parseInt(_elementField.text)
                                : -1
                Cpp_IO_Eip.addTag(_nameField.text,
                                  _tagField.text,
                                  Cpp_IO_Eip.tagTypeList[_typeCombo.currentIndex],
                                  isNaN(element) ? -1 : element)
                _nameField.text = ""
                _tagField.text = ""
                _elementField.text = ""
              }
            }
          }
        }
      }

      Item {
        implicitHeight: 4
      }

      Label {
        text: qsTr("Configured Tags")
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
                text: qsTr("Tag")
                color: palette.text
                Layout.fillWidth: true
                font: Cpp_Misc_CommonFonts.boldUiFont
              }

              Label {
                color: palette.text
                text: qsTr("Action")
                Layout.preferredWidth: 90
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
            id: _tagsTable

            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: Cpp_IO_Eip.tagCount

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
                  Layout.fillWidth: true
                  elide: Text.ElideRight
                  text: Cpp_IO_Eip.tagInfo(index)
                  font: Cpp_Misc_CommonFonts.monoFont
                }

                Button {
                  implicitHeight: 28
                  text: qsTr("Remove")
                  Layout.preferredWidth: 90
                  onClicked: Cpp_IO_Eip.removeTag(index)
                }
              }
            }

            Label {
              opacity: 0.5
              color: palette.text
              anchors.centerIn: parent
              visible: _tagsTable.count === 0
              horizontalAlignment: Text.AlignHCenter
              text: qsTr("No tags configured.\nAdd controller tags above to poll them.")
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
          text: qsTr("Total tags: %1").arg(Cpp_IO_Eip.tagCount)
        }

        Button {
          text: qsTr("Generate Project")
          visible: Cpp_IO_Eip.tagCount > 0
          onClicked: Cpp_IO_Eip.generateProject()
        }

        Button {
          text: qsTr("Clear All")
          visible: Cpp_IO_Eip.tagCount > 0
          onClicked: Cpp_IO_Eip.clearTags()
        }

        Button {
          text: qsTr("Close")
          onClicked: root.close()
        }
      }
    }
  }
}
