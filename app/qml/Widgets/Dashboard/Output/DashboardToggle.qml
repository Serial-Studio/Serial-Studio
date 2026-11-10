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

  property string idPrefix: ""

  readonly property string title: root.widget ? (root.widget.title || "") : ""
  readonly property bool lever: Cpp_ThemeManager.widgetLooks["output"] !== "plain"
                                && root.height >= 90

  //
  // Asks the model for the opposite of what the control shows; feedback, not the press, moves it
  //
  function flip() {
    if (!root.model)
      return

    Cpp_UI_Alarms.playEvent("toggle")
    root.model.checked = !root.desiredChecked
  }
  readonly property bool unknown: root.model
                                  ? (root.model.stateBound && !root.model.stateKnown)
                                  : false

  //
  // Restorable after a toggle clears the binding; never initialValue, which cannot track
  //
  readonly property bool desiredChecked: {
    if (!root.model)
      return false

    if (root.model.stateBound)
      return root.model.stateKnown && root.model.stateOn

    return root.model.checked === true
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

    Item {
      visible: !root.lever
      Layout.fillHeight: true
    }

    //
    // Lever switch: the hardware look of a toggle with room for it, unless the theme asks for
    // plain; the switch otherwise
    //
    Loader {
      active: root.lever
      visible: root.lever
      Layout.fillWidth: true
      Layout.fillHeight: true

      sourceComponent: HardwareLever {
        dimmed: root.unknown
        on: root.desiredChecked
        onLabel: root.widget ? (root.widget.onLabel || "") : ""
        offLabel: root.widget ? (root.widget.offLabel || "") : ""
        objectName: root.idPrefix.length > 0 ? root.idPrefix + ".toggle" : ""

        readonly property bool checked: root.desiredChecked

        onToggled: root.flip()
      }
    }

    Switch {
      id: control

      visible: !root.lever
      checked: root.desiredChecked
      Layout.alignment: Qt.AlignHCenter
      opacity: root.unknown ? 0.45 : 1.0

      objectName: !root.lever && root.idPrefix.length > 0 ? root.idPrefix + ".toggle" : ""

      palette.highlight: root.color

      onToggled: {
        root.flip()
        control.checked = Qt.binding(function() { return root.desiredChecked })
      }
    }

    Item {
      visible: !root.lever
      Layout.fillHeight: true
    }
  }
}
