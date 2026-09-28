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

  fixedSize: false
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight
  title: qsTr("Notification Channel Sounds")

  readonly property int colSpacing: 8
  readonly property int colWFile: 180
  readonly property int colWDelete: 28
  readonly property int colWChannel: 140
  readonly property int colScrollSlot: 12

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
      placeholderText: qsTr("(app default)")
      onEditingFinished: rowsModel.setProperty(cell.rowIndex, cell.key, text)
    }

    Widgets.IconButton {
      padding: 2
      iconSize: 14
      Layout.preferredWidth: 24
      Layout.preferredHeight: 24
      icon.source: "qrc:/icons/buttons/open.svg"
      onClicked: {
        root.browsingRow = cell.rowIndex
        root.browsingKey = cell.key
        _browse.open()
      }
    }
  }

  ColumnLayout {
    id: layout

    spacing: 8
    anchors.margins: 12
    anchors.fill: parent

    Label {
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("A Critical notification on a channel plays its Warning file, a Warning "
                 + "notification its Caution file, an Info notification its Advisory file. "
                 + "Empty cells use the sounds from Preferences.")
    }

    RowLayout {
      Layout.fillWidth: true
      spacing: root.colSpacing
      Layout.rightMargin: root.colScrollSlot

      Label {
        text: qsTr("Channel")
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["placeholder_text"]
        Layout.preferredWidth: root.colWChannel
      }
      Label {
        text: qsTr("Warning")
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["placeholder_text"]
        Layout.preferredWidth: root.colWFile
      }
      Label {
        text: qsTr("Caution")
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["placeholder_text"]
        Layout.preferredWidth: root.colWFile
      }
      Label {
        text: qsTr("Advisory")
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["placeholder_text"]
        Layout.preferredWidth: root.colWFile
      }
      Item {
        Layout.preferredWidth: root.colWDelete
      }
    }

    ListView {
      id: list

      clip: true
      model: rowsModel
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumHeight: 120
      boundsBehavior: Flickable.StopAtBounds
      Layout.preferredHeight: Math.max(120, contentHeight)

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
            placeholderText: qsTr("Channel name")
            Layout.preferredWidth: root.colWChannel
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
            Layout.preferredHeight: 24
            Layout.preferredWidth: root.colWDelete
            ToolTip.text: qsTr("Remove this channel.")
            icon.source: "qrc:/icons/buttons/trash.svg"
            onClicked: rowsModel.remove(row.index)
          }
        }
      }
    }

    RowLayout {
      spacing: 4
      Layout.fillWidth: true

      Widgets.IconButton {
        iconSize: 16
        text: qsTr("Add Channel")
        icon.source: "qrc:/icons/buttons/plus.svg"
        onClicked: root.addRow()
      }

      Item { Layout.fillWidth: true }

      Widgets.IconButton {
        iconSize: 16
        text: qsTr("Cancel")
        icon.source: "qrc:/icons/buttons/close.svg"
        onClicked: root.close()
      }

      Widgets.IconButton {
        iconSize: 16
        highlighted: true
        text: qsTr("Apply")
        icon.source: "qrc:/icons/buttons/apply.svg"
        onClicked: {
          Cpp_JSON_ProjectEditor.commitChannelSounds(root.collectRows())
          root.close()
        }
      }
    }
  }
}
