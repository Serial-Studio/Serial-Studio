/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR Commercial
 */

import QtQml
import QtQuick
import QtQuick.Shapes

Item {
  id: control

  property int trackWidth: 20
  property int progressWidth: 20

  property real startAngle: 0
  property real endAngle: 360

  property real value: 0.0
  property real minValue: 0.0
  property real maxValue: 1.0
  property real alarmValue: 0.5

  property int capStyle: Qt.RoundCap

  property color alarmColor: "#ff0000"
  property color trackColor: "#505050"
  property color handleColor: "#fefefe"
  property color progressColor: "#3a4ec4"

  property real stepSize: 0.1

  property Component alarmMarker: null
  property Component valueMarker: null

  property bool snap: false
  property bool hideTrack: false
  property bool hideProgress: false
  property bool alarmEnabled: false

  property int valueMarkerWidth: 22
  property int valueMarkerHeight: 22
  property int valueMarkerVerticalOffset: 0

  property int alarmMarkerWidth: 22
  property int alarmMarkerHeight: 22
  property int alarmMarkerVerticalOffset: 0

  readonly property alias angle: internal.angle
  readonly property alias baseRadius: internal.baseRadius
  readonly property alias alarmStartAngle: internal.alarmStartAngle

  implicitWidth: 250
  implicitHeight: 250

  Binding {
    target: control
    property: "value"
    when: internal.setUpdatedValue
    restoreMode: Binding.RestoreBinding
    value: control.snap ? internal.snappedValue : internal.mapFromValue(startAngle, endAngle, minValue, maxValue, internal.angleProxy)
  }

  QtObject {
    id: internal

    property real snappedValue: 0.0
    property bool setUpdatedValue: false
    property real angleProxy: control.startAngle
    property color trackColor: control.trackColor
    property color transparentColor: "transparent"
    property real actualSpanAngle: control.endAngle - control.startAngle
    property var centerPt: Qt.point(control.width / 2, control.height / 2)
    property real baseRadius: Math.min(control.width, control.height) / 2 - Math.max(control.trackWidth, control.progressWidth) / 2
    readonly property real angle: internal.mapFromValue(control.minValue, control.maxValue, control.startAngle, control.endAngle, control.value)
    property real alarmStartAngle: internal.mapFromValue(control.minValue, control.maxValue, control.startAngle, control.endAngle, control.alarmValue)

    function mapFromValue(inMin, inMax, outMin, outMax, inValue) {
      return (inValue - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
    }
  }

  //
  // Track Shape
  //
  Shape {
    id: trackShape

    layer.samples: 8
    layer.enabled: true
    width: control.width
    height: control.height
    visible: !control.hideTrack

    ShapePath {
      id: trackShapePath

      capStyle: control.capStyle
      strokeColor: control.trackColor
      strokeWidth: control.trackWidth
      fillColor: internal.transparentColor

      PathAngleArc {
        centerX: control.width / 2
        centerY: control.height / 2
        radiusX: internal.baseRadius
        radiusY: internal.baseRadius
        startAngle: control.startAngle - 90
        sweepAngle: internal.actualSpanAngle
      }
    }
  }

  //
  // Progress Shape
  //
  Shape {
    id: progressShape

    layer.samples: 8
    layer.enabled: true
    width: control.width
    height: control.height
    visible: !control.hideProgress

    ShapePath {
      id: progressShapePath

      capStyle: control.capStyle
      strokeWidth: control.progressWidth
      strokeColor: control.progressColor
      fillColor: internal.transparentColor

      PathAngleArc {
        centerX: control.width / 2
        centerY: control.height / 2
        radiusX: internal.baseRadius
        radiusY: internal.baseRadius
        startAngle: control.startAngle - 90
        sweepAngle: control.angle - control.startAngle
      }
    }
  }

  //
  // Alarm Progress Shape
  //
  Shape {
    id: alarmShape

    layer.samples: 8
    layer.enabled: true
    width: control.width
    height: control.height
    visible: control.alarmEnabled && control.value > control.alarmValue

    ShapePath {
      id: alarmShapePath

      capStyle: control.capStyle
      strokeWidth: control.progressWidth
      strokeColor: control.alarmColor
      fillColor: internal.transparentColor

      PathAngleArc {
        centerX: control.width / 2
        centerY: control.height / 2
        radiusX: internal.baseRadius
        radiusY: internal.baseRadius
        startAngle: internal.alarmStartAngle - 90
        sweepAngle: control.angle - internal.alarmStartAngle
      }
    }
  }

  //
  // Alarm Marker
  //
  Item {
    id: alarmMarkerItem

    z: 3
    antialiasing: true
    width: control.alarmMarkerWidth
    height: control.alarmMarkerHeight
    x: control.width / 2 - width / 2
    y: control.height / 2 - height / 2
    visible: control.alarmEnabled && control.alarmMarker != null

    transform: [
      Translate {
        y: -(Math.min(control.width, control.height) / 2) + Math.max(control.trackWidth, control.progressWidth) / 2 + control.alarmMarkerVerticalOffset
      },
      Rotation {
        angle: internal.alarmStartAngle
        origin.x: alarmMarkerItem.width / 2
        origin.y: alarmMarkerItem.height / 2
      }
    ]

    Loader {
      sourceComponent: control.alarmMarker
    }
  }

  //
  // Value Marker
  //
  Item {
    id: valueMarkerItem

    z: 3
    antialiasing: true
    width: control.valueMarkerWidth
    height: control.valueMarkerHeight
    x: control.width / 2 - width / 2
    y: control.height / 2 - height / 2
    visible: control.valueMarker != null

    transform: [
      Translate {
        y: -(Math.min(control.width, control.height) / 2) + Math.max(control.trackWidth, control.progressWidth) / 2 + control.valueMarkerVerticalOffset
      },
      Rotation {
        angle: internal.angle
        origin.x: valueMarkerItem.width / 2
        origin.y: valueMarkerItem.height / 2
      }
    ]

    Loader {
      sourceComponent: control.valueMarker
    }
  }
}
