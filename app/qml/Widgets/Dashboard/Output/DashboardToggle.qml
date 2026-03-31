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

    Switch {
      id: toggle

      checked: root.model ? root.model.checked : false
      Layout.alignment: Qt.AlignHCenter

      palette.highlight: root.color

      onToggled: {
        if (root.model)
          root.model.checked = toggle.checked
      }
    }

    Label {
      text: {
        if (!root.model)
          return ""

        if (toggle.checked)
          return root.model.onLabel || qsTr("ON")

        return root.model.offLabel || qsTr("OFF")
      }
      color: root.color
      font: Cpp_Misc_CommonFonts.uiFont
      Layout.alignment: Qt.AlignHCenter
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
