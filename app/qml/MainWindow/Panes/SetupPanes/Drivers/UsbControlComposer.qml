/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../../../../Widgets" as Widgets

ColumnLayout {
  id: root

  spacing: 4

  //
  // Local state: derived from the request-type field
  //
  property bool pending: false
  readonly property bool validType: !isNaN(requestType)
  readonly property int requestType: hexToInt(typeField.text)
  readonly property bool isIn: validType && (requestType & 0x80) !== 0

  //
  // Parses a hex string (optional 0x prefix) to an int, or NaN when malformed
  //
  function hexToInt(text) {
    if (!text)
      return NaN

    let s = text.trim()
    if (s.toLowerCase().startsWith("0x"))
      s = s.substring(2)

    if (!/^[0-9a-fA-F]+$/.test(s))
      return NaN

    return parseInt(s, 16)
  }

  //
  // Title
  //
  Label {
    text: qsTr("Control Transfer")
    font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)
  }

  //
  // Setup-packet fields
  //
  GridLayout {
    columns: 2
    rowSpacing: 4
    columnSpacing: 4
    Layout.fillWidth: true

    Label {
      text: qsTr("Request Type") + ":"
    } Widgets.LineField {
      id: typeField

      text: "0x41"
      Layout.fillWidth: true
      placeholderText: "0x41"
      font: Cpp_Misc_CommonFonts.customMonoFont(1, false)
    }

    Label {
      text: qsTr("Request") + ":"
    } Widgets.LineField {
      id: requestField

      Layout.fillWidth: true
      placeholderText: "0x00"
      font: Cpp_Misc_CommonFonts.customMonoFont(1, false)
    }

    Label {
      text: qsTr("wValue") + ":"
    } Widgets.LineField {
      id: valueField

      text: "0x0000"
      Layout.fillWidth: true
      placeholderText: "0x0000"
      font: Cpp_Misc_CommonFonts.customMonoFont(1, false)
    }

    Label {
      text: qsTr("wIndex") + ":"
    } Widgets.LineField {
      id: indexField

      text: "0x0000"
      Layout.fillWidth: true
      placeholderText: "0x0000"
      font: Cpp_Misc_CommonFonts.customMonoFont(1, false)
    }

    Label {
      text: qsTr("Direction") + ":"
    } Label {
      opacity: 0.85
      Layout.fillWidth: true
      text: root.isIn ? qsTr("IN · device → host") : qsTr("OUT · host → device")
    }

    Label {
      visible: !root.isIn
      text: qsTr("Data") + ":"
    } Widgets.LineField {
      id: payloadField

      visible: !root.isIn
      Layout.fillWidth: true
      placeholderText: "EF BE 00 00"
      font: Cpp_Misc_CommonFonts.customMonoFont(1, false)
    }

    Label {
      visible: root.isIn
      text: qsTr("Read Length") + ":"
    } SpinBox {
      id: lengthSpin

      from: 0
      to: 4096
      value: 64
      editable: true
      visible: root.isIn
      Layout.fillWidth: true
    }
  }

  //
  // Send button
  //
  Button {
    text: qsTr("Send Control Transfer")
    Layout.alignment: Qt.AlignRight
    enabled: !root.pending && typeField.text.length > 0 && requestField.text.length > 0
             && valueField.text.length > 0 && indexField.text.length > 0

    onClicked: {
      root.pending = true
      responseArea.text = qsTr("Sending…")
      Cpp_IO_USB.sendControlRequest(typeField.text, requestField.text, valueField.text,
                                    indexField.text, payloadField.text, lengthSpin.value)
    }
  }

  //
  // Response / result area
  //
  Rectangle {
    Layout.fillWidth: true
    Layout.preferredHeight: 80
    color: Cpp_ThemeManager.colors["base"]
    border.color: Cpp_ThemeManager.colors["groupbox_border"]

    ScrollView {
      clip: true
      anchors.margins: 4
      anchors.fill: parent

      TextArea {
        id: responseArea

        readOnly: true
        selectByMouse: true
        wrapMode: TextArea.WrapAnywhere
        color: Cpp_ThemeManager.colors["text"]
        font: Cpp_Misc_CommonFonts.customMonoFont(1, false)
        placeholderText: qsTr("Transfer result appears here.")
      }
    }
  }

  //
  // Result signal from the driver (queued from the libusb event thread)
  //
  Connections {
    target: Cpp_IO_USB

    function onControlTransferFinished(ok, bytesTransferred, responseHex, message) {
      root.pending = false
      let out = message
      if (ok && responseHex.length > 0)
        out += "\n\n" + responseHex

      responseArea.text = out
    }
  }
}
