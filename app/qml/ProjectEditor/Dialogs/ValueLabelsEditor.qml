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
// Value labels editor (spec 0108): whole numbers shown as words, plus one text for the rest.
//
Widgets.SmartDialog {
  id: root

  objectName: "valueLabelsEditor"

  fixedSize: false
  title: qsTr("Value Labels")
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  readonly property int colSpacing: 8
  readonly property int colWValue: 120
  readonly property int colWDelete: 28
  readonly property int contentWidth: 560
  readonly property int colScrollSlot: 12

  property int uniqueId: -1
  property string defaultText: ""

  //
  // Working copy of the table. Rows: value (text of a whole number), text.
  //
  ListModel {
    id: rowsModel
  }

  //
  // Presets for common two- and three-state signals; each replaces the table.
  //
  readonly property var presets: [
    {
      label: qsTr("Off / On"),
      rows: [{ value: 0, text: qsTr("Off") }, { value: 1, text: qsTr("On") }]
    },
    {
      label: qsTr("Closed / Open"),
      rows: [{ value: 0, text: qsTr("Closed") }, { value: 1, text: qsTr("Open") }]
    },
    {
      label: qsTr("Stopped / Running"),
      rows: [{ value: 0, text: qsTr("Stopped") }, { value: 1, text: qsTr("Running") }]
    },
    {
      label: qsTr("No / Yes"),
      rows: [{ value: 0, text: qsTr("No") }, { value: 1, text: qsTr("Yes") }]
    },
    {
      label: qsTr("Low / Medium / High"),
      rows: [{ value: 0, text: qsTr("Low") },
             { value: 1, text: qsTr("Medium") },
             { value: 2, text: qsTr("High") }]
    },
    {
      label: qsTr("OK / Warning / Fault"),
      rows: [{ value: 0, text: qsTr("OK") },
             { value: 1, text: qsTr("Warning") },
             { value: 2, text: qsTr("Fault") }]
    }
  ]

  //
  // Public API
  //
  function showDialog(uniqueId, rows, defaultText) {
    root.uniqueId = uniqueId
    root.defaultText = defaultText ?? ""
    rowsModel.clear()
    for (let i = 0; i < (rows ? rows.length : 0); ++i)
      rowsModel.append({ value: String(rows[i].value), text: String(rows[i].text ?? "") })

    presetCombo.currentIndex = -1
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

  function applyPreset(index) {
    if (index < 0 || index >= root.presets.length)
      return

    rowsModel.clear()
    const rows = root.presets[index].rows
    for (let i = 0; i < rows.length; ++i)
      rowsModel.append({ value: String(rows[i].value), text: rows[i].text })
  }

  //
  // True when two rows list the same number; only the first one is kept on Apply.
  //
  function hasDuplicates() {
    const seen = {}
    for (let i = 0; i < rowsModel.count; ++i) {
      const key = String(rowsModel.get(i).value).trim()
      if (key.length === 0)
        continue

      if (seen[key])
        return true

      seen[key] = true
    }
    return false
  }

  //--------------------------------------------------------------------------------------------
  // Layout: section labels live outside their content cards, as in the Alarm Bands editor
  //--------------------------------------------------------------------------------------------

  dialogContent: ColumnLayout {
    id: layout

    spacing: 12
    anchors.centerIn: parent
    Layout.minimumWidth: root.contentWidth

    //
    // Introduction: what the dialog does, in one plain sentence and one caveat
    //
    Label {
      wrapMode: Text.WordWrap
      Layout.fillWidth: true
      Layout.maximumWidth: root.contentWidth
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Show words instead of numbers. For example, when this dataset reports 1, "
                 + "the dashboard can show \"Running\" instead. Plots, alarms, and exports still "
                 + "use the number.")
    }

    //
    // Section 1: presets
    //
    ColumnLayout {
      spacing: 4
      Layout.fillWidth: true

      Label {
        text: qsTr("Start From a Preset")
        font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      }

      Rectangle {
        radius: 2
        border.width: 1
        Layout.fillWidth: true
        implicitHeight: presetLayout.implicitHeight + 16
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        RowLayout {
          id: presetLayout

          spacing: 8
          anchors.margins: 8
          anchors.fill: parent

          Label {
            text: qsTr("Preset")
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["text"]
          }

          ComboBox {
            id: presetCombo

            objectName: "valueLabelsEditor.preset"

            currentIndex: -1
            textRole: "label"
            model: root.presets
            Layout.minimumWidth: 240
            font: Cpp_Misc_CommonFonts.uiFont
            Layout.alignment: Qt.AlignVCenter
            displayText: currentIndex < 0 ? qsTr("Choose a preset…")
                                          : root.presets[currentIndex].label
            onActivated: {
              root.applyPreset(presetCombo.currentIndex)
              presetCombo.currentIndex = -1
            }
          }

          Label {
            opacity: 0.7
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
            text: qsTr("Replaces the labels below.")
          }
        }
      }
    }

    //
    // Section 2: the number -> word table
    //
    ColumnLayout {
      spacing: 4
      Layout.fillWidth: true
      Layout.fillHeight: true

      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Label {
          text: qsTr("Labels")
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
          text: qsTr("Add Label")
          ToolTip.visible: hovered
          onClicked: root.addRow()
          Layout.alignment: Qt.AlignVCenter
          objectName: "valueLabelsEditor.add"
          icon.source: "qrc:/icons/buttons/plus.svg"
          ToolTip.text: qsTr("Add a row for the next number.")
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
              text: qsTr("Number")
              font: Cpp_Misc_CommonFonts.boldUiFont
              Layout.preferredWidth: root.colWValue
              color: Cpp_ThemeManager.colors["placeholder_text"]
            }

            Label {
              text: qsTr("Shown As")
              Layout.fillWidth: true
              font: Cpp_Misc_CommonFonts.boldUiFont
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
                  Layout.alignment: Qt.AlignVCenter
                  Layout.preferredWidth: root.colWValue
                  validator: RegularExpressionValidator { regularExpression: /^-?\d{0,16}$/ }
                  onEditingFinished: rowsModel.setProperty(row.index, "value", text)
                }

                Widgets.LineField {
                  text: row.text
                  Layout.fillWidth: true
                  font: Cpp_Misc_CommonFonts.uiFont
                  Layout.alignment: Qt.AlignVCenter
                  objectName: "valueLabelsEditor.text"
                  placeholderText: qsTr("For example, Running")
                  onEditingFinished: rowsModel.setProperty(row.index, "text", text)
                }

                Widgets.IconButton {
                  padding: 2
                  iconSize: 14
                  ToolTip.delay: 700
                  ToolTip.visible: hovered
                  Layout.alignment: Qt.AlignVCenter
                  onClicked: rowsModel.remove(row.index)
                  Layout.preferredWidth: root.colWDelete
                  objectName: "valueLabelsEditor.remove"
                  ToolTip.text: qsTr("Remove this label.")
                  icon.source: "qrc:/icons/buttons/trash.svg"
                  icon.color: Cpp_ThemeManager.colors["text"]
                }
              }
            }

            //
            // Empty state: visible only while no label is defined
            //
            Label {
              width: parent.width
              anchors.centerIn: parent
              wrapMode: Text.WordWrap
              visible: rowsModel.count === 0
              font: Cpp_Misc_CommonFonts.uiFont
              horizontalAlignment: Text.AlignHCenter
              color: Cpp_ThemeManager.colors["placeholder_text"]
              text: qsTr("No labels yet. Choose a preset or click Add Label.")
            }
          }
        }
      }

      Label {
        opacity: 0.7
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        color: Cpp_ThemeManager.colors["text"]
        font: Cpp_Misc_CommonFonts.customUiFont(0.85, false)
        text: rowsModel.count > 0 && root.hasDuplicates()
              ? qsTr("A number appears more than once. Only its first row is kept when you "
                     + "click Apply.")
              : qsTr("Numbers with decimals are rounded to the nearest whole number before "
                     + "they are matched.")
      }
    }

    //
    // Section 3: what to show for a number that is not in the table
    //
    ColumnLayout {
      spacing: 4
      Layout.fillWidth: true

      Label {
        text: qsTr("Other Numbers")
        font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
        color: Cpp_ThemeManager.colors["pane_section_label"]
        Component.onCompleted: font.capitalization = Font.AllUppercase
      }

      Rectangle {
        radius: 2
        border.width: 1
        Layout.fillWidth: true
        implicitHeight: otherLayout.implicitHeight + 16
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        RowLayout {
          id: otherLayout

          anchors.margins: 8
          anchors.fill: parent
          spacing: root.colSpacing

          Label {
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["text"]
            text: qsTr("Any number not listed")
          }

          Widgets.LineField {
            text: root.defaultText
            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.uiFont
            Layout.alignment: Qt.AlignVCenter
            objectName: "valueLabelsEditor.catchAll"
            onEditingFinished: root.defaultText = text
            placeholderText: qsTr("Leave empty to show the number")
          }
        }
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
        objectName: "valueLabelsEditor.cancel"
        ToolTip.text: qsTr("Discard changes.")
        icon.source: "qrc:/icons/buttons/close.svg"
      }

      Widgets.IconButton {
        iconSize: 16
        highlighted: true
        ToolTip.delay: 700
        text: qsTr("Apply")
        ToolTip.visible: hovered
        objectName: "valueLabelsEditor.apply"
        icon.source: "qrc:/icons/buttons/apply.svg"
        ToolTip.text: qsTr("Save the labels to this dataset.")
        onClicked: {
          forceActiveFocus()
          Cpp_JSON_ProjectEditor.commitValueLabels(root.uniqueId, root.collectRows(),
                                                   root.defaultText.trim())
          root.close()
        }
      }
    }
  }
}
