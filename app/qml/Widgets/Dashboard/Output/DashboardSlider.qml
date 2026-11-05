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

OutputValueControl {
  id: root

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

      Label {
        text: root.from.toFixed(0)
        color: Cpp_ThemeManager.colors["placeholder_text"]
        font: Cpp_Misc_CommonFonts.customUiFont(0.7, false)
      }

      Slider {
        id: control

        to: root.to
        from: root.from
        stepSize: root.step
        Layout.fillWidth: true
        value: root.desiredValue
        opacity: root.unknown ? 0.45 : 1.0

        objectName: root.idPrefix.length > 0 ? root.idPrefix + ".slider" : ""

        Keys.onPressed: (event) => root.handleKey(event)

        WheelHandler {
          enabled: control.activeFocus
          acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
          onWheel: (event) => root.nudge(event.angleDelta.y > 0 ? 1 : -1)
        }

        palette.dark: root.color
        palette.highlight: root.color

        onMoved: root.commitMove(control.value)
        onPressedChanged: root.trackPress(control)
      }

      Label {
        text: root.to.toFixed(0)
        color: Cpp_ThemeManager.colors["placeholder_text"]
        font: Cpp_Misc_CommonFonts.customUiFont(0.7, false)
      }
    }

    Label {
      color: root.color
      text: root.valueText
      Layout.alignment: Qt.AlignHCenter
      font: Cpp_Misc_CommonFonts.customMonoFont(0.8, true)
    }
  }
}
