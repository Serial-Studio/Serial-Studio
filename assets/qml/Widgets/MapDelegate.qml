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
import QtLocation 5.11
import QtPositioning 5.11
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Universal 2.0

import Qt.labs.settings 1.0

ColumnLayout {
    id: gps
    spacing: app.spacing
    Component.onCompleted: centerMap()

    //
    // Real-time GPS coordinates components
    //
    property real latitude: CppQmlBridge.gpsLatitude
    property real longitude: CppQmlBridge.gpsLongitude

    //
    // Will be true if GPS coordinates are different from (0,0)
    //
    readonly property bool gpsWorking: latitude != 0 || longitude != 0

    //
    // Location of Queretaro
    //
    readonly property var qroCoordinates: QtPositioning.coordinate(20.5846129, -100.385372)

    //
    // Used to know if we need to center the map
    //
    property var oldCoordinates: QtPositioning.coordinate(0,0)

    //
    // Real-time position
    //
    readonly property var gpsCoordinates: QtPositioning.coordinate(latitude, longitude)

    //
    // Center map when connecting with CanSat
    //
    Connections {
        target: CppQmlBridge

        function onUpdated() {
            if (oldCoordinates === QtPositioning.coordinate(0,0)) {
                map.center = gpsCoordinates
                oldCoordinates = gpsCoordinates
            }

            gps.latitude = CppQmlBridge.gpsLatitude
            gps.longitude = CppQmlBridge.gpsLongitude
        }
    } Connections {
        target: CppSerialManager

        function onPortChanged() {
            oldCoordinates = QtPositioning.coordinate(0,0)
        }
    }

    //
    // Centers the map to Queretaro if the GPS is not working,
    // otherwise, centers the map to the CanSat's position
    //
    function centerMap() {
        // GPS not responding, go to QRO
        if (!gpsWorking)
            map.center = qroCoordinates

        // Show GPS position
        else
            map.center = gpsCoordinates
    }

    //
    // Save settings between runs
    //
    Settings {
        property alias mapType: mapTypeSelector.currentIndex
    }

    //
    // Controls
    //
    RowLayout {
        spacing: app.spacing
        Layout.fillWidth: true

        ComboBox {
            id: mapTypeSelector
            textRole: "description"
            Layout.fillWidth: true
            model: map.supportedMapTypes
            onCurrentIndexChanged: map.activeMapType = map.supportedMapTypes[currentIndex]
        }

        Button {
            text: qsTr("Re-center")
            icon.color: palette.buttonText
            icon.source: "qrc:/icons/location-on.svg"
            onClicked: centerMap()
        }
    }

    //
    // Map
    //
    Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true

        border {
            width: 2
            color: "#646464"
        }

        Map {
            id: map
            anchors.fill: parent
            copyrightsVisible: false
            color: Universal.background
            anchors.margins: parent.border.width
            zoomLevel: (map.minimumZoomLevel + map.maximumZoomLevel) * 0.8

            MapQuickItem {
                sourceItem: Rectangle {
                    id: dot
                    width: 20
                    height: 20
                    opacity: 0.1
                    color: "#f00"
                    border.width: 2
                    radius: width / 2
                    border.color: "#fff"

                    Connections {
                        target: CppSerialManager
                        function onPacketReceived() {
                            timer.restart()
                            dot.opacity = 0.8
                        }
                    }

                    Behavior on opacity { NumberAnimation { duration: 500 } }

                    Timer {
                        id: timer
                        repeat: true
                        interval: 500
                        Component.onCompleted: start()
                        onTriggered: dot.opacity = CppSerialManager.connected ? 0.25 : 0.1
                    }
                }

                coordinate: gpsWorking ? gpsCoordinates : qroCoordinates
                anchorPoint: Qt.point(sourceItem.width / 2,
                                      sourceItem.height/ 2)
            }

            plugin: Plugin {
                name: "osm"
            }
        }
    }
}
