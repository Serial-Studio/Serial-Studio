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

import "../.." as Widgets

Item {
  id: root

  required property color color
  required property var model
  required property var widget

  readonly property real scale: root.widget ? (root.widget.sizeScale || 1) : 1
  readonly property bool mono: root.widget ? (root.widget.monoIcon || false) : false
  readonly property bool checkable: root.widget ? (root.widget.checkable || false) : false
  readonly property string iconSource: root.widget ? (root.widget.icon || "") : ""
  readonly property string onLabel: root.widget ? (root.widget.onLabel || "") : ""
  readonly property string offLabel: root.widget ? (root.widget.offLabel || "") : ""
  readonly property string title: root.widget ? (root.widget.title || "") : ""
  readonly property bool unknown: root.model
                                  ? (root.model.stateBound && !root.model.stateKnown)
                                  : false

  //
  // Restorable after a click clears the binding; never initialValue, which cannot track
  //
  readonly property bool desiredChecked: {
    if (!root.model)
      return false

    if (root.model.stateBound)
      return root.model.stateKnown && root.model.stateOn

    return root.model.checked === true
  }

  //
  // Readable caption over an arbitrary accent, composited over the backdrop when translucent
  //
  function captionColor(fill, backdrop) {
    var c = Qt.darker(fill, 1.0)

    var r = c.r, g = c.g, b = c.b
    if (c.a < 1.0) {
      var bg = Qt.darker(backdrop !== undefined ? backdrop : palette.window, 1.0)
      r = c.a * r + (1 - c.a) * bg.r
      g = c.a * g + (1 - c.a) * bg.g
      b = c.a * b + (1 - c.a) * bg.b
    }

    function lin(v) {
      return v <= 0.04045 ? v / 12.92 : Math.pow((v + 0.055) / 1.055, 2.4)
    }

    var L = 0.2126 * lin(r) + 0.7152 * lin(g) + 0.0722 * lin(b)
    return L > 0.179 ? "#000000" : "#ffffff"
  }

  ColumnLayout {
    spacing: 6
    anchors.margins: 8
    anchors.fill: parent

    Item { Layout.fillHeight: true }

    Widgets.IconButton {
      id: control

      checkable: root.checkable
      checked: root.desiredChecked
      Layout.alignment: Qt.AlignHCenter
      opacity: root.unknown ? 0.45 : 1.0
      iconSize: Math.round(16 * root.scale)
      icon.color: root.mono ? control.color : "transparent"
      font: Cpp_Misc_CommonFonts.customUiFont(root.scale, false)
      Layout.preferredWidth: Math.min(parent.width, Math.round(200 * root.scale))
      icon.source: root.iconSource.length > 0 ? root.iconSource : "qrc:/actions/Gears.svg"

      text: {
        var caption = root.checkable
                      ? (control.checked ? root.onLabel : root.offLabel)
                      : root.onLabel
        if (caption.length > 0)
          return caption

        return root.title.length > 0 ? root.title : qsTr("Send")
      }

      palette.button: root.color
      color: root.captionColor(root.color)

      onClicked: {
        if (!root.model)
          return

        if (!root.checkable) {
          root.model.click()
          return
        }

        root.model.checked = !root.desiredChecked
        control.checked = Qt.binding(function() { return root.desiredChecked })
      }
    }

    Item { Layout.fillHeight: true }
  }
}
