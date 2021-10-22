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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt.labs.settings 1.0

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
    backgroundColor: Cpp_ThemeManager.paneWindowBackground

    //
    // Maps the slider position to points
    // https://stackoverflow.com/a/846249
    //
    function logslider(position) {
        var minp = 0
        var maxp = 100
        var minv = Math.log(10)
        var maxv = Math.log(10000)
        var scale = (maxv - minv) / (maxp - minp);
        return Math.exp(minv + scale * (position - minp)).toFixed(0);
    }

    //
    // Maps the points value to the slider position
    // https://stackoverflow.com/a/846249
    //
    function logposition(value) {
        var minp = 0
        var maxp = 100
        var minv = Math.log(10)
        var maxv = Math.log(10000)
        var scale = (maxv - minv) / (maxp - minp)
        var result = (Math.log(value) - minv) / scale + minp;
        return result.toFixed(0)
    }

    //
    // Settings
    //
    Settings {
        property alias points: dial.value
    }

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
            id: layout
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
            // Horizontal range title
            //
            RowLayout {
                spacing: app.spacing
                Layout.fillWidth: true
                visible: Cpp_UI_Dashboard.plotCount > 0 || Cpp_UI_Dashboard.multiPlotCount > 0

                Widgets.Icon {
                    width: 18
                    height: 18
                    color: palette.text
                    source: "qrc:/icons/points.svg"
                }

                Label {
                    font.bold: true
                    text: qsTr("Plot divisions (%1)").arg(logslider(dial.value))
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            //
            // Horizontal scale controls
            //
            Dial {
                id: dial
                to: 100
                from: 10
                value: logposition(100)
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                visible: Cpp_UI_Dashboard.plotCount > 0 || Cpp_UI_Dashboard.multiPlotCount > 0

                onValueChanged: {
                    var log = logslider(value)
                    if (Cpp_UI_Dashboard.points !== log)
                        Cpp_UI_Dashboard.points = log
                }
            }

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
                title: qsTr("Datasets")
                icon: "qrc:/icons/group.svg"
                count: Cpp_UI_Dashboard.groupCount
                titles: Cpp_UI_Dashboard.groupTitles
                onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setGroupVisible(index, checked)
            }

            //
            // Multiplots
            //
            ViewOptionsDelegate {
                title: qsTr("Multiple data plots")
                icon: "qrc:/icons/multiplot.svg"
                count: Cpp_UI_Dashboard.multiPlotCount
                titles: Cpp_UI_Dashboard.multiPlotTitles
                onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setMultiplotVisible(index, checked)
            }

            //
            // LEDs
            //
            ViewOptionsDelegate {
                title: qsTr("LED Panels")
                icon: "qrc:/icons/led.svg"
                count: Cpp_UI_Dashboard.ledCount
                titles: Cpp_UI_Dashboard.ledTitles
                onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setLedVisible(index, checked)
            }

            //
            // FFT
            //
            ViewOptionsDelegate {
                title: qsTr("FFT plots")
                icon: "qrc:/icons/fft.svg"
                count: Cpp_UI_Dashboard.fftCount
                titles: Cpp_UI_Dashboard.fftTitles
                onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setFFTVisible(index, checked)
            }

            //
            // Plots
            //
            ViewOptionsDelegate {
                title: qsTr("Data plots")
                icon: "qrc:/icons/plot.svg"
                count: Cpp_UI_Dashboard.plotCount
                titles: Cpp_UI_Dashboard.plotTitles
                onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setPlotVisible(index, checked)
            }

            //
            // Bars
            //
            ViewOptionsDelegate {
                title: qsTr("Bars")
                icon: "qrc:/icons/bar.svg"
                count: Cpp_UI_Dashboard.barCount
                titles: Cpp_UI_Dashboard.barTitles
                onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setBarVisible(index, checked)
            }

            //
            // Gauges
            //
            ViewOptionsDelegate {
                title: qsTr("Gauges")
                icon: "qrc:/icons/gauge.svg"
                count: Cpp_UI_Dashboard.gaugeCount
                titles: Cpp_UI_Dashboard.gaugeTitles
                onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setGaugeVisible(index, checked)
            }

            //
            // Compasses
            //
            ViewOptionsDelegate {
                title: qsTr("Compasses")
                icon: "qrc:/icons/compass.svg"
                count: Cpp_UI_Dashboard.compassCount
                titles: Cpp_UI_Dashboard.compassTitles
                onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setCompassVisible(index, checked)
            }

            //
            // Gyroscopes
            //
            ViewOptionsDelegate {
                title: qsTr("Gyroscopes")
                icon: "qrc:/icons/gyro.svg"
                count: Cpp_UI_Dashboard.gyroscopeCount
                titles: Cpp_UI_Dashboard.gyroscopeTitles
                onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setGyroscopeVisible(index, checked)
            }

            //
            // Accelerometers
            //
            ViewOptionsDelegate {
                title: qsTr("Accelerometers")
                icon: "qrc:/icons/accelerometer.svg"
                count: Cpp_UI_Dashboard.accelerometerCount
                titles: Cpp_UI_Dashboard.accelerometerTitles
                onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setAccelerometerVisible(index, checked)
            }

            //
            // Maps
            //
            ViewOptionsDelegate {
                title: qsTr("GPS")
                icon: "qrc:/icons/gps.svg"
                count: Cpp_UI_Dashboard.gpsCount
                titles: Cpp_UI_Dashboard.gpsTitles
                onCheckedChanged: (index, checked) =>  Cpp_UI_Dashboard.setGpsVisible(index, checked)
            }
        }
    }
}
