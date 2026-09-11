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

  readonly property string title: root.widget ? (root.widget.title || "") : ""
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
    }

    Item { Layout.fillHeight: true }

    Switch {
      id: control

      checked: root.desiredChecked
      Layout.alignment: Qt.AlignHCenter
      opacity: root.unknown ? 0.45 : 1.0

      palette.highlight: root.color

      onToggled: {
        if (!root.model)
          return

        root.model.checked = !root.desiredChecked
        control.checked = Qt.binding(function() { return root.desiredChecked })
      }
    }

    Item { Layout.fillHeight: true }
  }
}
