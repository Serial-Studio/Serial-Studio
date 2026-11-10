/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../.." as Widgets

Item {
  id: root

  required property color color
  required property var model
  required property var widget

  property string idPrefix: ""

  readonly property string title: root.widget ? (root.widget.title || "") : ""

  //
  // Style drawn and size class (spec 0118): the C++ side already resolved the kind rule
  //
  readonly property bool keypad: root.style === 5
  readonly property bool commandLine: root.style === 6

  property int recall: -1

  //
  // Command line (spec 0118 R4): Up steps back through the sent lines, Down forward to an empty
  // line. A recall fills the field and never sends.
  //
  function recallStep(delta) {
    if (!root.model)
      return

    const last  = root.model.historyCount - 1
    root.recall = Math.max(-1, Math.min(last, root.recall + delta))
    input.text  = root.recall < 0 ? "" : root.model.historyAt(root.recall)
  }
  readonly property bool compact: root.style === 7
  readonly property bool readout: root.style === 4
  readonly property real scale: root.widget ? (root.widget.sizeScale || 1) : 1
  readonly property int style: root.widget ? (root.widget.effectiveStyle || 0) : 0

  readonly property bool unknown: root.model
                                  ? (root.model.stateBound && !root.model.stateKnown)
                                  : false

  property bool editing: false
  property bool sentOnce: false

  //
  // The readout's value: what the source reports when bound, else the last value admitted
  //
  readonly property string readoutText: {
    if (!root.model)
      return ""

    if (root.model.stateBound)
      return root.model.stateKnown ? root.model.formatted(root.model.stateValue) : ""

    return root.sentOnce && isFinite(root.model.admittedValue)
           ? root.model.formatted(root.model.admittedValue) : ""
  }

  //
  // Opens the readout for typing, starting from the value it shows
  //
  function beginEdit() {
    if (!root.model)
      return

    root.editing = true
    input.text = root.readoutText
    input.forceActiveFocus()
    input.selectAll()
  }
  readonly property bool numeric: root.widget ? root.widget.inputKind === 1 : false
  readonly property string units: root.widget ? (root.widget.units || "") : ""
  readonly property string rangeMessage: root.model && root.numeric ? root.model.rangeMessage : ""
  readonly property string rangeHint: root.model && root.numeric
                                      ? root.model.formatted(root.widget.minValue)
                                        + " - " + root.model.formatted(root.widget.maxValue)
                                        + (root.units.length > 0 ? " " + root.units : "")
                                      : ""

  //
  // Dress of the field (spec 0113): placeholder, send caption and icon come from the control's
  // off label, on label and icon; a Number field with zero decimals takes whole numbers only
  //
  readonly property string hint: root.widget ? (root.widget.offLabel || "") : ""
  readonly property string sendIcon: root.widget ? (root.widget.icon || "") : ""
  readonly property bool wholeNumbers: root.numeric && root.widget.decimals === 0
  readonly property string caption: root.widget ? (root.widget.onLabel || "") : ""

  //
  // Parses the typed text the way the C++ rule does for a plain number; an empty or unreadable
  // field counts from Min. The control, not this file, decides what is in range.
  //
  function typedNumber() {
    const parsed = Number(input.text.trim().replace(",", "."))
    return input.text.trim().length > 0 && isFinite(parsed) ? parsed : root.widget.minValue
  }

  function nudge(steps) {
    if (!root.model || !root.numeric)
      return

    input.text = root.model.formatted(root.model.stepped(root.typedNumber(), steps))
  }

  function send() {
    if (!root.model || input.text.length === 0)
      return

    if (!root.numeric) {
      root.model.sendText(input.text)
      root.recall = -1
      input.clear()
      return
    }

    const sent = root.typedNumber()
    if (!root.model.sendText(input.text))
      return

    root.sentOnce = true
    input.text    = root.model.formatted(root.model.admittedValue !== undefined
                                         ? root.model.admittedValue : sent)
  }

  ColumnLayout {
    spacing: 4
    anchors.margins: 8
    anchors.fill: parent

    OutputSectionLabel {
      text: root.title
      textScale: root.scale
      labelColor: root.color
      visible: !(root.widget && root.widget.hideTitle === true)
    }

    Loader {
      Layout.fillWidth: true
      active: root.readout && !root.editing
      visible: root.readout && !root.editing

      sourceComponent: TextFieldReadout {
        units: root.units
        accent: root.color
        dimmed: root.unknown
        textScale: root.scale
        text: root.readoutText
        objectName: root.idPrefix.length > 0 ? root.idPrefix + ".readout" : ""

        onActivated: root.beginEdit()
      }
    }

    RowLayout {
      spacing: 4
      Layout.fillWidth: true
      visible: !root.readout || root.editing

      Widgets.LineField {
        id: input

        Layout.fillWidth: true
        font: Cpp_Misc_CommonFonts.customMonoFont(0.8 * root.scale, false)
        objectName: root.idPrefix.length > 0 ? root.idPrefix + ".input" : ""
        placeholderText: {
          if (root.hint.length > 0)
            return root.hint

          return root.numeric ? root.rangeHint : qsTr("Enter command…")
        }
        validator: RegularExpressionValidator {
          regularExpression: {
            if (!root.numeric)
              return /.*/

            return root.wholeNumbers ? /^-?[0-9]*$/ : /^-?[0-9]*[.,]?[0-9]*$/
          }
        }
        inputMethodHints: root.numeric ? Qt.ImhFormattedNumbersOnly : Qt.ImhNone

        palette.highlight: root.color
        leftPadding: root.commandLine ? promptMark.implicitWidth + 10 : 4
        palette.text: root.commandLine ? Qt.rgba(0.82, 0.92, 0.84, 1) : root.palette.text
        palette.base: root.commandLine ? Qt.rgba(0.06, 0.07, 0.08, 1) : root.palette.base

        Text {
          id: promptMark

          x: 6
          visible: root.commandLine
          text: String.fromCharCode(0x203A)
          color: Qt.rgba(0.47, 0.86, 0.55, 1)
          anchors.verticalCenter: parent.verticalCenter
          font: Cpp_Misc_CommonFonts.customMonoFont(0.9 * root.scale, true)
        }

        onAccepted: {
          root.send()
          if (root.readout && root.rangeMessage.length === 0)
            root.editing = false
        }
        Keys.onEscapePressed: {
          if (root.readout)
            root.editing = false
        }

        TapHandler {
          enabled: root.keypad && root.enabled && Cpp_IO_Manager.isConnected
          onTapped: {
            keypadPopup.entry = ""
            keypadPopup.open()
          }
        }

        TextFieldKeypad {
          id: keypadPopup

          model: root.model
          y: input.height + 4
          textScale: root.scale
          idPrefix: root.idPrefix

          onEntered: (text) => {
            input.text = text
            root.send()
          }
        }

        Keys.onPressed: (event) => {
          if (!root.numeric) {
            if (!root.commandLine || (event.key !== Qt.Key_Up && event.key !== Qt.Key_Down))
              return

            root.recallStep(event.key === Qt.Key_Up ? 1 : -1)
            event.accepted = true
            return
          }

          const factor = (event.modifiers & Qt.ShiftModifier) ? 10 : 1
          if (event.key === Qt.Key_Up)
            root.nudge(factor)
          else if (event.key === Qt.Key_Down)
            root.nudge(-factor)
          else
            return

          event.accepted = true
        }

        WheelHandler {
          enabled: root.numeric && input.activeFocus
          acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
          onWheel: (event) => root.nudge(event.angleDelta.y > 0 ? 1 : -1)
        }
      }

      Label {
        visible: root.numeric && root.units.length > 0
        text: root.units
        color: Cpp_ThemeManager.colors["placeholder_text"]
        font: Cpp_Misc_CommonFonts.customUiFont(0.8 * root.scale, false)
      }

      Widgets.IconButton {
        id: sendButton

        visible: !root.compact
        iconSize: Math.round(16 * root.scale)
        text: root.caption.length > 0 ? root.caption : qsTr("Send")
        font: Cpp_Misc_CommonFonts.customUiFont(0.8 * root.scale, false)
        icon.source: root.sendIcon.length > 0 ? root.sendIcon : "qrc:/icons/buttons/send.svg"

        palette.button: root.color
        palette.buttonText: Cpp_ThemeManager.colors["highlighted_text"]

        objectName: root.idPrefix.length > 0 ? root.idPrefix + ".send" : ""

        onClicked: root.send()
      }
    }

    Label {
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      text: root.rangeMessage
      visible: root.rangeMessage.length > 0
      color: Cpp_ThemeManager.colors["alarm"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.75 * root.scale, false)
      objectName: root.idPrefix.length > 0 ? root.idPrefix + ".outOfRange" : ""
    }
  }
}
