/*
 * Copyright (c) 2021 Alex Spataru <https://github.com/alex-spataru>
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

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0

import "../Widgets" as Widgets

Widgets.Window {
    id: root

    //
    // Window properties
    //
    gradient: true
    title: qsTr("View")
    headerDoubleClickEnabled: false
    icon.source: "qrc:/icons/visibility.svg"
    backgroundColor: Cpp_ThemeManager.embeddedWindowBackground

    //
    // Put all items inside a scrollview
    //
    ScrollView {
        clip: true
        contentWidth: -1
        anchors.fill: parent
        anchors.margins: app.spacing
        anchors.topMargin: root.borderWidth
        anchors.bottomMargin: root.borderWidth

        //
        // Main layout
        //
        ColumnLayout {
            x: app.spacing
            spacing: app.spacing / 2
            width: parent.width - 10 - 2 * app.spacing

            //
            // Spacer
            //
            Item {
                height: app.spacing
            }

            //
            // Groups
            //
            ViewOptionsDelegate {
                title: qsTr("Data groups")
                icon: "qrc:/icons/group.svg"
                count: Cpp_UI_Dashboard.groupCount
                titles: Cpp_UI_Dashboard.groupTitles
                onCheckedChanged: Cpp_UI_Dashboard.setGroupVisible(index, checked)
            }

            //
            // Plots
            //
            ViewOptionsDelegate {
                title: qsTr("Data plots")
                icon: "qrc:/icons/plot.svg"
                count: Cpp_UI_Dashboard.plotCount
                titles: Cpp_UI_Dashboard.plotTitles
                onCheckedChanged: Cpp_UI_Dashboard.setPlotVisible(index, checked)
            }

            //
            // Bars
            //
            ViewOptionsDelegate {
                title: qsTr("Bars")
                icon: "qrc:/icons/bar.svg"
                count: Cpp_UI_Dashboard.barCount
                titles: Cpp_UI_Dashboard.barTitles
                onCheckedChanged: Cpp_UI_Dashboard.setBarVisible(index, checked)
            }

            //
            // Gauges
            //
            ViewOptionsDelegate {
                title: qsTr("Gauges")
                icon: "qrc:/icons/gauge.svg"
                count: Cpp_UI_Dashboard.gaugeCount
                titles: Cpp_UI_Dashboard.gaugeTitles
                onCheckedChanged: Cpp_UI_Dashboard.setGaugeVisible(index, checked)
            }

            //
            // Thermometers
            //
            ViewOptionsDelegate {
                title: qsTr("Thermometers")
                icon: "qrc:/icons/thermometer.svg"
                count: Cpp_UI_Dashboard.thermometerCount
                titles: Cpp_UI_Dashboard.thermometerTitles
                onCheckedChanged: Cpp_UI_Dashboard.setThermometerVisible(index, checked)
            }

            //
            // Compasses
            //
            ViewOptionsDelegate {
                title: qsTr("Compasses")
                icon: "qrc:/icons/compass.svg"
                count: Cpp_UI_Dashboard.compassCount
                titles: Cpp_UI_Dashboard.compassTitles
                onCheckedChanged: Cpp_UI_Dashboard.setCompassVisible(index, checked)
            }

            //
            // Gyroscopes
            //
            ViewOptionsDelegate {
                title: qsTr("Gyroscopes")
                icon: "qrc:/icons/gyro.svg"
                count: Cpp_UI_Dashboard.gyroscopeCount
                titles: Cpp_UI_Dashboard.gyroscopeTitles
                onCheckedChanged: Cpp_UI_Dashboard.setGyroscopeVisible(index, checked)
            }

            //
            // Accelerometers
            //
            ViewOptionsDelegate {
                title: qsTr("Accelerometers")
                icon: "qrc:/icons/accelerometer.svg"
                count: Cpp_UI_Dashboard.accelerometerCount
                titles: Cpp_UI_Dashboard.accelerometerTitles
                onCheckedChanged: Cpp_UI_Dashboard.setAccelerometerVisible(index, checked)
            }

            //
            // Maps
            //
            ViewOptionsDelegate {
                title: qsTr("Maps")
                icon: "qrc:/icons/map.svg"
                count: Cpp_UI_Dashboard.mapCount
                titles: Cpp_UI_Dashboard.mapTitles
                onCheckedChanged: Cpp_UI_Dashboard.setMapVisible(index, checked)
            }
        }
    }
}
