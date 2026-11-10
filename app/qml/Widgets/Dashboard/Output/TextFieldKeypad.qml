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

import "../.." as Widgets

//
// Keypad entry (spec 0118): a numeric keypad anchored to a number field. Keys edit a local
// entry through the control's rules; only Enter hands the entry back to be sent.
//
Popup {
  id: root

  property var model: null
  property string entry: ""
  property real textScale: 1
  property string idPrefix: ""

  signal entered(string text)

  readonly property real key: Math.round(40 * root.textScale)
  readonly property bool sign: root.model ? root.model.keypadSign : false
  readonly property bool point: root.model ? root.model.keypadDecimal : false

  //
  // Applies one key through the control, which owns the entry rules
  //
  function press(name) {
    if (root.model)
      root.entry = root.model.keypadApply(root.entry, name)
  }

  padding: 8
  focus: true
  modal: false
  closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

  enter: Widgets.PopupEnter {}
  exit: Widgets.PopupExit {}

  background: Widgets.MenuSurface {}

  contentItem: ColumnLayout {
    spacing: 6

    Label {
      Layout.fillWidth: true
      horizontalAlignment: Text.AlignRight
      text: root.entry.length > 0 ? root.entry : "0"
      color: Cpp_ThemeManager.colors["start_menu_text"]
      font: Cpp_Misc_CommonFonts.customMonoFont(1.6 * root.textScale, true)
    }

    GridLayout {
      columns: 4
      rowSpacing: 4
      columnSpacing: 4

      Repeater {
        model: [
          {name: "7", label: "7"}, {name: "8", label: "8"}, {name: "9", label: "9"},
          {name: "<", label: String.fromCharCode(0x232B), id: "back"},
          {name: "4", label: "4"}, {name: "5", label: "5"}, {name: "6", label: "6"},
          {name: "C", label: "C", id: "clear"},
          {name: "1", label: "1"}, {name: "2", label: "2"}, {name: "3", label: "3"},
          {name: "-", label: String.fromCharCode(0xB1), id: "sign"},
          {name: "0", label: "0"}, {name: ".", label: ".", id: "point"}
        ]

        delegate: Button {
          required property var modelData

          readonly property string keyId: modelData.id !== undefined ? modelData.id
                                                                     : modelData.name

          text: modelData.label
          opacity: enabled ? 1 : 0
          Layout.preferredWidth: root.key
          Layout.preferredHeight: root.key
          font: Cpp_Misc_CommonFonts.customUiFont(1.1 * root.textScale, true)
          enabled: (keyId !== "sign" || root.sign) && (keyId !== "point" || root.point)
          objectName: root.idPrefix.length > 0 ? root.idPrefix + ".keypad." + keyId : ""

          onClicked: root.press(modelData.name)
        }
      }

      Button {
        text: qsTr("Cancel")
        Layout.preferredWidth: root.key
        Layout.preferredHeight: root.key
        onClicked: root.close()
        font: Cpp_Misc_CommonFonts.customUiFont(0.8 * root.textScale, false)
        objectName: root.idPrefix.length > 0 ? root.idPrefix + ".keypad.cancel" : ""
      }

      Button {
        highlighted: true
        text: qsTr("Enter")
        Layout.preferredWidth: root.key
        Layout.preferredHeight: root.key
        font: Cpp_Misc_CommonFonts.customUiFont(0.8 * root.textScale, true)
        objectName: root.idPrefix.length > 0 ? root.idPrefix + ".keypad.enter" : ""
        onClicked: {
          root.entered(root.entry)
          root.close()
        }
      }
    }
  }
}
