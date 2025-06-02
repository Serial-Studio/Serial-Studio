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
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtCore
import QtQuick
import QtLocation
import QtPositioning
import QtQuick.Effects
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../"

Item {
  id: root

  //
  // Widget data inputs
  //
  required property color color
  required property GPSModel model
  required property MiniWindow windowRoot

  //
  // Window flags
  //
  readonly property bool hasToolbar: true

  //
  // Custom properties to control the map from other QML files
  //
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
  Settings {
    property alias activeMapType: mapType.currentIndex
  }

  //
  // User interface
  //
  Page {
    anchors.fill: parent
    palette.mid: Cpp_ThemeManager.colors["mid"]
    palette.dark: Cpp_ThemeManager.colors["dark"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.link: Cpp_ThemeManager.colors["link"]
    palette.light: Cpp_ThemeManager.colors["light"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.shadow: Cpp_ThemeManager.colors["shadow"]
    palette.accent: Cpp_ThemeManager.colors["accent"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.midlight: Cpp_ThemeManager.colors["midlight"]
    palette.highlight: Cpp_ThemeManager.colors["highlight"]
    palette.windowText: Cpp_ThemeManager.colors["window_text"]
    palette.brightText: Cpp_ThemeManager.colors["bright_text"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.toolTipBase: Cpp_ThemeManager.colors["tooltip_base"]
    palette.toolTipText: Cpp_ThemeManager.colors["tooltip_text"]
    palette.linkVisited: Cpp_ThemeManager.colors["link_visited"]
    palette.alternateBase: Cpp_ThemeManager.colors["alternate_base"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    ColumnLayout {
      id: layout
      spacing: 0
      anchors.fill: parent

      //
      // Spacer
      //
      Item {
        implicitHeight: 4
      }

      //
      // Map type selector
      //
      RowLayout {
        spacing: 4
        Layout.leftMargin: 4
        Layout.rightMargin: 4
        Layout.fillWidth: true

        Button {
          icon.width: 18
          icon.height: 18
          Layout.minimumWidth: 24
          Layout.maximumWidth: 24
          Layout.minimumHeight: 24
          Layout.maximumHeight: 24
          Layout.alignment: Qt.AlignVCenter
          icon.color: Cpp_ThemeManager.colors["text"]
          icon.source: "qrc:/rcc/icons/buttons/center.svg"
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
          onCurrentIndexChanged: map.activeMapType = map.supportedMapTypes[currentIndex]
        }
      }

      //
      // Spacer
      //
      Item {
        implicitHeight: 4
      }

      //
      // Map widget
      //
      Map {
        id: map
        copyrightsVisible: false

        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.leftMargin: -8
        Layout.rightMargin: -8
        Layout.bottomMargin: -8

        property geoCoordinate startCentroid

        plugin: Plugin {
          preferred: "osm"

          PluginParameter {
            name: "osm.mapping.highdpi_tiles"
            value: true
          }

          PluginParameter {
            name: "osm.mapping.providersrepository.address"
            value: Cpp_JSON_ProjectModel.osmAddress
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
  }
}
