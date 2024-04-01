/*
 * Copyright (c) 2020-2024 Alex Spataru <https://github.com/alex-spataru>
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

import QtCore
import QtQuick
import QtLocation
import QtPositioning
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: root

    //
    // Custom properties to control the map from other QML files
    //
    property real latitude: 0
    property real longitude: 0
    property real altitude: 0 // Not used yet :(
    onLatitudeChanged: {
        if (autoCenter.checked)
            centerMap()
    }

    onLongitudeChanged: {
        if (autoCenter.checked)
            centerMap()
    }

    //
    // Centers the map to the current coordinates
    //
    function centerMap() {
        map.center = QtPositioning.coordinate(root.latitude, root.longitude)
    }

    //
    // Save settings accross runs
    //
    Settings {
        property alias mapTilt: tiltSlider.value
        property alias mapZoom: zoomSlider.value
        property alias mapCenter: autoCenter.checked
        property alias mapVariant: mapType.currentIndex
    }

    //
    // UI controls
    //
    ColumnLayout {
        spacing: app.spacing
        anchors.fill: parent
        anchors.margins: app.spacing

        RowLayout {
            spacing: app.spacing

            Label {
                text: qsTr("Map Type:")
                Layout.alignment: Qt.AlignVCenter
            }

            ComboBox {
                id: mapType
                Layout.fillWidth: true
                textRole: "description"
                model: map.supportedMapTypes
                Layout.alignment: Qt.AlignVCenter
                onCurrentIndexChanged: map.activeMapType = map.supportedMapTypes[currentIndex]
            }
        }

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
                    color: Cpp_ThemeManager.border
                }

                gradient: Gradient {
                    GradientStop {
                        color: "#6ba9d1"
                        position: Math.max(0.4,
                                           (map.maximumTilt - map.tilt) /
                                           map.maximumTilt)
                    }

                    GradientStop {
                        position: 0
                        color: "#283e51"
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
                        anchorPoint: Qt.point(sourceItem.width / 2,
                                              sourceItem.height/ 2)
                        coordinate: QtPositioning.coordinate(root.latitude,
                                                             root.longitude)

                        sourceItem: Rectangle {
                            id: dot
                            width: 20
                            height: 20
                            opacity: 0.8
                            border.width: 2
                            radius: width / 2
                            color: "#ff0000"
                            border.color: "#ffffff"
                        }
                    }

                    plugin: Plugin {
                        preferred: "osm"

                        PluginParameter {
                            name: "osm.mapping.highdpi_tiles"
                            value: true
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
