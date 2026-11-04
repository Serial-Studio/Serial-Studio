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

import "../../Widgets" as Widgets

//
// Value labels editor (spec 0108): integer value -> text rows plus one catch-all text.
//
Widgets.SmartDialog {
  id: root

  objectName: "valueLabelsEditor"

  fixedSize: false
  title: qsTr("Value Labels")
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  readonly property int colSpacing: 8
  readonly property int colWText: 260
  readonly property int colWValue: 120
  readonly property int colWDelete: 28
  readonly property int colScrollSlot: 12

  property int uniqueId: -1
  property string defaultText: ""

  ListModel {
    id: rowsModel
  }

  //
  // Public API
  //
  function showDialog(uniqueId, rows, defaultText) {
    root.uniqueId = uniqueId
    root.defaultText = defaultText ?? ""
    rowsModel.clear()
    for (let i = 0; i < (rows ? rows.length : 0); ++i)
      rowsModel.append({ value: String(rows[i].value), text: String(rows[i].text ?? "") })

    root.show()
    root.raise()
    root.requestActivate()
  }

  function collectRows() {
    const out = []
    for (let i = 0; i < rowsModel.count; ++i) {
      const r = rowsModel.get(i)
      const value = Number(String(r.value).trim())
      const text = String(r.text).trim()
      if (String(r.value).trim().length === 0 || !Number.isInteger(value) || text.length === 0)
        continue

      out.push({ value: value, text: text })
    }
    return out
  }

  function addRow() {
    const next = rowsModel.count > 0 ? Number(rowsModel.get(rowsModel.count - 1).value) + 1 : 0
    rowsModel.append({ value: String(Number.isInteger(next) ? next : 0), text: "" })
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
      Layout.maximumWidth: root.colWValue + root.colWText + root.colWDelete + 40
      text: qsTr("Text shown in place of the value wherever it appears as text. Plots, alarms "
                 + "and exports keep the number. A non-integer value is rounded before the "
                 + "lookup.")
    }

    RowLayout {
      Layout.fillWidth: true
      spacing: root.colSpacing
      Layout.rightMargin: root.colScrollSlot

      Label {
        text: qsTr("Value")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWValue
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }

      Label {
        text: qsTr("Label")
        Layout.fillWidth: true
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWText
        color: Cpp_ThemeManager.colors["placeholder_text"]
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
        required property string value
        required property string text

        implicitHeight: 36
        width: ListView.view.width

        RowLayout {
          anchors.fill: parent
          spacing: root.colSpacing
          anchors.rightMargin: root.colScrollSlot

          Widgets.LineField {
            text: row.value
            objectName: "valueLabelsEditor.value"
            font: Cpp_Misc_CommonFonts.monoFont
            Layout.preferredWidth: root.colWValue
            validator: RegularExpressionValidator { regularExpression: /^-?\d{0,16}$/ }
            onEditingFinished: rowsModel.setProperty(row.index, "value", text)
          }

          Widgets.LineField {
            text: row.text
            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.uiFont
            placeholderText: qsTr("Label text")
            Layout.preferredWidth: root.colWText
            objectName: "valueLabelsEditor.text"
            onEditingFinished: rowsModel.setProperty(row.index, "text", text)
          }

          Widgets.IconButton {
            padding: 2
            iconSize: 14
            ToolTip.delay: 700
            ToolTip.visible: hovered
            Layout.preferredHeight: 24
            Layout.preferredWidth: root.colWDelete
            objectName: "valueLabelsEditor.remove"
            ToolTip.text: qsTr("Remove this label.")
            icon.source: "qrc:/icons/buttons/trash.svg"
            onClicked: rowsModel.remove(row.index)
          }
        }
      }
    }

    //
    // Catch-all row: fixed below the table, applies to every value no row lists
    //
    RowLayout {
      Layout.fillWidth: true
      spacing: root.colSpacing
      Layout.rightMargin: root.colScrollSlot + root.colWDelete + root.colSpacing

      Label {
        text: qsTr("Any other")
        Layout.preferredWidth: root.colWValue
        color: Cpp_ThemeManager.colors["text"]
      }

      Widgets.LineField {
        text: root.defaultText
        Layout.fillWidth: true
        font: Cpp_Misc_CommonFonts.uiFont
        Layout.preferredWidth: root.colWText
        objectName: "valueLabelsEditor.catchAll"
        placeholderText: qsTr("(show the number)")
        onEditingFinished: root.defaultText = text
      }
    }

    RowLayout {
      spacing: 4
      Layout.fillWidth: true

      Widgets.IconButton {
        iconSize: 16
        text: qsTr("Add Label")
        onClicked: root.addRow()
        objectName: "valueLabelsEditor.add"
        icon.source: "qrc:/icons/buttons/plus.svg"
      }

      Item {
        Layout.fillWidth: true
      }

      Widgets.IconButton {
        iconSize: 16
        text: qsTr("Cancel")
        onClicked: root.close()
        objectName: "valueLabelsEditor.cancel"
        icon.source: "qrc:/icons/buttons/close.svg"
      }

      Widgets.IconButton {
        iconSize: 16
        highlighted: true
        text: qsTr("Apply")
        objectName: "valueLabelsEditor.apply"
        icon.source: "qrc:/icons/buttons/apply.svg"
        onClicked: {
          Cpp_JSON_ProjectEditor.commitValueLabels(root.uniqueId, root.collectRows(),
                                                   root.defaultText.trim())
          root.close()
        }
      }
    }
  }
}
