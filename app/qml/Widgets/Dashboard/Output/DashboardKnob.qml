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

  //
  // Selector style (spec 0113): a labelled rotary switch in place of the dial
  //
  readonly property bool selector: root.widget ? root.widget.style === 3 : false
  readonly property var positionLabels: {
    const text = root.widget && root.widget.labels ? root.widget.labels : ""
    if (text.length === 0)
      return []

    return text.split(",").map((label) => label.trim())
  }

  ColumnLayout {
    spacing: 4
    anchors.margins: 8
    anchors.fill: parent

    OutputSectionLabel {
      text: root.title
      labelColor: root.color
      visible: !(root.widget && root.widget.hideTitle === true)
    }

    Loader {
      active: root.selector
      Layout.fillWidth: true
      visible: root.selector
      Layout.fillHeight: true

      sourceComponent: HardwareSelector {
        id: selectorPart

        to: root.to
        from: root.from
        step: root.step
        dimmed: root.unknown
        value: root.desiredValue
        labels: root.positionLabels
        objectName: root.idPrefix.length > 0 ? root.idPrefix + ".knob" : ""
        springReturn: root.widget ? root.widget.springReturn === true : false

        onMoved: (value) => root.commitMove(value)
        onReleased: {
          if (root.model)
            root.model.release()
        }
        onPressedChanged: {
          if (selectorPart.springReturn)
            root.trackPress(selectorPart)
        }
      }
    }

    Dial {
      id: control

      visible: !root.selector

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

      objectName: !root.selector && root.idPrefix.length > 0 ? root.idPrefix + ".knob" : ""

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
      visible: !root.selector
      Layout.alignment: Qt.AlignHCenter
      font: Cpp_Misc_CommonFonts.customMonoFont(0.8, true)
    }
  }
}
