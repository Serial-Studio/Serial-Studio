/*
 * Copyright (c) 2018 Kaan-Sat <https://kaansat.com.mx/>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

import QtQuick 2.0
import QtLocation 5.12
import QtPositioning 5.11
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import Qt.labs.settings 1.0

import SerialStudio 1.0

import "."

Window {
    id: root

    //
    // Window properties
    //
    spacing: -1
    gradient: true
    implicitWidth: 260
    visible: opacity > 0
    opacity: enabled ? 1 : 0
    backgroundColor: "#09090c"
    implicitHeight: implicitWidth + 96
    icon.source: "qrc:/icons/location-on.svg"

    //
    // Settings
    //
    Settings {
        property alias mapTilt: tiltSlider.value
        property alias mapZoom: zoomSlider.value
        property alias autoCenter: autoCenter.checked
    }

    //
    // Custom properties
    //
    property real latitude: 0
    property real longitude: 0
    property int groupIndex: 0
    readonly property var gpsCoordinates: QtPositioning.coordinate(latitude,
                                                                   longitude)

    //
    // Connections with widget manager
    //
    Connections {
        target: Cpp_UI_WidgetProvider
        function onDataChanged() {
            root.updateValues()
        }
    }

    //
    // Updates the internal values of the map widget
    //
    function updateValues() {
        if (Cpp_UI_WidgetProvider.mapGroupCount() > root.groupIndex) {
            root.latitude = Cpp_UI_WidgetProvider.mapLatitude(root.groupIndex)
            root.longitude = Cpp_UI_WidgetProvider.mapLongitude(root.groupIndex)
            root.title = Cpp_UI_WidgetProvider.mapGroupAt(root.groupIndex).title

            if (autoCenter.checked)
                root.centerMap()
        }

        else {
            root.title = ""
            root.latitude = 0
            root.longitude = 0
        }
    }

    //
    // Animations
    //
    Behavior on latitude {NumberAnimation{}}
    Behavior on longitude {NumberAnimation{}}

    //
    // Centers the map to the current coordinates
    //
    function centerMap() {
        map.center = gpsCoordinates
    }

    //
    // UI controls
    //
    ColumnLayout {
        spacing: app.spacing
        anchors.fill: parent
        anchors.margins: app.spacing

        //
        // Center map + zoom slider
        //
        RowLayout {
            CheckBox {
                id: autoCenter
                checked: true
                checkable: true
                Layout.leftMargin: -6
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Center on coordinate")
                onCheckedChanged: {
                    if (checked)
                        root.centerMap()
                }
            }

            Slider {
                id: zoomSlider
                value: map.zoomLevel
                Layout.fillWidth: true
                to: map.maximumZoomLevel
                from: map.minimumZoomLevel
                Layout.alignment: Qt.AlignHCenter
                onValueChanged: {
                    if (map.zoomLevel !== value)
                        map.zoomLevel = value
                }
            }
        }

        //
        // Map
        //
        RowLayout {
            //
            // Tilt slider
            //
            Slider {
                id: tiltSlider
                orientation: Qt.Vertical
                Layout.fillHeight: true
                from: map.minimumTilt
                to: map.maximumTilt
                value: map.tilt
                onValueChanged: {
                    if (map.tilt != value)
                        map.tilt = value
                }
            }

            //
            // Map
            //
            Rectangle {
                id: mapRect
                clip: true
                Layout.fillWidth: true
                Layout.fillHeight: true

                border {
                    width: 2
                    color: "#646464"
                }

                gradient: Gradient {
                    GradientStop {
                        color: "#6Ba9d1"
                        position: Math.max(0.4,
                                           (map.maximumTilt - map.tilt) /
                                            map.maximumTilt)
                    }

                    GradientStop {
                        position: 0
                        color: "#283E51"
                    }
                }

                Map {
                    id: map
                    smooth: true
                    antialiasing: true
                    color: "transparent"
                    anchors.fill: parent
                    copyrightsVisible: false
                    anchors.margins: parent.border.width

                    tilt: 27
                    zoomLevel: 16

                    MapQuickItem {
                        coordinate: gpsCoordinates
                        anchorPoint: Qt.point(sourceItem.width / 2,
                                              sourceItem.height/ 2)

                        sourceItem: Rectangle {
                            id: dot
                            width: 20
                            height: 20
                            opacity: 0.8
                            color: "#f00"
                            border.width: 2
                            radius: width / 2
                            border.color: "#fff"
                        }
                    }

                    plugin: Plugin {
                        name: "mapbox"

                        PluginParameter {
                            name: "mapbox.map_id"
                            value: "mapbox.satellite"
                        }

                        PluginParameter {
                            name: "mapbox.access_token"
                            value: "pk.eyJ1IjoiYWxleC1zcGF0YXJ1IiwiYSI6ImNra29icjU1bDAxbTEyb2tsemZ1OGtnajgifQ.N75tGLUx5G2HmSeVXHmBGw"
                        }
                    }
                }

                Rectangle {
                    id: smog
                    height: 32
                    opacity: 0.5

                    Connections {
                        target: map
                        function onTiltChanged() {
                            var x = map.tilt / map.maximumTilt
                            smog.y = (1.666 * x - 1.416) * mapRect.height
                        }
                    }

                    gradient: Gradient {
                        GradientStop {
                            position: 0
                            color: "transparent"
                        }

                        GradientStop {
                            position: 0.5
                            color: "#dedede"
                        }

                        GradientStop {
                            position: 1
                            color: "transparent"
                        }
                    }

                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                }
            }
        }
    }
}
