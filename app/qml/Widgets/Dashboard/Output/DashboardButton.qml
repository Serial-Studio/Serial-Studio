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
    anchors.margins: 16
    anchors.fill: parent

    Item { Layout.fillHeight: true }

    Label {
      color: root.color
      Layout.alignment: Qt.AlignHCenter
      font: Cpp_Misc_CommonFonts.boldUiFont
      text: root.model ? root.model.title : ""
      visible: root.model && root.model.title.length > 0
    }

    Button {
      text: (root.model && root.model.title.length > 0)
            ? root.model.title : qsTr("Send")
      Layout.alignment: Qt.AlignHCenter
      Layout.preferredWidth: Math.min(parent.width * 0.6, 200)
      Layout.preferredHeight: 40

      palette.button: root.color
      palette.buttonText: {
        var r = root.color.r
        var g = root.color.g
        var b = root.color.b
        var luminance = 0.299 * r + 0.587 * g + 0.114 * b
        return luminance > 0.5 ? "#000000" : "#ffffff"
      }

      onClicked: {
        if (root.model)
          root.model.click()
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
