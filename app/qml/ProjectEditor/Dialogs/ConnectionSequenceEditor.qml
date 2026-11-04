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

import "../../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  objectName: "connectionSequenceEditor"

  fixedSize: false
  title: qsTr("Connection Sequence")
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Emitted with the edited sequence as JSON text; the host commits it
  //
  signal applied(string json)

  //
  // Whole-sequence time budget and the first validation problem (step -1: whole sequence)
  //
  property int budgetMs: 10000
  property var verdict: ({ step: -1, message: "" })

  //
  // Shared column widths, so the header row and the step rows line up
  //
  readonly property int colWMove: 24
  readonly property int colWKind: 150
  readonly property int colWIndex: 24
  readonly property int colSpacing: 8
  readonly property int colWDelete: 28
  readonly property int colScrollSlot: 12

  //
  // Step kinds: JSON slug and the label the manual and the console use
  //
  readonly property var kinds: [
    { slug: "break", label: qsTr("Send BREAK") },
    { slug: "wait", label: qsTr("Wait") },
    { slug: "line", label: qsTr("Set Control Line") },
    { slug: "send", label: qsTr("Send Data") },
    { slug: "expect", label: qsTr("Expect Reply") },
    { slug: "discard", label: qsTr("Clear Received Data") },
    { slug: "baud", label: qsTr("Set Baud Rate") },
    { slug: "flow", label: qsTr("Set Flow Control") }
  ]
  readonly property var kindLabels: root.kinds.map(k => k.label)

  //
  // End-of-line choices of a send step, stored as escape text like an action's
  //
  readonly property var lineEndings: [
    { value: "", label: qsTr("No Line Ending") },
    { value: "\\n", label: qsTr("New Line") },
    { value: "\\r", label: qsTr("Carriage Return") },
    { value: "\\r\\n", label: qsTr("CR + NL") }
  ]
  readonly property var lineEndingLabels: root.lineEndings.map(e => e.label)

  //
  // Flow-control slugs in the order of Cpp_IO_Serial.flowControlList
  //
  readonly property var flowModes: ["none", "rtscts", "xonxoff"]

  //
  // Working copy: one row per step, every role present so the model stays uniform
  //
  ListModel { id: stepsModel }

  //
  // Public API
  //
  function showDialog(json) {
    root.loadJson(json)
    root.show()
    root.raise()
    root.requestActivate()
  }

  function defaults(kind) {
    return {
      kind: kind,
      ms: kind === "break" ? 55 : 100,
      line: "dtr",
      high: true,
      payload: "",
      hex: false,
      eol: "",
      checksum: "",
      patterns: "",
      timeoutMs: 1000,
      optional: false,
      rate: 115200,
      mode: "none"
    }
  }

  function loadJson(json) {
    let sequence = {}
    try {
      sequence = JSON.parse(json && json.length > 0 ? json : "{}")
    } catch (e) {
      sequence = {}
    }

    stepsModel.clear()
    root.budgetMs = Number(sequence.budgetMs ?? 10000)
    const steps = Array.isArray(sequence.steps) ? sequence.steps : []
    for (let i = 0; i < steps.length; ++i) {
      const s   = steps[i]
      const row = root.defaults(String(s.kind ?? "wait"))
      row.ms        = Number(s.ms ?? row.ms)
      row.line      = String(s.line ?? row.line)
      row.high      = s.high !== false
      row.payload   = String(s.data ?? "")
      row.hex       = s.hex === true
      row.eol       = String(s.eol ?? "")
      row.checksum  = String(s.checksum ?? "")
      row.patterns  = Array.isArray(s.patterns) ? s.patterns.join(" | ") : ""
      row.timeoutMs = Number(s.timeoutMs ?? row.timeoutMs)
      row.optional  = s.optional === true
      row.rate      = Number(s.rate ?? row.rate)
      row.mode      = String(s.mode ?? row.mode)
      stepsModel.append(row)
    }

    root.refresh()
  }

  function splitPatterns(text) {
    return text.split("|").map(p => p.trim()).filter(p => p.length > 0)
  }

  function toSequence() {
    const steps = []
    for (let i = 0; i < stepsModel.count; ++i) {
      const r = stepsModel.get(i)
      const s = { kind: r.kind }
      if (r.kind === "break" || r.kind === "wait") {
        s.ms = r.ms
      } else if (r.kind === "line") {
        s.line = r.line
        s.high = r.high
      } else if (r.kind === "send") {
        s.data = r.payload
        s.hex = r.hex
        s.eol = r.eol
        s.checksum = r.checksum
      } else if (r.kind === "expect") {
        s.patterns = root.splitPatterns(r.patterns)
        s.timeoutMs = r.timeoutMs
        s.optional = r.optional
      } else if (r.kind === "baud") {
        s.rate = r.rate
      } else if (r.kind === "flow") {
        s.mode = r.mode
      }

      steps.push(s)
    }

    return { budgetMs: root.budgetMs, steps: steps }
  }

  function refresh() {
    const json = JSON.stringify(root.toSequence())
    root.verdict = Cpp_IO_Serial.validateConnectionSequence(json)
  }

  function setField(index, role, value) {
    if (index < 0 || index >= stepsModel.count)
      return

    stepsModel.setProperty(index, role, value)
    root.refresh()
  }

  function addStep() {
    stepsModel.append(root.defaults("wait"))
    root.refresh()
  }

  function removeStep(index) {
    stepsModel.remove(index)
    root.refresh()
  }

  function moveStep(index, delta) {
    const to = index + delta
    if (to < 0 || to >= stepsModel.count)
      return

    stepsModel.move(index, to, 1)
    root.refresh()
  }

  function kindIndex(slug) {
    return Math.max(0, root.kinds.findIndex(k => k.slug === slug))
  }

  function checksumName(index) {
    return index <= 0 ? "" : Cpp_Console_Handler.checksumMethods[index]
  }

  function checksumIndex(name) {
    return name.length === 0 ? 0 : Math.max(0, Cpp_Console_Handler.checksumMethods.indexOf(name))
  }

  //
  // Text field that follows its model value only while unfocused and commits on editing
  // finished; a numeric field clamps to [minimum, maximum] and reverts on anything else
  //
  component SyncField: Widgets.LineField {
    id: field

    property var modelValue
    property int minimum: 0
    property bool numeric: false
    property int maximum: 600000

    signal committed(var value)

    Layout.alignment: Qt.AlignVCenter
    font: Cpp_Misc_CommonFonts.monoFont
    Component.onCompleted: text = String(field.modelValue ?? "")
    inputMethodHints: field.numeric ? Qt.ImhDigitsOnly : Qt.ImhNone
    onModelValueChanged: {
      if (!activeFocus)
        text = String(field.modelValue ?? "")
    }
    onActiveFocusChanged: {
      if (!activeFocus)
        text = String(field.modelValue ?? "")
    }
    onEditingFinished: {
      if (!field.numeric) {
        field.committed(text)
        return
      }

      const v = parseInt(text)
      if (!isNaN(v))
        field.committed(Math.max(field.minimum, Math.min(field.maximum, v)))
    }
  }

  //--------------------------------------------------------------------------------------------
  // Layout
  //--------------------------------------------------------------------------------------------

  dialogContent: ColumnLayout {
    id: layout

    spacing: 12
    anchors.centerIn: parent
    Layout.minimumWidth: 840

    Label {
      wrapMode: Text.WordWrap
      Layout.fillWidth: true
      Layout.maximumWidth: 840
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Some devices need a few steps before they start sending data, such as a "
                 + "reset, a wake-up command, or a reply to wait for. These steps run in order "
                 + "every time Serial Studio connects. If one fails, the connection stops and "
                 + "the console says which step failed.")
    }

    //
    // Steps table
    //
    ColumnLayout {
      spacing: 4
      Layout.fillWidth: true
      Layout.fillHeight: true

      RowLayout {
        spacing: 8
        Layout.fillWidth: true

        Label {
          text: qsTr("Steps")
          font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
          color: Cpp_ThemeManager.colors["pane_section_label"]
          Component.onCompleted: font.capitalization = Font.AllUppercase
        }

        Item { Layout.fillWidth: true }

        Widgets.IconButton {
          objectName: "connectionSequenceEditor.addStep"

          iconSize: 14
          ToolTip.delay: 700
          text: qsTr("Add Step")
          ToolTip.visible: hovered
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/icons/buttons/plus.svg"
          ToolTip.text: qsTr("Append a step at the end of the sequence.")
          onClicked: root.addStep()
        }
      }

      Rectangle {
        radius: 2
        border.width: 1
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumWidth: 840
        Layout.minimumHeight: 280
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.color: Cpp_ThemeManager.colors["groupbox_border"]

        ListView {
          id: stepList

          spacing: 1
          clip: true
          model: stepsModel
          anchors.margins: 8
          anchors.fill: parent
          boundsBehavior: Flickable.StopAtBounds

          ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
          }

          delegate: Rectangle {
            id: stepRow

            required property int index
            required property int ms
            required property int rate
            required property bool hex
            required property bool high
            required property string eol
            required property string kind
            required property string line
            required property string mode
            required property int timeoutMs
            required property bool optional
            required property string payload
            required property string checksum
            required property string patterns

            radius: 3
            implicitHeight: 36
            width: ListView.view.width
            color: "transparent"
            border.width: stepRow.index === root.verdict.step ? 1 : 0
            border.color: Cpp_ThemeManager.colors["error"]

            RowLayout {
              anchors.fill: parent
              spacing: root.colSpacing
              anchors.rightMargin: root.colScrollSlot

              Label {
                text: String(stepRow.index + 1)
                font: Cpp_Misc_CommonFonts.monoFont
                horizontalAlignment: Text.AlignRight
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: root.colWIndex
                color: Cpp_ThemeManager.colors["placeholder_text"]
              }

              Widgets.Combo {
                objectName: "connectionSequenceEditor.stepKind"

                model: root.kindLabels
                font: Cpp_Misc_CommonFonts.uiFont
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: root.colWKind
                currentIndex: root.kindIndex(stepRow.kind)
                onActivated: (index) => root.setField(stepRow.index, "kind", root.kinds[index].slug)
              }

              Loader {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                sourceComponent: {
                  switch (stepRow.kind) {
                  case "break":
                  case "wait":
                    return msEditor
                  case "line":
                    return lineEditor
                  case "send":
                    return sendEditor
                  case "expect":
                    return expectEditor
                  case "baud":
                    return baudEditor
                  case "flow":
                    return flowEditor
                  default:
                    return discardEditor
                  }
                }
              }

              Widgets.IconButton {
                objectName: "connectionSequenceEditor.moveUp"

                padding: 2
                iconSize: 14
                rotation: 90
                ToolTip.delay: 700
                mirrorIconInRtl: false
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Move up.")
                enabled: stepRow.index > 0
                Layout.preferredHeight: 24
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: root.colWMove
                icon.source: "qrc:/icons/buttons/media-prev.svg"
                icon.color: enabled ? Cpp_ThemeManager.colors["text"]
                                    : Cpp_ThemeManager.colors["mid"]
                onClicked: root.moveStep(stepRow.index, -1)
              }

              Widgets.IconButton {
                objectName: "connectionSequenceEditor.moveDown"

                padding: 2
                iconSize: 14
                rotation: 90
                ToolTip.delay: 700
                mirrorIconInRtl: false
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Move down.")
                Layout.preferredHeight: 24
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: root.colWMove
                enabled: stepRow.index < stepsModel.count - 1
                icon.source: "qrc:/icons/buttons/media-next.svg"
                icon.color: enabled ? Cpp_ThemeManager.colors["text"]
                                    : Cpp_ThemeManager.colors["mid"]
                onClicked: root.moveStep(stepRow.index, 1)
              }

              Widgets.IconButton {
                objectName: "connectionSequenceEditor.removeStep"

                padding: 2
                iconSize: 14
                ToolTip.delay: 700
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Remove this step.")
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: root.colWDelete
                icon.source: "qrc:/icons/buttons/trash.svg"
                icon.color: Cpp_ThemeManager.colors["text"]
                onClicked: root.removeStep(stepRow.index)
              }
            }

            //
            // Send BREAK and Wait: a duration
            //
            Component {
              id: msEditor

              RowLayout {
                spacing: 4

                SyncField {
                  objectName: "connectionSequenceEditor.duration"

                  numeric: true
                  modelValue: stepRow.ms
                  Layout.preferredWidth: 90
                  minimum: stepRow.kind === "break" ? 1 : 0
                  maximum: stepRow.kind === "break" ? 10000 : 600000
                  onCommitted: (value) => root.setField(stepRow.index, "ms", value)
                }

                Label {
                  text: qsTr("ms")
                  Layout.fillWidth: true
                  color: Cpp_ThemeManager.colors["placeholder_text"]
                }
              }
            }

            //
            // Set Line: DTR or RTS, high or low
            //
            Component {
              id: lineEditor

              RowLayout {
                spacing: 8

                Widgets.Combo {
                  objectName: "connectionSequenceEditor.line"

                  model: ["DTR", "RTS"]
                  Layout.preferredWidth: 90
                  font: Cpp_Misc_CommonFonts.uiFont
                  currentIndex: stepRow.line === "rts" ? 1 : 0
                  onActivated: (index) => root.setField(stepRow.index, "line",
                                                        index === 1 ? "rts" : "dtr")
                }

                Widgets.Combo {
                  objectName: "connectionSequenceEditor.level"

                  Layout.preferredWidth: 90
                  font: Cpp_Misc_CommonFonts.uiFont
                  model: [qsTr("High"), qsTr("Low")]
                  currentIndex: stepRow.high ? 0 : 1
                  onActivated: (index) => root.setField(stepRow.index, "high", index === 0)
                }

                Item { Layout.fillWidth: true }
              }
            }

            //
            // Send Data: payload, hex flag, end of line and checksum
            //
            Component {
              id: sendEditor

              RowLayout {
                spacing: 8

                SyncField {
                  objectName: "connectionSequenceEditor.payload"

                  Layout.fillWidth: true
                  modelValue: stepRow.payload
                  placeholderText: stepRow.hex ? qsTr("e.g. 15 15") : qsTr("e.g. AT\\r")
                  onCommitted: (value) => root.setField(stepRow.index, "payload", value)
                }

                CheckBox {
                  objectName: "connectionSequenceEditor.hex"

                  text: qsTr("Hex")
                  checked: stepRow.hex
                  Layout.alignment: Qt.AlignVCenter
                  onToggled: root.setField(stepRow.index, "hex", checked)
                }

                Widgets.Combo {
                  objectName: "connectionSequenceEditor.lineEnding"

                  Layout.preferredWidth: 130
                  model: root.lineEndingLabels
                  font: Cpp_Misc_CommonFonts.uiFont
                  currentIndex: Math.max(0, root.lineEndings.findIndex(e => e.value === stepRow.eol))
                  onActivated: (index) => root.setField(stepRow.index, "eol",
                                                        root.lineEndings[index].value)
                }

                Widgets.Combo {
                  objectName: "connectionSequenceEditor.checksum"

                  Layout.preferredWidth: 120
                  font: Cpp_Misc_CommonFonts.uiFont
                  model: Cpp_Console_Handler.checksumMethods
                  currentIndex: root.checksumIndex(stepRow.checksum)
                  onActivated: (index) => root.setField(stepRow.index, "checksum",
                                                        root.checksumName(index))
                }
              }
            }

            //
            // Expect Reply: patterns, timeout and the optional flag
            //
            Component {
              id: expectEditor

              RowLayout {
                spacing: 8

                SyncField {
                  objectName: "connectionSequenceEditor.patterns"

                  Layout.fillWidth: true
                  modelValue: stepRow.patterns
                  ToolTip.delay: 500
                  ToolTip.visible: hovered
                  placeholderText: qsTr("Reply in hex, for example 06")
                  ToolTip.text: qsTr("The bytes the device should send back, in hex. Write ?? "
                                     + "for a byte that can be anything. To accept more than one "
                                     + "reply, separate them with a vertical bar, for example "
                                     + "06 | 15.")
                  onCommitted: (value) => root.setField(stepRow.index, "patterns", value)
                }

                Label {
                  text: qsTr("wait up to")
                  color: Cpp_ThemeManager.colors["placeholder_text"]
                }

                SyncField {
                  objectName: "connectionSequenceEditor.timeout"

                  minimum: 1
                  numeric: true
                  Layout.preferredWidth: 70
                  modelValue: stepRow.timeoutMs
                  onCommitted: (value) => root.setField(stepRow.index, "timeoutMs", value)
                }

                Label {
                  text: qsTr("ms")
                  color: Cpp_ThemeManager.colors["placeholder_text"]
                }

                CheckBox {
                  objectName: "connectionSequenceEditor.optional"

                  ToolTip.delay: 500
                  ToolTip.visible: hovered
                  checked: stepRow.optional
                  text: qsTr("Continue If No Reply")
                  ToolTip.text: qsTr("If no matching reply arrives in time, note it in the "
                                     + "console and go on to the next step instead of stopping "
                                     + "the connection.")
                  Layout.alignment: Qt.AlignVCenter
                  onToggled: root.setField(stepRow.index, "optional", checked)
                }
              }
            }

            //
            // Discard Input: no parameters
            //
            Component {
              id: discardEditor

              Label {
                elide: Text.ElideRight
                text: qsTr("Throws away anything the device has sent so far.")
                color: Cpp_ThemeManager.colors["placeholder_text"]
              }
            }

            //
            // Set Baud Rate: the running rate, never the saved one
            //
            Component {
              id: baudEditor

              RowLayout {
                spacing: 4

                SyncField {
                  objectName: "connectionSequenceEditor.baudRate"

                  minimum: 1
                  numeric: true
                  maximum: 15000000
                  modelValue: stepRow.rate
                  Layout.preferredWidth: 110
                  onCommitted: (value) => root.setField(stepRow.index, "rate", value)
                }

                Label {
                  Layout.fillWidth: true
                  text: qsTr("baud until disconnected")
                  color: Cpp_ThemeManager.colors["placeholder_text"]
                }
              }
            }

            //
            // Set Flow Control: the running mode
            //
            Component {
              id: flowEditor

              RowLayout {
                spacing: 8

                Widgets.Combo {
                  objectName: "connectionSequenceEditor.flowControl"

                  Layout.preferredWidth: 130
                  font: Cpp_Misc_CommonFonts.uiFont
                  model: Cpp_IO_Serial.flowControlList
                  currentIndex: Math.max(0, root.flowModes.indexOf(stepRow.mode))
                  onActivated: (index) => root.setField(stepRow.index, "mode",
                                                        root.flowModes[index])
                }

                Item { Layout.fillWidth: true }
              }
            }
          }

          //
          // Empty-state hint
          //
          Label {
            anchors.centerIn: parent
            visible: stepsModel.count === 0
            font: Cpp_Misc_CommonFonts.uiFont
            color: Cpp_ThemeManager.colors["placeholder_text"]
            text: qsTr("No steps. Serial Studio starts reading data as soon as the port opens.")
          }
        }
      }
    }

    //
    // Validation banner
    //
    Label {
      wrapMode: Text.WordWrap
      Layout.fillWidth: true
      Layout.maximumWidth: 840
      visible: root.verdict.message.length > 0
      color: Cpp_ThemeManager.colors["error"]
      text: root.verdict.step >= 0
            ? qsTr("Step %1: %2").arg(root.verdict.step + 1).arg(root.verdict.message)
            : root.verdict.message
    }

    //
    // Footer: time limit, Cancel, Apply
    //
    RowLayout {
      spacing: 8
      Layout.topMargin: 4
      Layout.fillWidth: true

      Label {
        ToolTip.delay: 500
        text: qsTr("Time Limit") + ":"
        Layout.alignment: Qt.AlignVCenter
        ToolTip.visible: _limitArea.containsMouse
        ToolTip.text: qsTr("All steps must finish within this time, or the connection stops.")

        MouseArea {
          id: _limitArea

          hoverEnabled: true
          anchors.fill: parent
          acceptedButtons: Qt.NoButton
        }
      }

      SyncField {
        objectName: "connectionSequenceEditor.budget"

        minimum: 1
        numeric: true
        modelValue: root.budgetMs
        Layout.preferredWidth: 90
        onCommitted: (value) => {
          root.budgetMs = value
          root.refresh()
        }
      }

      Label {
        text: qsTr("ms")
        Layout.alignment: Qt.AlignVCenter
        color: Cpp_ThemeManager.colors["placeholder_text"]
      }

      Item { Layout.fillWidth: true }

      Widgets.IconButton {
        objectName: "connectionSequenceEditor.cancel"

        iconSize: 16
        ToolTip.delay: 700
        text: qsTr("Cancel")
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Discard changes.")
        icon.source: "qrc:/icons/buttons/close.svg"
        onClicked: root.close()
      }

      Widgets.IconButton {
        objectName: "connectionSequenceEditor.apply"

        iconSize: 16
        highlighted: true
        ToolTip.delay: 700
        text: qsTr("Apply")
        ToolTip.visible: hovered
        enabled: root.verdict.message.length === 0
        icon.source: "qrc:/icons/buttons/apply.svg"
        ToolTip.text: qsTr("Apply the sequence; it runs on the next connection.")
        onClicked: {
          forceActiveFocus()
          root.applied(JSON.stringify(root.toSequence()))
          root.close()
        }
      }
    }
  }
}
