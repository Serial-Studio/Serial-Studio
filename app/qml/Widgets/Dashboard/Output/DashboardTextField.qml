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

Item {
  id: root

  required property color color
  required property var windowRoot
  required property var model
  required property string widgetId

  ColumnLayout {
    spacing: 8
    anchors.fill: parent
    anchors.margins: 16

    Item { Layout.fillHeight: true }

    Label {
      text: root.model ? root.model.title : ""
      color: root.color
      font: Cpp_Misc_CommonFonts.boldUiFont
      Layout.alignment: Qt.AlignHCenter
    }

    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      TextField {
        id: input

        placeholderText: qsTr("Enter command...")
        font: Cpp_Misc_CommonFonts.monoFont
        Layout.fillWidth: true

        palette.highlight: root.color

        onAccepted: sendButton.clicked()
      }

      Button {
        id: sendButton
        text: qsTr("Send")
        Layout.preferredWidth: 80

        palette.button: root.color
        palette.buttonText: "white"

        onClicked: {
          if (root.model && input.text.length > 0) {
            root.model.sendText(input.text)
            input.clear()
          }
        }
      }
    }

    Label {
      visible: root.model && !root.model.hasTransmitFunction
      text: qsTr("No transmit function defined")
      color: "#ff6666"
      font: Cpp_Misc_CommonFonts.uiFont
      Layout.alignment: Qt.AlignHCenter
    }

    Item { Layout.fillHeight: true }
  }
}
