/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import "../../Widgets" as Widgets

//
// Channel sound map editor (spec 0087 R14): one row per channel, one WAV per priority.
//
Widgets.SmartDialog {
  id: root

  objectName: "channelSoundsEditor"

  fixedSize: false
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight
  title: qsTr("Notification Channel Sounds")

  readonly property int colSpacing: 8
  readonly property int colWFile: 180
  readonly property int colWDelete: 28
  readonly property int colWChannel: 140
  readonly property int colScrollSlot: 12
  readonly property int contentWidth: 770

  property int browsingRow: -1
  property string browsingKey: ""

  ListModel {
    id: rowsModel
  }

  //
  // Public API
  //
  function showDialog(rows) {
    rowsModel.clear()
    for (let i = 0; i < (rows ? rows.length : 0); ++i) {
      const r = rows[i]
      rowsModel.append({
        channel: String(r.channel ?? ""),
        warning: String(r.warning ?? ""),
        caution: String(r.caution ?? ""),
        advisory: String(r.advisory ?? "")
      })
    }

    root.show()
    root.raise()
    root.requestActivate()
  }

  function collectRows() {
    const out = []
    for (let i = 0; i < rowsModel.count; ++i) {
      const r = rowsModel.get(i)
      if (String(r.channel).trim().length === 0)
        continue

      out.push({
        channel: String(r.channel).trim(),
        warning: String(r.warning),
        caution: String(r.caution),
        advisory: String(r.advisory)
      })
    }
    return out
  }

  function addRow() {
    rowsModel.append({ channel: "", warning: "", caution: "", advisory: "" })
  }

  FileDialog {
    id: _browse

    title: qsTr("Select Sound File")
    fileMode: FileDialog.OpenFile
    nameFilters: [qsTr("WAV audio (*.wav)"), qsTr("All files (*)")]
    onAccepted: {
      if (root.browsingRow >= 0 && root.browsingRow < rowsModel.count)
        rowsModel.setProperty(root.browsingRow, root.browsingKey, String(selectedFile))
    }
  }

  //
  // One priority cell: path field plus browse and clear buttons
  //
  component SoundCell: RowLayout {
    id: cell

    required property int rowIndex
    required property string key
    required property string value

    spacing: 2
    Layout.preferredWidth: root.colWFile

    Widgets.LineField {
      id: pathField

      text: cell.value
      Layout.fillWidth: true
      font: Cpp_Misc_CommonFonts.uiFont
      objectName: "channelSoundsEditor.sound"
      placeholderText: qsTr("From Preferences")
      onEditingFinished: rowsModel.setProperty(cell.rowIndex, cell.key, text)
    }

    Widgets.IconButton {
      padding: 2
      iconSize: 14
      ToolTip.delay: 700
      ToolTip.visible: hovered
      Layout.preferredWidth: 24
      Layout.preferredHeight: 24
      objectName: "channelSoundsEditor.browse"
      ToolTip.text: qsTr("Choose a WAV file.")
      icon.source: "qrc:/icons/buttons/open.svg"
      onClicked: {
        root.browsingRow = cell.rowIndex
        root.browsingKey = cell.key
        _browse.open()
      }
    }
  }

  //--------------------------------------------------------------------------------------------
  // Layout: section label outside its content card, as in the Value Labels editor
  //--------------------------------------------------------------------------------------------

  dialogContent: ColumnLayout {
    id: layout

    spacing: 12
    anchors.centerIn: parent
    Layout.minimumWidth: root.contentWidth

    //
    // Introduction: what the dialog does, in one plain sentence and one example
    //
    Label {
      wrapMode: Text.WordWrap
      Layout.fillWidth: true
      Layout.maximumWidth: root.contentWidth
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Give a notification channel its own alert sounds. For example, notifications "
                 + "sent to the \"Engine\" channel can play a horn instead of the standard tone.")
    }

    //
    // The channel -> sounds table
    //
    ColumnLayout {
      spacing: 4
      Layout.fillWidth: true
      Layout.fillHeight: true

      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Label {
          text: qsTr("Channels")
          font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
          color: Cpp_ThemeManager.colors["pane_section_label"]
          Component.onCompleted: font.capitalization = Font.AllUppercase
        }

        Item {
          Layout.fillWidth: true
        }

        Widgets.IconButton {
          iconSize: 14
          ToolTip.delay: 700
          ToolTip.visible: hovered
          onClicked: root.addRow()
          text: qsTr("Add Channel")
          Layout.alignment: Qt.AlignVCenter
          objectName: "channelSoundsEditor.add"
          icon.source: "qrc:/icons/buttons/plus.svg"
          ToolTip.text: qsTr("Add a row for another channel.")
        }
      }

      Rectangle {
        radius: 2
        border.width: 1
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 200
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        ColumnLayout {
          spacing: 0
          anchors.margins: 8
          anchors.fill: parent

          //
          // Column headers: shared widths with the row delegate
          //
          RowLayout {
            Layout.fillWidth: true
            spacing: root.colSpacing
            Layout.rightMargin: root.colScrollSlot

            Label {
              text: qsTr("Channel")
              font: Cpp_Misc_CommonFonts.boldUiFont
              Layout.preferredWidth: root.colWChannel
              color: Cpp_ThemeManager.colors["placeholder_text"]
            }

            Label {
              text: qsTr("Warning")
              font: Cpp_Misc_CommonFonts.boldUiFont
              Layout.preferredWidth: root.colWFile
              color: Cpp_ThemeManager.colors["placeholder_text"]
            }

            Label {
              text: qsTr("Caution")
              font: Cpp_Misc_CommonFonts.boldUiFont
              Layout.preferredWidth: root.colWFile
              color: Cpp_ThemeManager.colors["placeholder_text"]
            }

            Label {
              text: qsTr("Advisory")
              font: Cpp_Misc_CommonFonts.boldUiFont
              Layout.preferredWidth: root.colWFile
              color: Cpp_ThemeManager.colors["placeholder_text"]
            }

            Item {
              Layout.preferredWidth: root.colWDelete
            }
          }

          Rectangle {
            implicitHeight: 1
            Layout.topMargin: 4
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          ListView {
            id: list

            spacing: 1
            clip: true
            model: rowsModel
            Layout.topMargin: 4
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 150
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
              policy: ScrollBar.AsNeeded
            }

            delegate: Item {
              id: row

              required property int index
              required property string channel
              required property string warning
              required property string caution
              required property string advisory

              implicitHeight: 36
              width: ListView.view.width

              RowLayout {
                anchors.fill: parent
                spacing: root.colSpacing
                anchors.rightMargin: root.colScrollSlot

                Widgets.LineField {
                  text: row.channel
                  font: Cpp_Misc_CommonFonts.uiFont
                  Layout.alignment: Qt.AlignVCenter
                  Layout.preferredWidth: root.colWChannel
                  objectName: "channelSoundsEditor.channel"
                  placeholderText: qsTr("For example, Engine")
                  onEditingFinished: rowsModel.setProperty(row.index, "channel", text)
                }

                SoundCell { rowIndex: row.index; key: "warning"; value: row.warning }
                SoundCell { rowIndex: row.index; key: "caution"; value: row.caution }
                SoundCell { rowIndex: row.index; key: "advisory"; value: row.advisory }

                Widgets.IconButton {
                  padding: 2
                  iconSize: 14
                  ToolTip.delay: 700
                  ToolTip.visible: hovered
                  Layout.alignment: Qt.AlignVCenter
                  onClicked: rowsModel.remove(row.index)
                  Layout.preferredWidth: root.colWDelete
                  objectName: "channelSoundsEditor.remove"
                  ToolTip.text: qsTr("Remove this channel.")
                  icon.source: "qrc:/icons/buttons/trash.svg"
                  icon.color: Cpp_ThemeManager.colors["text"]
                }
              }
            }

            //
            // Empty state: visible only while no channel is listed
            //
            Label {
              width: parent.width
              anchors.centerIn: parent
              wrapMode: Text.WordWrap
              visible: rowsModel.count === 0
              font: Cpp_Misc_CommonFonts.uiFont
              horizontalAlignment: Text.AlignHCenter
              color: Cpp_ThemeManager.colors["placeholder_text"]
              text: qsTr("No channels yet. Click Add Channel to give one its own sounds.")
            }
          }
        }
      }

      Label {
        opacity: 0.7
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        Layout.maximumWidth: root.contentWidth
        color: Cpp_ThemeManager.colors["text"]
        font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
        text: qsTr("A Critical notification plays the Warning sound, a Warning notification the "
                   + "Caution sound, and an Info notification the Advisory sound. A sound left "
                   + "empty uses the one from Preferences.")
      }
    }

    //
    // Footer: Cancel / Apply
    //
    RowLayout {
      spacing: 8
      Layout.topMargin: 4
      Layout.fillWidth: true

      Item {
        Layout.fillWidth: true
      }

      Widgets.IconButton {
        iconSize: 16
        ToolTip.delay: 700
        text: qsTr("Cancel")
        ToolTip.visible: hovered
        onClicked: root.close()
        ToolTip.text: qsTr("Discard changes.")
        objectName: "channelSoundsEditor.cancel"
        icon.source: "qrc:/icons/buttons/close.svg"
      }

      Widgets.IconButton {
        iconSize: 16
        highlighted: true
        ToolTip.delay: 700
        text: qsTr("Apply")
        ToolTip.visible: hovered
        objectName: "channelSoundsEditor.apply"
        icon.source: "qrc:/icons/buttons/apply.svg"
        ToolTip.text: qsTr("Save the channel sounds to this project.")
        onClicked: {
          forceActiveFocus()
          Cpp_JSON_ProjectEditor.commitChannelSounds(root.collectRows())
          root.close()
        }
      }
    }
  }
}
