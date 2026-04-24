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
      text: root.model ? root.model.title : ""
      color: root.color
      font: Cpp_Misc_CommonFonts.boldUiFont
      Layout.alignment: Qt.AlignHCenter
    }

    RowLayout {
      spacing: 8
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignHCenter

      Label {
        text: root.model ? root.model.minValue.toFixed(1) : "0"
        color: root.color
        font: Cpp_Misc_CommonFonts.uiFont
      }

      Slider {
        id: slider

        Layout.fillWidth: true
        from: root.model ? root.model.minValue : 0
        to: root.model ? root.model.maxValue : 100
        stepSize: root.model ? root.model.stepSize : 1

        palette.dark: root.color
        palette.highlight: root.color

        onMoved: {
          if (root.model)
            root.model.currentValue = slider.value
        }
      }

      Label {
        text: root.model ? root.model.maxValue.toFixed(1) : "100"
        color: root.color
        font: Cpp_Misc_CommonFonts.uiFont
      }
    }

    Label {
      text: {
        if (!root.model)
          return ""

        var val = slider.value.toFixed(2)
        var units = root.model.units
        return val + (units.length > 0 ? " " + units : "")
      }
      color: root.color
      Layout.alignment: Qt.AlignHCenter
      font: Cpp_Misc_CommonFonts.boldUiFont
    }

    Label {
      color: "#ff6666"
      font: Cpp_Misc_CommonFonts.uiFont
      Layout.alignment: Qt.AlignHCenter
      text: qsTr("No transmit function defined")
      visible: root.model && !root.model.hasTransmitFunction
    }

    Item { Layout.fillHeight: true }
  }
}
