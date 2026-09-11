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
  required property var model
  required property var widget

  readonly property real from: root.widget ? (root.widget.minValue || 0) : 0
  readonly property real to: root.widget ? (root.widget.maxValue || 100) : 100
  readonly property real step: root.widget ? (root.widget.stepSize || 1) : 1
  readonly property string title: root.widget ? (root.widget.title || "") : ""
  readonly property bool unknown: root.model
                                  ? (root.model.stateBound && !root.model.stateKnown)
                                  : false

  //
  // Restorable after a drag clears the binding, so feedback can move the handle again
  //
  readonly property real desiredValue: root.model
                                       && root.model.currentValue !== undefined
                                       ? root.model.currentValue
                                       : root.from

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

        palette.dark: root.color
        palette.highlight: root.color

        onMoved: {
          if (root.model)
            root.model.currentValue = control.value
        }

        onPressedChanged: {
          if (!root.model)
            return

          if (control.pressed) {
            root.model.beginInteraction()
            return
          }

          root.model.endInteraction()
          control.value = Qt.binding(function() { return root.desiredValue })
        }
      }

      Label {
        text: root.to.toFixed(0)
        color: Cpp_ThemeManager.colors["placeholder_text"]
        font: Cpp_Misc_CommonFonts.customUiFont(0.7, false)
      }
    }

    Label {
      color: root.color
      Layout.alignment: Qt.AlignHCenter
      text: control.value.toFixed(root.step < 1 ? 2 : 0)
      font: Cpp_Misc_CommonFonts.customMonoFont(0.8, true)
    }
  }
}
