/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */


import QtQuick

//
// Readout display (spec 0118): a dark glass panel showing a number field's value in large
// digits with its units. Click it, or press Space or Return, to edit the value.
//
Item {
  id: root

  property string text: ""
  property string units: ""
  property bool dimmed: false
  property real textScale: 1
  property color accent: Cpp_ThemeManager.colors["highlight"]

  signal activated()

  readonly property color ink: Qt.hsva(Math.max(0, root.accent.hsvHue),
                                       root.accent.hsvSaturation * 0.65, 1, 1)

  activeFocusOnTab: true
  opacity: root.dimmed ? 0.45 : 1.0
  Keys.onSpacePressed: root.activated()
  Keys.onReturnPressed: root.activated()
  implicitHeight: Math.ceil(digits.implicitHeight + 12 * root.textScale)

  Rectangle {
    radius: 4
    border.width: 1
    anchors.fill: parent
    border.color: Qt.rgba(0, 0, 0, 0.6)
    gradient: Gradient {
      GradientStop { position: 0.0; color: Qt.rgba(0.05, 0.06, 0.07, 1) }
      GradientStop { position: 1.0; color: Qt.rgba(0.11, 0.13, 0.15, 1) }
    }
  }

  //
  // Glass highlight: a faint line along the bottom edge, as on the theme's well material
  //
  Rectangle {
    height: 1
    opacity: 0.18
    color: Qt.rgba(1, 1, 1, 1)
    anchors {
      leftMargin: 4
      rightMargin: 4
      bottomMargin: 1
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
  }

  Row {
    anchors.centerIn: parent
    spacing: Math.round(6 * root.textScale)

    Text {
      id: digits

      color: root.ink
      text: root.text.length > 0 ? root.text : "--"
      font: Cpp_Misc_CommonFonts.customMonoFont(2.2 * root.textScale, true)
    }

    Text {
      opacity: 0.8
      color: root.ink
      text: root.units
      visible: root.units.length > 0
      anchors.baseline: digits.baseline
      font: Cpp_Misc_CommonFonts.customUiFont(0.9 * root.textScale, false)
    }
  }

  //
  // Focus mark: a thin ring just outside the glass
  //
  Rectangle {
    radius: 6
    border.width: 2
    color: "transparent"
    anchors.margins: -3
    anchors.fill: parent
    visible: root.activeFocus
    border.color: Cpp_ThemeManager.colors["highlight"]
  }

  MouseArea {
    anchors.fill: parent
    cursorShape: Qt.IBeamCursor
    onClicked: root.activated()
  }
}
