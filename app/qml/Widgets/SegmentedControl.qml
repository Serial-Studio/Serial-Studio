/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.impl

Rectangle {
  id: root

  //
  // Emitted when the user picks a segment other than the current one
  //
  signal activated(int index)

  //
  // Model is a list of {text, icon, tooltip} entries
  //
  property var model: []
  property int iconSize: 18
  property int currentIndex: 0
  property color iconColor: "transparent"

  //
  // Container styling
  //
  radius: 5
  border.width: 1
  implicitHeight: 32
  color: "transparent"
  opacity: enabled ? 1 : 0.5
  implicitWidth: _row.implicitWidth
  border.color: Cpp_ThemeManager.colors["widget_border"]

  Row {
    id: _row

    anchors.fill: parent

    Repeater {
      model: root.model

      delegate: AbstractButton {
        id: _segment

        required property var modelData
        required property int index

        leftPadding: 10
        rightPadding: 10
        height: root.height
        checked: root.currentIndex === index

        onClicked: {
          if (root.currentIndex !== index)
            root.activated(index)
        }

        ToolTip.delay: 500
        ToolTip.text: modelData.tooltip ?? ""
        ToolTip.visible: hovered && ToolTip.text !== ""

        background: Rectangle {
          border.width: 1
          anchors.fill: parent
          anchors.margins: 2
          radius: root.radius - 2
          color: Cpp_ThemeManager.colors["toolbar_checked_button_background"]
          border.color: Cpp_ThemeManager.colors["toolbar_checked_button_border"]
          opacity: {
            const full = Cpp_ThemeManager.colors["toolbar_checked_button_opacity"]
            if (_segment.checked)
              return full

            return _segment.hovered ? full * 0.4 : 0
          }

          Behavior on opacity { NumberAnimation {} }
        }

        contentItem: RowLayout {
          spacing: 6

          IconImage {
            color: root.iconColor
            source: _segment.modelData.icon ?? ""
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.iconSize
            Layout.preferredHeight: root.iconSize
            sourceSize: Qt.size(root.iconSize, root.iconSize)
            visible: (_segment.modelData.icon ?? "") !== ""
          }

          Label {
            font: _segment.font
            text: _segment.modelData.text ?? ""
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["button_text"]
          }
        }
      }
    }
  }
}
