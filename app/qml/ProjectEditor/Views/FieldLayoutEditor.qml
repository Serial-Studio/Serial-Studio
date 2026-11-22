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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../../Widgets" as Widgets

Item {
  id: root

  objectName: "fieldLayoutEditor"

  //
  // The FrameParserModel bridge of the pane that hosts this editor
  //
  property var editor: null

  //
  // Decoded sample values, one text per field (empty when the sample is too short)
  //
  property var decoded: []

  //
  // Shared column widths, so the header row and the field rows line up
  //
  readonly property int colWMove: 24
  readonly property int colWIndex: 28
  readonly property int colWSmall: 56
  readonly property int colWType: 110
  readonly property int colSpacing: 6
  readonly property int maxFields: 256
  readonly property int colWOrder: 120
  readonly property int colWNumber: 80
  readonly property int colWValue: 110
  readonly property int colWDelete: 28
  readonly property int colScrollSlot: 12

  readonly property var typeLabels: [qsTr("Unsigned"), qsTr("Signed"),
                                     qsTr("Float 32"), qsTr("Float 64")]
  readonly property var orderLabels: [qsTr("Little endian"), qsTr("Big endian")]

  implicitHeight: layout.implicitHeight + 16

  //
  // Editable working copy of the field table. Rows: offset, bit, length, type, endian,
  // scale, add.
  //
  ListModel { id: fieldsModel }

  //
  // Reads the table from the project (also after undo or a template reset)
  //
  function load() {
    fieldsModel.clear()
    if (!root.editor)
      return

    const rows = root.editor.layoutFields
    for (let i = 0; i < rows.length; ++i)
      fieldsModel.append(rows[i])

    root.refreshSample()
  }

  //
  // Writes the working copy to the project and re-decodes the sample
  //
  function commit() {
    if (!root.editor)
      return

    const rows = []
    for (let i = 0; i < fieldsModel.count; ++i) {
      const row = fieldsModel.get(i)
      rows.push({
        "offset": row.offset,
        "bit": row.bit,
        "length": row.length,
        "type": row.type,
        "endian": row.endian,
        "scale": row.scale,
        "add": row.add
      })
    }

    root.editor.setLayoutFields(rows)
    root.refreshSample()
  }

  //
  // Decodes the pasted sample frame with the stored table
  //
  function refreshSample() {
    root.decoded = root.editor ? root.editor.decodeLayoutSample(sampleField.text) : []
  }

  //
  // Appends a field that starts where the last one ends
  //
  function addField() {
    if (fieldsModel.count >= root.maxFields)
      return

    let next = 0
    if (fieldsModel.count > 0) {
      const last = fieldsModel.get(fieldsModel.count - 1)
      const bits = last.type === 2 ? 32 : (last.type === 3 ? 64 : last.length)
      next = last.offset + Math.ceil((last.bit + bits) / 8)
    }

    fieldsModel.append({
      "offset": Math.min(next, 65535),
      "bit": 0,
      "length": 8,
      "type": 0,
      "endian": 0,
      "scale": 1,
      "add": 0
    })
    root.commit()
  }

  //
  // Stores one cell and commits; an unparsable number leaves the cell as it was
  //
  function setCell(index, key, value) {
    if (isNaN(value))
      return

    fieldsModel.setProperty(index, key, value)
    root.commit()
  }

  onEditorChanged: root.load()
  Component.onCompleted: root.load()

  Connections {
    target: root.editor

    function onLayoutFieldsChanged() {
      root.load()
    }
  }

  ColumnLayout {
    id: layout

    spacing: 4
    anchors.margins: 8
    anchors.fill: parent

    //
    // Introduction
    //
    Label {
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      font: Cpp_Misc_CommonFonts.uiFont
      color: Cpp_ThemeManager.colors["placeholder_text"]
      text: qsTr("Each row becomes one channel. Describe where the value sits in the frame "
                 + "and how its bits are read.")
    }

    //
    // Column headers: shared widths with the row delegate
    //
    RowLayout {
      Layout.topMargin: 4
      Layout.fillWidth: true
      spacing: root.colSpacing
      Layout.rightMargin: root.colScrollSlot

      Label {
        text: qsTr("#")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWIndex
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        text: qsTr("Byte")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWSmall
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        text: qsTr("Bit")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWSmall
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        text: qsTr("Bits")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWSmall
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        text: qsTr("Type")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWType
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        text: qsTr("Byte order")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWOrder
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        text: qsTr("Scale")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWNumber
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        text: qsTr("Offset")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWNumber
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        Layout.fillWidth: true
        text: qsTr("Sample value")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.minimumWidth: root.colWValue
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Item { Layout.preferredWidth: root.colWMove }
      Item { Layout.preferredWidth: root.colWMove }
      Item { Layout.preferredWidth: root.colWDelete }
    }

    Rectangle {
      implicitHeight: 1
      Layout.fillWidth: true
      color: Cpp_ThemeManager.colors["groupbox_border"]
    }

    //
    // Field rows
    //
    ListView {
      id: fieldList

      clip: true
      spacing: 1
      model: fieldsModel
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumHeight: 72
      boundsBehavior: Flickable.StopAtBounds
      Layout.preferredHeight: Math.min(contentHeight, 220)

      ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
      }

      delegate: Item {
        id: fieldRow

        required property int index
        required property int offset
        required property int bit
        required property int length
        required property int type
        required property int endian
        required property double scale
        required property double add

        readonly property bool isFloat: fieldRow.type >= 2

        implicitHeight: 32
        width: ListView.view.width

        RowLayout {
          anchors.fill: parent
          spacing: root.colSpacing
          anchors.rightMargin: root.colScrollSlot

          Label {
            text: String(fieldRow.index + 1)
            font: Cpp_Misc_CommonFonts.monoFont
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWIndex
            color: Cpp_ThemeManager.colors["placeholder_text"]
          }

          Widgets.LineField {
            text: String(fieldRow.offset)
            font: Cpp_Misc_CommonFonts.monoFont
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWSmall
            objectName: "fieldLayout.offset." + fieldRow.index
            validator: IntValidator { bottom: 0; top: 65535 }
            onEditingFinished: root.setCell(fieldRow.index, "offset", parseInt(text))
          }

          Widgets.LineField {
            text: String(fieldRow.bit)
            font: Cpp_Misc_CommonFonts.monoFont
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWSmall
            objectName: "fieldLayout.bit." + fieldRow.index
            validator: IntValidator { bottom: 0; top: 7 }
            onEditingFinished: root.setCell(fieldRow.index, "bit", parseInt(text))
          }

          Widgets.LineField {
            enabled: !fieldRow.isFloat
            font: Cpp_Misc_CommonFonts.monoFont
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWSmall
            objectName: "fieldLayout.length." + fieldRow.index
            validator: IntValidator { bottom: 1; top: 64 }
            onEditingFinished: root.setCell(fieldRow.index, "length", parseInt(text))
            text: fieldRow.type === 2 ? "32" : (fieldRow.type === 3 ? "64" : String(fieldRow.length))
          }

          Widgets.Combo {
            model: root.typeLabels
            currentIndex: fieldRow.type
            font: Cpp_Misc_CommonFonts.uiFont
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWType
            objectName: "fieldLayout.type." + fieldRow.index
            onActivated: root.setCell(fieldRow.index, "type", currentIndex)
          }

          Widgets.Combo {
            model: root.orderLabels
            currentIndex: fieldRow.endian
            font: Cpp_Misc_CommonFonts.uiFont
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWOrder
            objectName: "fieldLayout.endian." + fieldRow.index
            onActivated: root.setCell(fieldRow.index, "endian", currentIndex)
          }

          Widgets.LineField {
            text: String(fieldRow.scale)
            Layout.alignment: Qt.AlignVCenter
            font: Cpp_Misc_CommonFonts.monoFont
            Layout.preferredWidth: root.colWNumber
            objectName: "fieldLayout.scale." + fieldRow.index
            onEditingFinished: root.setCell(fieldRow.index, "scale", Number(text))
          }

          Widgets.LineField {
            text: String(fieldRow.add)
            Layout.alignment: Qt.AlignVCenter
            font: Cpp_Misc_CommonFonts.monoFont
            Layout.preferredWidth: root.colWNumber
            objectName: "fieldLayout.add." + fieldRow.index
            onEditingFinished: root.setCell(fieldRow.index, "add", Number(text))
          }

          Label {
            Layout.fillWidth: true
            elide: Text.ElideRight
            font: Cpp_Misc_CommonFonts.monoFont
            Layout.alignment: Qt.AlignVCenter
            Layout.minimumWidth: root.colWValue
            color: Cpp_ThemeManager.colors["text"]
            text: {
              const value = root.decoded[fieldRow.index]
              return value !== undefined && value.length > 0 ? value : "--"
            }
          }

          Widgets.IconButton {
            padding: 2
            iconSize: 14
            rotation: 90
            ToolTip.delay: 700
            mirrorIconInRtl: false
            ToolTip.visible: hovered
            enabled: fieldRow.index > 0
            Layout.preferredHeight: 24
            ToolTip.text: qsTr("Move up.")
            objectName: "fieldLayout.moveUp." + fieldRow.index
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWMove
            icon.source: "qrc:/icons/buttons/media-prev.svg"
            icon.color: enabled
                        ? Cpp_ThemeManager.colors["text"]
                        : Cpp_ThemeManager.colors["mid"]
            onClicked: {
              fieldsModel.move(fieldRow.index, fieldRow.index - 1, 1)
              root.commit()
            }
          }

          Widgets.IconButton {
            padding: 2
            iconSize: 14
            rotation: 90
            ToolTip.delay: 700
            mirrorIconInRtl: false
            ToolTip.visible: hovered
            Layout.preferredHeight: 24
            ToolTip.text: qsTr("Move down.")
            objectName: "fieldLayout.moveDown." + fieldRow.index
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWMove
            enabled: fieldRow.index < fieldsModel.count - 1
            icon.source: "qrc:/icons/buttons/media-next.svg"
            icon.color: enabled
                        ? Cpp_ThemeManager.colors["text"]
                        : Cpp_ThemeManager.colors["mid"]
            onClicked: {
              fieldsModel.move(fieldRow.index, fieldRow.index + 1, 1)
              root.commit()
            }
          }

          Widgets.IconButton {
            padding: 2
            iconSize: 14
            ToolTip.delay: 700
            ToolTip.visible: hovered
            enabled: fieldsModel.count > 1
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWDelete
            ToolTip.text: qsTr("Remove this field.")
            objectName: "fieldLayout.remove." + fieldRow.index
            icon.source: "qrc:/icons/buttons/trash.svg"
            icon.color: Cpp_ThemeManager.colors["text"]
            onClicked: {
              fieldsModel.remove(fieldRow.index)
              root.commit()
            }
          }
        }
      }
    }

    //
    // Add a field, and the sample frame the value column decodes
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      Widgets.IconButton {
        iconSize: 14
        ToolTip.delay: 700
        text: qsTr("Add Field")
        ToolTip.visible: hovered
        objectName: "fieldLayout.add"
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/icons/buttons/plus.svg"
        enabled: fieldsModel.count < root.maxFields
        ToolTip.text: qsTr("Add a field after the last one.")
        onClicked: root.addField()
      }

      Label {
        text: qsTr("Sample frame:")
        font: Cpp_Misc_CommonFonts.uiFont
        Layout.alignment: Qt.AlignVCenter
      }

      Widgets.LineField {
        id: sampleField

        Layout.fillWidth: true
        objectName: "fieldLayout.sample"
        Layout.alignment: Qt.AlignVCenter
        font: Cpp_Misc_CommonFonts.monoFont
        placeholderText: qsTr("Paste a frame as hexadecimal bytes, for example A5 FF 85")
        onTextChanged: root.refreshSample()
      }
    }
  }
}
