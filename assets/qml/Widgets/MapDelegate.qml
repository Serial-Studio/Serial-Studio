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

import Group 1.0
import Dataset 1.0

import "."

Window {
    id: root

    //
    // Window properties
    //
    spacing: -1
    implicitWidth: 260
    visible: opacity > 0
    opacity: enabled ? 1 : 0
    implicitHeight: implicitWidth + 96
    icon.source: "qrc:/icons/location-on.svg"
    borderColor: Qt.rgba(45/255, 96/255, 115/255, 1)
    backgroundColor: Qt.rgba(9 / 255, 9 / 255, 12 / 255, 1)

    //
    // Custom properties
    //
    property real latitude: 0
    property real longitude: 0
    property int groupIndex: 0
    readonly property var gpsCoordinates: QtPositioning.coordinate(latitude, longitude)

    //
    // Connections with widget manager
    //
    Connections {
        target: CppWidgets
        function onDataChanged() {
            root.updateValues()
        }
    }

    //
    // Updates the internal values of the map widget
    //
    function updateValues() {
        if (CppWidgets.mapGroupCount() > root.groupIndex) {
            root.latitude = CppWidgets.mapLatitude(root.groupIndex)
            root.longitude = CppWidgets.mapLongitude(root.groupIndex)
            root.title = CppWidgets.mapGroupAt(root.groupIndex).title
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
    // Save settings between runs
    //
    Settings {
        property alias mapType: mapTypeSelector.currentIndex
    }

    //
    // UI controls
    //
    ColumnLayout {
        spacing: app.spacing
        anchors.fill: parent
        anchors.margins: app.spacing

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
                    coordinate: gpsCoordinates
                    anchorPoint: Qt.point(sourceItem.width / 2, sourceItem.height/ 2)

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
                    name: "mapboxgl"
                }
            }
        }
    }
}
