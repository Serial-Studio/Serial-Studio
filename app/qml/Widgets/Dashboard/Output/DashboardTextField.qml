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
  readonly property bool numeric: root.widget ? root.widget.inputKind === 1 : false
  readonly property string units: root.widget ? (root.widget.units || "") : ""
  readonly property string rangeMessage: root.model && root.numeric ? root.model.rangeMessage : ""
  readonly property string rangeHint: root.model && root.numeric
                                      ? root.model.formatted(root.widget.minValue)
                                        + " - " + root.model.formatted(root.widget.maxValue)
                                        + (root.units.length > 0 ? " " + root.units : "")
                                      : ""

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
      input.clear()
      return
    }

    const sent = root.typedNumber()
    if (root.model.sendText(input.text))
      input.text = root.model.formatted(root.model.admittedValue !== undefined
                                        ? root.model.admittedValue : sent)
  }

  ColumnLayout {
    spacing: 4
    anchors.margins: 8
    anchors.fill: parent

    OutputSectionLabel {
      text: root.title
      labelColor: root.color
    }

    RowLayout {
      spacing: 4
      Layout.fillWidth: true

      Widgets.LineField {
        id: input

        Layout.fillWidth: true
        font: Cpp_Misc_CommonFonts.customMonoFont(0.8, false)
        objectName: root.idPrefix.length > 0 ? root.idPrefix + ".input" : ""
        placeholderText: root.numeric ? root.rangeHint : qsTr("Enter command…")
        inputMethodHints: root.numeric ? Qt.ImhFormattedNumbersOnly : Qt.ImhNone

        palette.highlight: root.color

        onAccepted: root.send()

        Keys.onPressed: (event) => {
          if (!root.numeric)
            return

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
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)
      }

      Widgets.IconButton {
        id: sendButton

        iconSize: 16
        text: qsTr("Send")
        icon.source: "qrc:/icons/buttons/send.svg"
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)

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
      font: Cpp_Misc_CommonFonts.customUiFont(0.75, false)
      objectName: root.idPrefix.length > 0 ? root.idPrefix + ".outOfRange" : ""
    }
  }
}
