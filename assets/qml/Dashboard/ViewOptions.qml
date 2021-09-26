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
            width: parent.width - 10 - 2 * app.spacing

            Item {
                height: app.spacing
            }

            //
            // Groups
            //
            RowLayout {
                spacing: app.spacing
                visible: Cpp_UI_Dashboard.groupCount > 0

                Image {
                    width: sourceSize.width
                    height: sourceSize.height
                    sourceSize: Qt.size(18, 18)
                    source: "qrc:/icons/group.svg"
                    ColorOverlay {
                        source: parent
                        color: palette.text
                        anchors.fill: parent
                    }
                } Label {
                    font.bold: true
                    text: qsTr("Data groups") + ":"
                } Item {
                    Layout.fillWidth: true
                }
            } Item {
                height: app.spacing / 2
            } Repeater {
                model: Cpp_UI_Dashboard.groupCount
                delegate: Switch {
                    checked: true
                    Layout.fillWidth: true
                    text: Cpp_UI_Dashboard.groupTitles()[index]
                    palette.highlight: Cpp_ThemeManager.alternativeHighlight
                    onCheckedChanged: Cpp_UI_Dashboard.setGroupVisible(index, checked)
                }
            }

            //
            // Spacer
            //
            Item {
                height: app.spacing
            }

            //
            // Plots
            //
            RowLayout {
                spacing: app.spacing
                visible: Cpp_UI_Dashboard.plotCount > 0

                Image {
                    width: sourceSize.width
                    height: sourceSize.height
                    sourceSize: Qt.size(18, 18)
                    source: "qrc:/icons/plot.svg"
                    ColorOverlay {
                        source: parent
                        color: palette.text
                        anchors.fill: parent
                    }
                } Label {
                    font.bold: true
                    text: qsTr("Plots") + ":"
                } Item {
                    Layout.fillWidth: true
                }
            } Item {
                height: app.spacing / 2
            } Repeater {
                model: Cpp_UI_Dashboard.plotCount
                delegate: Switch {
                    checked: true
                    Layout.fillWidth: true
                    text: Cpp_UI_Dashboard.plotTitles()[index]
                    palette.highlight: Cpp_ThemeManager.alternativeHighlight
                    onCheckedChanged: Cpp_UI_Dashboard.setPlotVisible(index, checked)
                }
            }

            //
            // Spacer
            //
            Item {
                height: app.spacing
            }

            //
            // Bars
            //
            RowLayout {
                spacing: app.spacing
                visible: Cpp_UI_Dashboard.barCount > 0

                Image {
                    width: sourceSize.width
                    height: sourceSize.height
                    sourceSize: Qt.size(18, 18)
                    source: "qrc:/icons/bar.svg"
                    ColorOverlay {
                        source: parent
                        color: palette.text
                        anchors.fill: parent
                    }
                } Label {
                    font.bold: true
                    text: qsTr("Bars") + ":"
                } Item {
                    Layout.fillWidth: true
                }
            } Item {
                height: app.spacing / 2
            } Repeater {
                model: Cpp_UI_Dashboard.barCount
                delegate: Switch {
                    checked: true
                    Layout.fillWidth: true
                    text: Cpp_UI_Dashboard.barTitles()[index]
                    palette.highlight: Cpp_ThemeManager.alternativeHighlight
                    onCheckedChanged: Cpp_UI_Dashboard.setBarVisible(index, checked)
                }
            }

            //
            // Spacer
            //
            Item {
                height: app.spacing
            }

            //
            // Gauges
            //
            RowLayout {
                spacing: app.spacing
                visible: Cpp_UI_Dashboard.gaugeCount > 0

                Image {
                    width: sourceSize.width
                    height: sourceSize.height
                    sourceSize: Qt.size(18, 18)
                    source: "qrc:/icons/gauge.svg"
                    ColorOverlay {
                        source: parent
                        color: palette.text
                        anchors.fill: parent
                    }
                } Label {
                    font.bold: true
                    text: qsTr("Gauges") + ":"
                } Item {
                    Layout.fillWidth: true
                }
            } Item {
                height: app.spacing / 2
            } Repeater {
                model: Cpp_UI_Dashboard.gaugeCount
                delegate: Switch {
                    checked: true
                    Layout.fillWidth: true
                    text: Cpp_UI_Dashboard.gaugeTitles()[index]
                    palette.highlight: Cpp_ThemeManager.alternativeHighlight
                    onCheckedChanged: Cpp_UI_Dashboard.setGaugeVisible(index, checked)
                }
            }

            //
            // Spacer
            //
            Item {
                height: app.spacing
            }

            //
            // Thermometers
            //
            RowLayout {
                spacing: app.spacing
                visible: Cpp_UI_Dashboard.thermometerCount > 0

                Image {
                    width: sourceSize.width
                    height: sourceSize.height
                    sourceSize: Qt.size(18, 18)
                    source: "qrc:/icons/thermometer.svg"
                    ColorOverlay {
                        source: parent
                        color: palette.text
                        anchors.fill: parent
                    }
                } Label {
                    font.bold: true
                    text: qsTr("Thermometers") + ":"
                } Item {
                    Layout.fillWidth: true
                }
            } Item {
                height: app.spacing / 2
            } Repeater {
                model: Cpp_UI_Dashboard.thermometerCount
                delegate: Switch {
                    checked: true
                    Layout.fillWidth: true
                    text: Cpp_UI_Dashboard.thermometerTitles()[index]
                    palette.highlight: Cpp_ThemeManager.alternativeHighlight
                    onCheckedChanged: Cpp_UI_Dashboard.setThermometerVisible(index, checked)
                }
            }

            //
            // Spacer
            //
            Item {
                height: app.spacing
            }

            //
            // Compasses
            //
            RowLayout {
                spacing: app.spacing
                visible: Cpp_UI_Dashboard.compassCount > 0

                Image {
                    width: sourceSize.width
                    height: sourceSize.height
                    sourceSize: Qt.size(18, 18)
                    source: "qrc:/icons/compass.svg"
                    ColorOverlay {
                        source: parent
                        color: palette.text
                        anchors.fill: parent
                    }
                } Label {
                    font.bold: true
                    text: qsTr("Compasses") + ":"
                } Item {
                    Layout.fillWidth: true
                }
            } Item {
                height: app.spacing / 2
            } Repeater {
                model: Cpp_UI_Dashboard.compassCount
                delegate: Switch {
                    checked: true
                    Layout.fillWidth: true
                    text: Cpp_UI_Dashboard.compassTitles()[index]
                    palette.highlight: Cpp_ThemeManager.alternativeHighlight
                    onCheckedChanged: Cpp_UI_Dashboard.setCompassVisible(index, checked)
                }
            }

            //
            // Spacer
            //
            Item {
                height: app.spacing
            }

            //
            // Gyroscopes
            //
            RowLayout {
                spacing: app.spacing
                visible: Cpp_UI_Dashboard.gyroscopeCount > 0

                Image {
                    width: sourceSize.width
                    height: sourceSize.height
                    sourceSize: Qt.size(18, 18)
                    source: "qrc:/icons/gyro.svg"
                    ColorOverlay {
                        source: parent
                        color: palette.text
                        anchors.fill: parent
                    }
                } Label {
                    font.bold: true
                    text: qsTr("Gyroscopes") + ":"
                } Item {
                    Layout.fillWidth: true
                }
            } Item {
                height: app.spacing / 2
            } Repeater {
                model: Cpp_UI_Dashboard.compassCount
                delegate: Switch {
                    checked: true
                    Layout.fillWidth: true
                    text: Cpp_UI_Dashboard.gyroscopeTitles()[index]
                    palette.highlight: Cpp_ThemeManager.alternativeHighlight
                    onCheckedChanged: Cpp_UI_Dashboard.setGyroscopeVisible(index, checked)
                }
            }

            //
            // Spacer
            //
            Item {
                height: app.spacing
            }

            //
            // Accelerometers
            //
            RowLayout {
                spacing: app.spacing
                visible: Cpp_UI_Dashboard.accelerometerCount > 0

                Image {
                    width: sourceSize.width
                    height: sourceSize.height
                    sourceSize: Qt.size(18, 18)
                    source: "qrc:/icons/accelerometer.svg"
                    ColorOverlay {
                        source: parent
                        color: palette.text
                        anchors.fill: parent
                    }
                } Label {
                    font.bold: true
                    text: qsTr("Accelerometers") + ":"
                } Item {
                    Layout.fillWidth: true
                }
            } Item {
                height: app.spacing / 2
            } Repeater {
                model: Cpp_UI_Dashboard.accelerometerCount
                delegate: Switch {
                    checked: true
                    Layout.fillWidth: true
                    text: Cpp_UI_Dashboard.accelerometerTitles()[index]
                    palette.highlight: Cpp_ThemeManager.alternativeHighlight
                    onCheckedChanged: Cpp_UI_Dashboard.setAccelerometerVisible(index, checked)
                }
            }

            //
            // Spacer
            //
            Item {
                height: app.spacing
            }

            //
            // Maps
            //
            RowLayout {
                spacing: app.spacing
                visible: Cpp_UI_Dashboard.mapCount > 0

                Image {
                    width: sourceSize.width
                    height: sourceSize.height
                    sourceSize: Qt.size(18, 18)
                    source: "qrc:/icons/map.svg"
                    ColorOverlay {
                        source: parent
                        color: palette.text
                        anchors.fill: parent
                    }
                } Label {
                    font.bold: true
                    text: qsTr("Maps") + ":"
                } Item {
                    Layout.fillWidth: true
                }
            } Item {
                height: app.spacing / 2
            } Repeater {
                model: Cpp_UI_Dashboard.mapCount
                delegate: Switch {
                    checked: true
                    Layout.fillWidth: true
                    text: Cpp_UI_Dashboard.mapTitles()[index]
                    palette.highlight: Cpp_ThemeManager.alternativeHighlight
                    onCheckedChanged: Cpp_UI_Dashboard.setMapVisible(index, checked)
                }
            }
        }
    }
}
