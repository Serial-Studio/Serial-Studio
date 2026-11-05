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

    Dial {
      id: control

      to: root.to
      from: root.from
      stepSize: root.step
      Layout.fillWidth: true
      Layout.fillHeight: true
      value: root.desiredValue
      inputMode: Dial.Circular
      Layout.maximumWidth: height
      Layout.alignment: Qt.AlignHCenter
      opacity: root.unknown ? 0.45 : 1.0

      objectName: root.idPrefix.length > 0 ? root.idPrefix + ".knob" : ""

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

      background: Rectangle {
        opacity: 0.5
        height: width
        border.width: 2
        radius: width / 2
        color: "transparent"
        border.color: root.color
        x: control.width / 2 - width / 2
        y: control.height / 2 - height / 2
        width: Math.min(control.availableWidth, control.availableHeight)
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
