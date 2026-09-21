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

  //
  // Selection pill: one rectangle that slides between segments, so a change of choice reads as
  // movement rather than as two unrelated fades. Armed a tick late so it never glides in from 0
  //
  Rectangle {
    id: _pill

    property bool armed: false
    readonly property Item segment: {
      void _segments.count
      return _segments.itemAt(root.currentIndex)
    }

    function arm() {
      _pill.armed = true
    }

    y: 2
    border.width: 1
    radius: root.radius - 2
    height: root.height - 4
    x: _pill.segment !== null ? _pill.segment.x + 2 : 0
    width: _pill.segment !== null ? _pill.segment.width - 4 : 0
    color: Cpp_ThemeManager.colors["toolbar_checked_button_background"]
    border.color: Cpp_ThemeManager.colors["toolbar_checked_button_border"]
    opacity: _pill.segment !== null ? Cpp_ThemeManager.colors["toolbar_checked_button_opacity"] : 0

    Component.onCompleted: Qt.callLater(_pill.arm)

    Behavior on x {
      enabled: _pill.armed && !Cpp_Misc_GraphicsBackend.reduceMotion
      NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
    }

    Behavior on width {
      enabled: _pill.armed && !Cpp_Misc_GraphicsBackend.reduceMotion
      NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
    }
  }

  Row {
    id: _row

    anchors.fill: parent

    Repeater {
      id: _segments

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
              return 0

            return _segment.hovered ? full * 0.4 : 0
          }

          Behavior on opacity { NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
        }

        contentItem: RowLayout {
          spacing: 6

          IconImage {
            color: root.iconColor
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: root.iconSize
            source: _segment.modelData.icon ?? ""
            Layout.preferredHeight: root.iconSize
            visible: (_segment.modelData.icon ?? "") !== ""
            sourceSize: Qt.size(root.iconSize, root.iconSize)
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
