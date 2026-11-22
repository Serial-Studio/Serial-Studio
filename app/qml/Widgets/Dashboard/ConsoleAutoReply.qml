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
import ".." as Widgets

//
// Auto-reply rule table (spec 0127 R4-R7): when a received message matches a rule, the console
// sends that rule's reply. Rules are app-wide; answering starts only when the user arms it.
//
Item {
  id: root

  objectName: "console.autoReplyPanel"

  readonly property var reply: Cpp_Console_Handler.autoReply
  readonly property var checksumNames: root.reply.checksumNames
  readonly property var modeLabels: [qsTr("Starts with"), qsTr("Ends with"), qsTr("Equals"),
                                     qsTr("Contains"), qsTr("Pattern"), qsTr("Regular expression")]

  //
  // Why the last edit was refused; cleared by the next accepted one
  //
  property string problem: ""

  //
  // Shared column widths, so the header row and the rule rows line up
  //
  readonly property int colWOn: 24
  readonly property int colWHex: 44
  readonly property int colWEol: 120
  readonly property int colWMove: 24
  readonly property int colWMode: 150
  readonly property int colSpacing: 6
  readonly property int colWDelete: 28
  readonly property int colWChecksum: 130
  readonly property int colScrollSlot: 12

  //
  // Deep copy of a stored rule with one field replaced; `key` may name a reply field as
  // "reply.payload"
  //
  function patched(rule, key, value) {
    const copy = JSON.parse(JSON.stringify(rule))
    if (key.startsWith("reply."))
      copy.reply[key.substring(6)] = value
    else
      copy[key] = value

    return copy
  }

  //
  // Stores an edited rule, or reports why it cannot be stored and leaves the old one in place
  //
  function commit(index, rule) {
    const why = root.reply.ruleProblem(rule)
    root.problem = why
    if (why.length === 0)
      root.reply.updateRule(index, rule)
  }

  function addRule() {
    root.problem = ""
    root.reply.addRule({
      "match": "OK",
      "mode": 0,
      "matchHex": false,
      "enabled": false,
      "reply": {
        "payload": "ACK",
        "hex": false,
        "lineEnding": 1,
        "checksumName": "",
        "encoding": 0
      }
    })
  }

  ColumnLayout {
    spacing: 4
    anchors.fill: parent

    //
    // Introduction and the arm switch
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      Label {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        font: Cpp_Misc_CommonFonts.uiFont
        color: Cpp_ThemeManager.colors["placeholder_text"]
        text: qsTr("When a received message matches a rule, its reply is sent to the same "
                   + "device. Text rules look at one line at a time. Hexadecimal rules look at "
                   + "the bytes received until the line goes quiet.")
      }

      Label {
        visible: root.reply.armed
        Layout.alignment: Qt.AlignVCenter
        font: Cpp_Misc_CommonFonts.monoFont
        text: qsTr("%1 sent").arg(root.reply.replyCount)
      }

      Widgets.PushButton {
        id: armButton

        objectName: "console.autoReply.arm"

        checkable: true
        checked: root.reply.armed
        Layout.alignment: Qt.AlignVCenter
        enabled: Cpp_IO_Manager.readWrite || root.reply.armed
        text: root.reply.armed ? qsTr("Stop Replying") : qsTr("Start Replying")
        onClicked: root.reply.armed ? root.reply.disarm() : root.reply.arm()
        ToolTip.delay: 700
        ToolTip.visible: hovered
        ToolTip.text: root.reply.armed
                      ? qsTr("Stop answering received messages")
                      : qsTr("Answer received messages with the enabled rules. Stops when the "
                             + "device disconnects.")

        //
        // A click severs the `checked` binding and C++ can refuse or drop the armed state,
        // so it is always synced back from the responder
        //
        Connections {
          target: root.reply
          function onArmedChanged() { armButton.checked = root.reply.armed }
        }
      }
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
        text: qsTr("When a message")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWMode
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        text: qsTr("Hex")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWHex
        horizontalAlignment: Text.AlignHCenter
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        Layout.fillWidth: true
        text: qsTr("This text or bytes")
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        Layout.fillWidth: true
        text: qsTr("Send this reply")
        font: Cpp_Misc_CommonFonts.boldUiFont
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        text: qsTr("Hex")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWHex
        horizontalAlignment: Text.AlignHCenter
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        text: qsTr("Line ending")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWEol
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }
      Label {
        text: qsTr("Checksum")
        font: Cpp_Misc_CommonFonts.boldUiFont
        Layout.preferredWidth: root.colWChecksum
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
      id: ruleList

      clip: true
      spacing: 1
      Layout.fillWidth: true
      model: root.reply.rules
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
            objectName: "console.autoReply.enabled." + ruleRow.index
            onClicked: root.reply.setRuleEnabled(ruleRow.index, checked)
          }

          Widgets.Combo {
            model: root.modeLabels
            currentIndex: ruleRow.rule.mode
            font: Cpp_Misc_CommonFonts.uiFont
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWMode
            objectName: "console.autoReply.mode." + ruleRow.index
            onActivated: root.commit(ruleRow.index,
                                     root.patched(ruleRow.rule, "mode", currentIndex))
          }

          CheckBox {
            checked: ruleRow.rule.matchHex
            Layout.preferredWidth: root.colWHex
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            objectName: "console.autoReply.matchHex." + ruleRow.index
            onClicked: root.commit(ruleRow.index,
                                   root.patched(ruleRow.rule, "matchHex", checked))
          }

          Widgets.LineField {
            Layout.fillWidth: true
            text: ruleRow.rule.match
            Layout.alignment: Qt.AlignVCenter
            font: Cpp_Misc_CommonFonts.monoFont
            objectName: "console.autoReply.match." + ruleRow.index
            onEditingFinished: {
              if (text !== ruleRow.rule.match)
                root.commit(ruleRow.index, root.patched(ruleRow.rule, "match", text))
            }
          }

          Widgets.LineField {
            Layout.fillWidth: true
            text: ruleRow.rule.reply.payload
            Layout.alignment: Qt.AlignVCenter
            font: Cpp_Misc_CommonFonts.monoFont
            objectName: "console.autoReply.reply." + ruleRow.index
            onEditingFinished: {
              if (text !== ruleRow.rule.reply.payload)
                root.commit(ruleRow.index, root.patched(ruleRow.rule, "reply.payload", text))
            }
          }

          CheckBox {
            checked: ruleRow.rule.reply.hex
            Layout.preferredWidth: root.colWHex
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            objectName: "console.autoReply.replyHex." + ruleRow.index
            onClicked: root.commit(ruleRow.index,
                                   root.patched(ruleRow.rule, "reply.hex", checked))
          }

          Widgets.Combo {
            font: Cpp_Misc_CommonFonts.uiFont
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWEol
            model: Cpp_Console_Handler.lineEndings
            currentIndex: ruleRow.rule.reply.lineEnding
            objectName: "console.autoReply.lineEnding." + ruleRow.index
            onActivated: root.commit(ruleRow.index,
                                     root.patched(ruleRow.rule, "reply.lineEnding", currentIndex))
          }

          Widgets.Combo {
            font: Cpp_Misc_CommonFonts.uiFont
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.colWChecksum
            model: Cpp_Console_Handler.checksumMethods
            objectName: "console.autoReply.checksum." + ruleRow.index
            currentIndex: Math.max(0, root.checksumNames.indexOf(ruleRow.rule.reply.checksumName))
            onActivated: root.commit(ruleRow.index,
                                     root.patched(ruleRow.rule, "reply.checksumName",
                                                  root.checksumNames[currentIndex]))
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
            objectName: "console.autoReply.moveUp." + ruleRow.index
            icon.color: enabled
                        ? Cpp_ThemeManager.colors["text"]
                        : Cpp_ThemeManager.colors["mid"]
            onClicked: root.reply.moveRule(ruleRow.index, ruleRow.index - 1)
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
            enabled: ruleRow.index < root.reply.rules.length - 1
            objectName: "console.autoReply.moveDown." + ruleRow.index
            icon.color: enabled
                        ? Cpp_ThemeManager.colors["text"]
                        : Cpp_ThemeManager.colors["mid"]
            onClicked: root.reply.moveRule(ruleRow.index, ruleRow.index + 1)
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
            objectName: "console.autoReply.remove." + ruleRow.index
            onClicked: root.reply.removeRule(ruleRow.index)
          }
        }
      }

      //
      // Empty-state hint
      //
      Label {
        anchors.centerIn: parent
        font: Cpp_Misc_CommonFonts.uiFont
        visible: root.reply.rules.length === 0
        color: Cpp_ThemeManager.colors["placeholder_text"]
        text: qsTr("No rules yet. Add one, edit what it looks for and what it answers, "
                   + "then tick it.")
      }
    }

    //
    // Add a rule, the idle gap for hexadecimal rules, and the reason an edit was refused
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      Widgets.IconButton {
        iconSize: 14
        ToolTip.delay: 700
        text: qsTr("Add Rule")
        ToolTip.visible: hovered
        Layout.alignment: Qt.AlignVCenter
        objectName: "console.autoReply.add"
        icon.source: "qrc:/icons/buttons/plus.svg"
        enabled: root.reply.rules.length < root.reply.maxRules
        ToolTip.text: qsTr("Add a rule. New rules start unticked.")
        onClicked: root.addRule()
      }

      Label {
        text: qsTr("Quiet time:")
        font: Cpp_Misc_CommonFonts.uiFont
        Layout.alignment: Qt.AlignVCenter
      }

      Widgets.SpinBox {
        from: 1
        to: 1000
        editable: true
        value: root.reply.gapMs
        Layout.preferredWidth: 96
        Layout.alignment: Qt.AlignVCenter
        objectName: "console.autoReply.gap"
        onValueModified: root.reply.gapMs = value
        ToolTip.delay: 700
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Milliseconds without data that end a message for hexadecimal rules.")
      }

      Label {
        text: qsTr("ms")
        font: Cpp_Misc_CommonFonts.uiFont
        Layout.alignment: Qt.AlignVCenter
      }

      Label {
        text: root.problem
        Layout.fillWidth: true
        elide: Text.ElideRight
        visible: root.problem.length > 0
        font: Cpp_Misc_CommonFonts.uiFont
        Layout.alignment: Qt.AlignVCenter
        color: Cpp_ThemeManager.colors["alarm"]
      }

      Item {
        Layout.fillWidth: true
        visible: root.problem.length === 0
      }
    }
  }
}
