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
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import ".." as Widgets

//
// Console colour rules (spec 0127 R8/R9): each rule paints a keyword in its own colours wherever
// it appears in the console. Rules are app-wide and only change how text is drawn.
//
Item {
  id: root

  objectName: "console.colorRulesPanel"

  readonly property var rules: Cpp_Console_Handler.highlightRules

  //
  // Shared column widths, so the header row and the rule rows line up
  //
  readonly property int colWOn: 24
  readonly property int colWCase: 96
  readonly property int colWMove: 24
  readonly property int colWColor: 56
  readonly property int colSpacing: 6
  readonly property int colWDelete: 28
  readonly property int colWPreview: 160
  readonly property int colScrollSlot: 12

  //
  // Copy of a stored rule with one field replaced
  //
  function patched(rule, key, value) {
    const copy = JSON.parse(JSON.stringify(rule))
    copy[key] = value
    return copy
  }

  function addRule() {
    root.rules.addRule({
      "keyword": "ERROR",
      "caseSensitive": true,
      "foreground": "#ffffff",
      "background": "#c0392b",
      "enabled": true
    })
  }

  //
  // Single ColorDialog reused for every row; the target row and field are tracked here
  //
  ColorDialog {
    id: colorDialog

    property int targetRow: -1
    property string targetKey: "foreground"

    title: qsTr("Choose Color")
    onAccepted: {
      if (colorDialog.targetRow < 0 || colorDialog.targetRow >= root.rules.rules.length)
        return

      const rule = root.rules.rules[colorDialog.targetRow]
      root.rules.updateRule(colorDialog.targetRow,
                            root.patched(rule, colorDialog.targetKey,
                                         String(colorDialog.selectedColor)))
    }
  }

  component Swatch: Rectangle {
    id: swatch

    required property int row
    required property string key
    required property string hint

    radius: 3
    border.width: 1
    implicitWidth: 24
    implicitHeight: 24
    border.color: Cpp_ThemeManager.colors["widget_border"]

    MouseArea {
      hoverEnabled: true
      anchors.fill: parent
      cursorShape: Qt.PointingHandCursor

      ToolTip.delay: 700
      ToolTip.text: swatch.hint
      ToolTip.visible: containsMouse

      onClicked: {
        colorDialog.targetRow = swatch.row
        colorDialog.targetKey = swatch.key
        colorDialog.selectedColor = swatch.color
        colorDialog.open()
      }
    }
  }

  ColumnLayout {
    spacing: 4
    anchors.fill: parent

    Label {
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      font: Cpp_Misc_CommonFonts.uiFont
      color: Cpp_ThemeManager.colors["placeholder_text"]
      text: qsTr("Each rule colours a word wherever it appears in the console. Only the display "
                 + "changes: copied text and the console log stay as received. Where two rules "
                 + "overlap, the one higher in the list wins.")
    }

    //
    // Column headers: shared widths with the row delegate
    //
    RowLayout {
      Layout.topMargin: 4
      Layout.fillWidth: true
      spacing: root.colSpacing
      Layout.rightMargin: root.colScrollSlot

      Item { Layout.preferredWidth: root.colWOn }
      Label {
        text: qsTr("Word")
        Layout.fillWidth: true
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        text: qsTr("Match case")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWCase
        horizontalAlignment: Text.AlignHCenter
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        text: qsTr("Text")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWColor
        horizontalAlignment: Text.AlignHCenter
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        text: qsTr("Fill")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWColor
        horizontalAlignment: Text.AlignHCenter
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        text: qsTr("Preview")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWPreview
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
    // Rule rows
    //
    ListView {
      clip: true
      spacing: 1
      Layout.fillWidth: true
      model: root.rules.rules
      Layout.fillHeight: true
      boundsBehavior: Flickable.StopAtBounds

      ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
      }

      delegate: Item {
        id: ruleRow

        required property int index
        required property var modelData

        readonly property var rule: ruleRow.modelData

        implicitHeight: 30
        width: ListView.view.width

        RowLayout {
          anchors.fill: parent
          spacing: root.colSpacing
          anchors.rightMargin: root.colScrollSlot

          CheckBox {
            checked: ruleRow.rule.enabled
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWOn
            objectName: "console.colorRules.enabled." + ruleRow.index
            onClicked: root.rules.setRuleEnabled(ruleRow.index, checked)
          }

          Widgets.LineField {
            maximumLength: 64
            Layout.fillWidth: true
            text: ruleRow.rule.keyword
            Layout.alignment: Qt.AlignVCenter
            font: Cpp_Misc_CommonFonts.monoFont
            objectName: "console.colorRules.keyword." + ruleRow.index
            onEditingFinished: {
              if (text.length > 0 && text !== ruleRow.rule.keyword)
                root.rules.updateRule(ruleRow.index, root.patched(ruleRow.rule, "keyword", text))
            }
          }

          CheckBox {
            checked: ruleRow.rule.caseSensitive
            Layout.preferredWidth: root.colWCase
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            objectName: "console.colorRules.case." + ruleRow.index
            onClicked: root.rules.updateRule(ruleRow.index,
                                             root.patched(ruleRow.rule, "caseSensitive", checked))
          }

          Item {
            Layout.preferredHeight: 24
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWColor

            Swatch {
              key: "foreground"
              row: ruleRow.index
              anchors.centerIn: parent
              color: ruleRow.rule.foreground
              hint: qsTr("Click to choose the text color.")
            }
          }

          Item {
            Layout.preferredHeight: 24
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWColor

            Swatch {
              key: "background"
              row: ruleRow.index
              anchors.centerIn: parent
              color: ruleRow.rule.background
              hint: qsTr("Click to choose the fill color.")
            }
          }

          Rectangle {
            radius: 2
            Layout.preferredHeight: 22
            color: ruleRow.rule.background
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWPreview
            opacity: ruleRow.rule.enabled ? 1 : 0.4

            Label {
              anchors.fill: parent
              anchors.leftMargin: 6
              anchors.rightMargin: 6
              elide: Text.ElideRight
              text: ruleRow.rule.keyword
              color: ruleRow.rule.foreground
              font: Cpp_Misc_CommonFonts.monoFont
              verticalAlignment: Text.AlignVCenter
            }
          }

          Widgets.IconButton {
            padding: 2
            iconSize: 14
            rotation: 90
            ToolTip.delay: 700
            mirrorIconInRtl: false
            ToolTip.visible: hovered
            enabled: ruleRow.index > 0
            Layout.preferredHeight: 24
            ToolTip.text: qsTr("Move up.")
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWMove
            icon.source: "qrc:/icons/buttons/media-prev.svg"
            objectName: "console.colorRules.moveUp." + ruleRow.index
            icon.color: enabled
                        ? Cpp_ThemeManager.colors["text"]
                        : Cpp_ThemeManager.colors["mid"]
            onClicked: root.rules.moveRule(ruleRow.index, ruleRow.index - 1)
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
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWMove
            icon.source: "qrc:/icons/buttons/media-next.svg"
            enabled: ruleRow.index < root.rules.rules.length - 1
            objectName: "console.colorRules.moveDown." + ruleRow.index
            icon.color: enabled
                        ? Cpp_ThemeManager.colors["text"]
                        : Cpp_ThemeManager.colors["mid"]
            onClicked: root.rules.moveRule(ruleRow.index, ruleRow.index + 1)
          }

          Widgets.IconButton {
            padding: 2
            iconSize: 14
            ToolTip.delay: 700
            ToolTip.visible: hovered
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWDelete
            ToolTip.text: qsTr("Remove this rule.")
            icon.source: "qrc:/icons/buttons/trash.svg"
            icon.color: Cpp_ThemeManager.colors["text"]
            objectName: "console.colorRules.remove." + ruleRow.index
            onClicked: root.rules.removeRule(ruleRow.index)
          }
        }
      }

      //
      // Empty-state hint
      //
      Label {
        anchors.centerIn: parent
        font: Cpp_Misc_CommonFonts.uiFont
        visible: root.rules.rules.length === 0
        color: Cpp_ThemeManager.colors["placeholder_text"]
        text: qsTr("No rules yet. Add one and type the word to colour.")
      }
    }

    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      Widgets.IconButton {
        iconSize: 14
        ToolTip.delay: 700
        text: qsTr("Add Rule")
        ToolTip.visible: hovered
        Layout.alignment: Qt.AlignVCenter
        objectName: "console.colorRules.add"
        icon.source: "qrc:/icons/buttons/plus.svg"
        enabled: root.rules.rules.length < root.rules.maxRules
        ToolTip.text: qsTr("Add a rule, then change its word and colors.")
        onClicked: root.addRule()
      }

      Item {
        Layout.fillWidth: true
      }
    }
  }
}
