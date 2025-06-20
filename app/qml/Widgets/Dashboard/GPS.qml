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
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick
import QtLocation
import QtPositioning
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio
import QtCore as QtCore

import "../"

Item {
  id: root

  //
  // Widget data inputs
  //
  required property color color
  required property GPSModel model
  required property var windowRoot

  //
  // Window flags
  //
  property bool hasToolbar: true

  //
  // Custom properties to control the map from other QML files
  //
  property int currentMapType: 0
  property real previousLatitude: 0
  property real previousLongitude: 0
  readonly property real altitude: root.model.altitude
  readonly property real latitude: root.model.latitude
  readonly property real longitude: root.model.longitude

  //
  // Centers the map to the current coordinates
  //
  onLatitudeChanged: root.centerMap()
  onLongitudeChanged: root.centerMap()
  function centerMap() {
    if (root.previousLatitude == 0 || root.previousLongitude == 0) {
      root.previousLatitude = root.latitude
      root.previousLongitude = root.longitude
      map.center = QtPositioning.coordinate(root.latitude, root.longitude)
    }
  }

  //
  // Save settings
  //
  QtCore.Settings {
    id: settings
    property alias activeMapType: root.currentMapType
  }

  //
  // Add toolbar
  //
  RowLayout {
    id: toolbar

    spacing: 4
    height: 48

    anchors {
      leftMargin: 8
      rightMargin: 8
      top: parent.top
      left: parent.left
      right: parent.right
    }

    ToolButton {
      icon.width: 24
      icon.height: 24
      icon.color: "transparent"
      icon.source: "qrc:/rcc/icons/dashboard-buttons/crosshair.svg"
      onClicked: map.center = QtPositioning.coordinate(root.latitude, root.longitude)
    }

    ComboBox {
      id: mapType
      Layout.fillWidth: true
      textRole: "description"
      Layout.minimumHeight: 24
      Layout.maximumHeight: 24
      model: map.supportedMapTypes
      Layout.alignment: Qt.AlignVCenter
      displayText: qsTr("Map Type: %1").arg(currentText)

      onModelChanged: {
        if (map.supportedMapTypes.length > root.currentMapType)
          currentIndex = root.currentMapType
      }

      onCurrentIndexChanged: {
        if (count > 0 && root.currentMapType !== currentIndex) {
          root.currentMapType = currentIndex
          map.activeMapType = map.supportedMapTypes[currentIndex]
        }
      }
    }
  }


  //
  // Map widget
  //
  Map {
    id: map
    copyrightsVisible: true

    anchors {
      left: parent.left
      right: parent.right
      top: toolbar.bottom
      bottom: parent.bottom
      margins: 1
      topMargin: 0
    }

    property geoCoordinate startCentroid

    //
    // Set OpenStreetMap plugin with local server that makes use of API keys
    //
    plugin: Plugin {
      preferred: "osm"

      PluginParameter {
        name: "osm.mapping.highdpi_tiles"
        value: true
      }

      PluginParameter {
        value: Cpp_JSON_ProjectModel.osmAddress
        name: "osm.mapping.providersrepository.address"
      }
    }

    //
    // Position indicator
    //
    MapQuickItem {
      id: positionIndicator
      width: sourceItem.width
      height: sourceItem.height
      anchorPoint: Qt.point(sourceItem.width / 2, sourceItem.height/ 2)
      coordinate: QtPositioning.coordinate(root.latitude, root.longitude)
      sourceItem: Button {
        flat: true
        width: 128
        height: 128
        icon.width: 96
        icon.height: 96
        background: Item {}
        icon.color: "#D8343D"
        Layout.alignment: Qt.AlignVCenter
        icon.source: "qrc:/rcc/images/crosshair.svg"
      }
    }

    //
    // Add a glowing effect to the position indicator
    //
    Repeater {
      model: [[64, 0.6], [23, 0.4], [1, 0.1]]
      delegate: MultiEffect {
        blur: 1
        saturation: 0.2
        blurEnabled: true
        blurMax: modelData[0]
        brightness: modelData[1]
        source: positionIndicator
        anchors.fill: positionIndicator
        enabled: !Cpp_Misc_ModuleManager.softwareRendering
        visible: !Cpp_Misc_ModuleManager.softwareRendering
      }
    }

    //
    // Pinch handler
    //
    PinchHandler {
      id: pinch
      target: null
      enabled: windowRoot.focused
      grabPermissions: PointerHandler.TakeOverForbidden
      onActiveChanged: if (active) {
                         map.startCentroid = map.toCoordinate(pinch.centroid.position, false)
                       }
      onScaleChanged: (delta) => {
                        map.zoomLevel += Math.log2(delta)
                        map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position)
                      }
      onRotationChanged: (delta) => {
                           map.bearing -= delta
                           map.alignCoordinateToPoint(map.startCentroid, pinch.centroid.position)
                         }

    }

    //
    // Zoom handler
    //
    WheelHandler {
      id: wheel
      enabled: windowRoot.focused
      // workaround for QTBUG-87646 / QTBUG-112394 / QTBUG-112432:
      // Magic Mouse pretends to be a trackpad but doesn't work with PinchHandler
      // and we don't yet distinguish mice and trackpads on Wayland either
      acceptedDevices: Qt.platform.pluginName === "cocoa" || Qt.platform.pluginName === "wayland"
                       ? PointerDevice.Mouse | PointerDevice.TouchPad
                       : PointerDevice.Mouse
      rotationScale: 1/120
      property: "zoomLevel"
    }

    //
    // Drag handler to pan the map
    //
    DragHandler {
      id: drag
      target: null
      enabled: windowRoot.focused
      onTranslationChanged: (delta) => map.pan(-delta.x, -delta.y)
    }

    //
    // Zoom in shortcut
    //
    Shortcut {
      enabled: map.zoomLevel < map.maximumZoomLevel
      sequence: StandardKey.ZoomIn
      onActivated: map.zoomLevel = Math.round(map.zoomLevel + 1)
    }

    //
    // Zoom out shortcut
    //
    Shortcut {
      enabled: map.zoomLevel > map.minimumZoomLevel
      sequence: StandardKey.ZoomOut
      onActivated: map.zoomLevel = Math.round(map.zoomLevel - 1)
    }
  }
}
