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

  property string idPrefix: ""

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
  // What the control is (project, spec 0113): 0 standard, 1 illuminated, 2 emergency stop;
  // a standard button with room for it is drawn as hardware unless the theme asks for plain
  //
  readonly property bool emergencyStop: root.style === 2
  readonly property bool hideTitle: root.widget ? root.widget.hideTitle === true : false
  readonly property int style: root.widget ? (root.widget.style || 0) : 0
  readonly property bool standardLook: !root.emergencyStop && !root.pushButton
  readonly property bool hardwareLook: Cpp_ThemeManager.widgetLooks["output"] !== "plain"
  readonly property bool pushButton: !root.emergencyStop
                                     && (root.style === 1
                                         || (root.hardwareLook && root.height >= 72))

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

  //
  // Caption for a pressed or released control: its on/off label, else its title
  //
  function caption(on) {
    var label = root.checkable ? (on ? root.onLabel : root.offLabel) : root.onLabel
    if (label.length > 0)
      return label

    return root.title.length > 0 ? root.title : qsTr("Send")
  }

  //
  // Height of a round hardware part: its nominal size, or what the cell leaves for it
  //
  function partHeight(nominal) {
    return Math.max(24, Math.min(Math.round(nominal * root.scale), root.height - 48))
  }

  //
  // One operator press: a pulse for a momentary button, a flip of the latch otherwise
  //
  function activate() {
    if (!root.model)
      return

    Cpp_UI_Alarms.playEvent("button")

    if (!root.checkable) {
      root.model.click()
      return
    }

    root.model.checked = !root.desiredChecked
  }

  ColumnLayout {
    spacing: 6
    anchors.margins: 8
    anchors.fill: parent

    Item { Layout.fillHeight: true }

    //
    // Emergency stop: one press latches and sends at once, a later press releases
    //
    Loader {
      Layout.fillWidth: true
      active: root.emergencyStop
      visible: root.emergencyStop
      Layout.preferredHeight: root.partHeight(132)

      sourceComponent: HardwareEmergencyStop {
        dimmed: root.unknown
        latched: root.desiredChecked
        sendable: Cpp_IO_Manager.isConnected
        legend: root.title.length > 0 ? root.title : qsTr("EMERGENCY STOP")
        objectName: root.idPrefix.length > 0 ? root.idPrefix + ".button" : ""

        readonly property bool checked: root.desiredChecked

        onPressRequested: {
          if (!root.model)
            return

          Cpp_UI_Alarms.playEvent("button")
          root.model.checked = true
        }

        onReleaseRequested: {
          if (!root.model)
            return

          Cpp_UI_Alarms.playEvent("button")
          root.model.checked = false
        }
      }
    }

    //
    // Panel push button: the illuminated style under any theme, or the hardware look
    //
    Loader {
      Layout.fillWidth: true
      active: root.pushButton
      visible: root.pushButton
      Layout.preferredHeight: root.partHeight(72)

      sourceComponent: HardwarePushButton {
        dimmed: root.unknown
        headColor: root.color
        lit: root.desiredChecked
        illuminated: root.style === 1
        objectName: root.idPrefix.length > 0 ? root.idPrefix + ".button" : ""

        readonly property bool checked: root.desiredChecked

        onClicked: root.activate()
      }
    }

    Label {
      elide: Label.ElideRight
      Layout.fillWidth: true
      text: root.caption(root.desiredChecked)
      visible: !root.standardLook && !root.hideTitle
      horizontalAlignment: Label.AlignHCenter
      color: Cpp_ThemeManager.colors["widget_text"]
      font: (Cpp_ThemeManager.themeName,
             Cpp_ThemeManager.lettering("engraving",
                                        Cpp_Misc_CommonFonts.customUiFont(0.85 * root.scale, true)))
    }

    Widgets.IconButton {
      id: control

      checkable: root.checkable
      visible: root.standardLook
      checked: root.desiredChecked
      Layout.alignment: Qt.AlignHCenter
      opacity: root.unknown ? 0.45 : 1.0
      iconSize: Math.round(16 * root.scale)
      icon.color: root.mono ? control.color : "transparent"
      font: Cpp_Misc_CommonFonts.customUiFont(root.scale, false)
      Layout.preferredWidth: Math.min(parent.width, Math.round(200 * root.scale))
      icon.source: root.iconSource.length > 0 ? root.iconSource : "qrc:/actions/Gears.svg"

      objectName: root.standardLook && root.idPrefix.length > 0 ? root.idPrefix + ".button" : ""

      text: root.caption(control.checked)

      palette.button: root.color
      color: root.captionColor(root.color)

      onClicked: {
        root.activate()
        control.checked = Qt.binding(function() { return root.desiredChecked })
      }
    }

    Item { Layout.fillHeight: true }
  }
}
