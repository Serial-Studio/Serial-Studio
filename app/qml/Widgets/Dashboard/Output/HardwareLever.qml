/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */


import QtQuick
import QtQuick.Shapes

import "../.." as Widgets

//
// Toggle lever switch (spec 0113): a bat handle on a knurled nut, thrown up for on and down
// for off, with the two legends engraved above and below. Click it, or press Space or Return.
//
Item {
  id: root

  property bool on: false
  property bool dimmed: false
  property string onLabel: ""
  property string offLabel: ""

  signal toggled()

  readonly property real cx: width / 2
  readonly property real cy: height / 2
  readonly property real legendGap: Math.max(onLegend.height, offLegend.height) + 4
  readonly property real nut: Math.min(width * 0.5, (height - root.legendGap * 2) * 0.42)

  activeFocusOnTab: true
  opacity: root.dimmed ? 0.45 : 1.0
  Keys.onSpacePressed: root.toggled()
  Keys.onReturnPressed: root.toggled()

  Text {
    id: onLegend

    color: Cpp_ThemeManager.colors["widget_text"]
    anchors.horizontalCenter: parent.horizontalCenter
    y: Math.max(0, root.cy - root.nut * 1.15 - height - 2)
    text: root.onLabel.length > 0 ? root.onLabel : qsTr("ON")
    font: Cpp_Misc_CommonFonts.customUiFont(0.8, root.on)
  }

  Text {
    id: offLegend

    color: Cpp_ThemeManager.colors["widget_text"]
    anchors.horizontalCenter: parent.horizontalCenter
    y: Math.min(root.height - height, root.cy + root.nut * 1.15 + 2)
    text: root.offLabel.length > 0 ? root.offLabel : qsTr("OFF")
    font: Cpp_Misc_CommonFonts.customUiFont(0.8, !root.on)
  }

  Widgets.PanelBezel {
    knurled: true
    width: root.nut
    height: root.nut
    anchors.centerIn: parent
  }

  //
  // Bat handle: a tapered shaft from the nut to a ball tip, pivoting on the nut's centre
  //
  Item {
    id: handle

    y: root.cy - height
    width: root.nut * 0.42
    height: root.nut * 1.1
    x: root.cx - width / 2
    transform: Rotation {
      origin.y: handle.height
      angle: root.on ? 0 : 180
      origin.x: handle.width / 2

      Behavior on angle {
        enabled: !Cpp_Misc_GraphicsBackend.reduceMotion

        NumberAnimation {
          duration: 120
          easing.type: Easing.OutCubic
        }
      }
    }

    Shape {
      smooth: true
      antialiasing: true
      anchors.fill: parent
      preferredRendererType: Shape.CurveRenderer

      ShapePath {
        strokeColor: Qt.rgba(0.35, 0.37, 0.40, 1)
        strokeWidth: Math.max(0.75, handle.width * 0.04)
        fillGradient: LinearGradient {
          x1: 0
          y1: 0
          y2: 0
          x2: handle.width
          GradientStop { position: 0.0; color: Qt.rgba(0.62, 0.64, 0.67, 1) }
          GradientStop { position: 0.4; color: Qt.rgba(0.95, 0.96, 0.97, 1) }
          GradientStop { position: 1.0; color: Qt.rgba(0.50, 0.52, 0.55, 1) }
        }

        startY: handle.height
        startX: handle.width * 0.38
        PathLine { x: handle.width * 0.30; y: handle.width * 0.5 }
        PathArc {
          useLargeArc: true
          y: handle.width * 0.5
          x: handle.width * 0.70
          radiusX: handle.width * 0.2
          radiusY: handle.width * 0.2
        }
        PathLine { x: handle.width * 0.62; y: handle.height }
        PathLine { x: handle.width * 0.38; y: handle.height }
      }
    }
  }

  //
  // Hub: the shiny cap the handle leaves the nut through
  //
  Rectangle {
    height: width
    border.width: 1
    radius: width / 2
    width: root.nut * 0.3
    anchors.centerIn: parent
    border.color: Qt.rgba(0.35, 0.37, 0.40, 1)
    gradient: Gradient {
      GradientStop { position: 0.0; color: Qt.rgba(0.96, 0.97, 0.98, 1) }
      GradientStop { position: 1.0; color: Qt.rgba(0.55, 0.57, 0.60, 1) }
    }
  }

  //
  // Focus mark: a thin ring around the nut
  //
  Rectangle {
    border.width: 2
    radius: width / 2
    color: "transparent"
    width: root.nut + 6
    height: root.nut + 6
    visible: root.activeFocus
    anchors.centerIn: parent
    border.color: Cpp_ThemeManager.colors["highlight"]
  }

  MouseArea {
    anchors.fill: parent
    onClicked: {
      root.forceActiveFocus()
      root.toggled()
    }
  }
}
