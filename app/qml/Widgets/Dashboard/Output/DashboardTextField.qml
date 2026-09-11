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

  readonly property string title: root.widget ? (root.widget.title || "") : ""

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
        placeholderText: qsTr("Enter command…")
        font: Cpp_Misc_CommonFonts.customMonoFont(0.8, false)

        palette.highlight: root.color

        onAccepted: sendButton.clicked()
      }

      Widgets.IconButton {
        id: sendButton

        iconSize: 16
        text: qsTr("Send")
        icon.source: "qrc:/icons/buttons/send.svg"
        font: Cpp_Misc_CommonFonts.customUiFont(0.8, false)

        palette.button: root.color
        palette.buttonText: Cpp_ThemeManager.colors["highlighted_text"]

        onClicked: {
          if (root.model && input.text.length > 0) {
            root.model.sendText(input.text)
            input.clear()
          }
        }
      }
    }
  }
}
