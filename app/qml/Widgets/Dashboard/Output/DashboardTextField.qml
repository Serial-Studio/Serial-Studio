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
  required property var windowRoot
  required property var model
  required property string widgetId

  ColumnLayout {
    spacing: 8
    anchors.margins: 16
    anchors.fill: parent

    Item { Layout.fillHeight: true }

    Label {
      color: root.color
      Layout.alignment: Qt.AlignHCenter
      font: Cpp_Misc_CommonFonts.boldUiFont
      text: root.model ? root.model.title : ""
    }

    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      Widgets.LineField {
        id: input

        Layout.fillWidth: true
        font: Cpp_Misc_CommonFonts.monoFont
        placeholderText: qsTr("Enter command…")

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
      color: Cpp_ThemeManager.colors["error"]
      font: Cpp_Misc_CommonFonts.uiFont
      Layout.alignment: Qt.AlignHCenter
      text: qsTr("No transmit function defined")
      visible: root.model && !root.model.hasTransmitFunction
    }

    Item { Layout.fillHeight: true }
  }
}
