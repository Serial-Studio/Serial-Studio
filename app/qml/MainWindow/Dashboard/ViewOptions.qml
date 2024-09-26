/*
 * Copyright (c) 2021-2023 Alex Spataru <https://github.com/alex-spataru>
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
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets

Widgets.Pane {
  id: root

  //
  // Window properties
  //
  title: qsTr("Widget Setup")
  icon: "qrc:/rcc/icons/panes/structure.svg"

  //
  // Signals
  //
  signal columnsChanged(var columns)

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
    property alias points: plotPoints.value
    property alias columns: columns.value
    property alias decimalPlaces: decimalPlaces.value
  }

  //
  // Use page item to set application palette
  //
  Page {
    anchors.fill: parent
    anchors.topMargin: -16
    anchors.leftMargin: -9
    anchors.rightMargin: -9
    anchors.bottomMargin: -9

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
      spacing: 0
      anchors.fill: parent

      //
      // Put most items inside a scrollview
      //
      ScrollView {
        clip: true
        contentWidth: -1
        Layout.leftMargin: 9
        Layout.fillWidth: true
        Layout.fillHeight: true

        ScrollBar.horizontal.interactive: false
        ScrollBar.vertical: ScrollBar {
          id: scrollbar
        }

        //
        // Main layout
        //
        ColumnLayout {
          x: 4
          id: layout
          spacing: 4
          width: parent.width - 24

          //
          // Spacer
          //
          Item {
            height: 10
          }

          //
          // View options title
          //
          RowLayout {
            spacing: 4
            Layout.fillWidth: true

            Image {
              sourceSize: Qt.size(18, 18)
              Layout.alignment: Qt.AlignVCenter
              source: "qrc:/rcc/icons/dashboard/view.svg"
            }

            Label {
              Layout.alignment: Qt.AlignVCenter
              text: qsTr("Visualization Options")
              font: Cpp_Misc_CommonFonts.customUiFont(10, true)
              color: Cpp_ThemeManager.colors["pane_section_label"]
              Component.onCompleted: font.capitalization = Font.AllUppercase
            }

            Item {
              Layout.fillWidth: true
            }
          }

          //
          // Spacer
          //
          Item {
            height: 4
          }

          //
          // Visualization controls
          //
          GridLayout {
            columns: 3
            rowSpacing: 4
            columnSpacing: 4

            //
            // Number of plot points slider
            //
            Label {
              text: qsTr("Points:")
              visible: Cpp_UI_Dashboard.plotCount > 0 || Cpp_UI_Dashboard.multiPlotCount > 0
            } Slider {
              id: plotPoints
              from: 0
              to: 100
              Layout.fillWidth: true
              value: logposition(100)
              visible: Cpp_UI_Dashboard.plotCount > 0 || Cpp_UI_Dashboard.multiPlotCount > 0
              onValueChanged: {
                var log = logslider(value)
                if (Cpp_UI_Dashboard.points !== log)
                  Cpp_UI_Dashboard.points = log
              }
            } Label {
              text: logslider(plotPoints.value)
              visible: Cpp_UI_Dashboard.plotCount > 0 || Cpp_UI_Dashboard.multiPlotCount > 0
            }

            //
            // Number of decimal places
            //
            Label {
              text: qsTr("Decimal places:")
              visible: Cpp_UI_Dashboard.groupCount > 0
            } Slider {
              id: decimalPlaces
              to: 6
              from: 0
              value: 2
              Layout.fillWidth: true
              visible: Cpp_UI_Dashboard.groupCount > 0
              onValueChanged: Cpp_UI_Dashboard.precision = value
            } Label {
              text: Cpp_UI_Dashboard.precision
              visible: Cpp_UI_Dashboard.groupCount > 0
            }

            //
            // Number of plot points slider
            //
            Label {
              text: qsTr("Columns:")
            } Slider {
              id: columns
              to: 5
              from: 1
              value: 3
              Layout.fillWidth: true
              onValueChanged: columnsChanged(Math.round(value))
            } Label {
              text: Math.round(columns.value)
            }
          }

          //
          // Spacer
          //
          Item {
            height: 8
          }

          //
          // Groups
          //
          ViewOptionsDelegate {
            colorfulSwitches: false
            title: qsTr("Data Grids")
            count: Cpp_UI_Dashboard.datagridCount
            titles: Cpp_UI_Dashboard.datagridTitles
            icon: "qrc:/rcc/icons/dashboard/datagrid.svg"
            onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setDataGridVisible(index, checked)
          }

          //
          // Multiplots
          //
          ViewOptionsDelegate {
            colorfulSwitches: false
            title: qsTr("Multiple Data Plots")
            count: Cpp_UI_Dashboard.multiPlotCount
            titles: Cpp_UI_Dashboard.multiPlotTitles
            icon: "qrc:/rcc/icons/dashboard/multiplot.svg"
            onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setMultiplotVisible(index, checked)
          }

          //
          // LEDs
          //
          ViewOptionsDelegate {
            colorfulSwitches: false
            title: qsTr("LED Panels")
            count: Cpp_UI_Dashboard.ledCount
            titles: Cpp_UI_Dashboard.ledTitles
            icon: "qrc:/rcc/icons/dashboard/led.svg"
            onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setLedVisible(index, checked)
          }

          //
          // FFT
          //
          ViewOptionsDelegate {
            title: qsTr("FFT Plots")
            count: Cpp_UI_Dashboard.fftCount
            titles: Cpp_UI_Dashboard.fftTitles
            icon: "qrc:/rcc/icons/dashboard/fft.svg"
            onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setFFTVisible(index, checked)
          }

          //
          // Plots
          //
          ViewOptionsDelegate {
            title: qsTr("Data Plots")
            count: Cpp_UI_Dashboard.plotCount
            titles: Cpp_UI_Dashboard.plotTitles
            icon: "qrc:/rcc/icons/dashboard/plot.svg"
            onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setPlotVisible(index, checked)
          }

          //
          // Bars
          //
          ViewOptionsDelegate {
            title: qsTr("Bars")
            count: Cpp_UI_Dashboard.barCount
            titles: Cpp_UI_Dashboard.barTitles
            icon: "qrc:/rcc/icons/dashboard/bar.svg"
            onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setBarVisible(index, checked)
          }

          //
          // Gauges
          //
          ViewOptionsDelegate {
            title: qsTr("Gauges")
            count: Cpp_UI_Dashboard.gaugeCount
            titles: Cpp_UI_Dashboard.gaugeTitles
            icon: "qrc:/rcc/icons/dashboard/gauge.svg"
            onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setGaugeVisible(index, checked)
          }

          //
          // Compasses
          //
          ViewOptionsDelegate {
            title: qsTr("Compasses")
            count: Cpp_UI_Dashboard.compassCount
            titles: Cpp_UI_Dashboard.compassTitles
            icon: "qrc:/rcc/icons/dashboard/compass.svg"
            onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setCompassVisible(index, checked)
          }

          //
          // Gyroscopes
          //
          ViewOptionsDelegate {
            colorfulSwitches: false
            title: qsTr("Gyroscopes")
            count: Cpp_UI_Dashboard.gyroscopeCount
            titles: Cpp_UI_Dashboard.gyroscopeTitles
            icon: "qrc:/rcc/icons/dashboard/gyroscope.svg"
            onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setGyroscopeVisible(index, checked)
          }

          //
          // Accelerometers
          //
          ViewOptionsDelegate {
            colorfulSwitches: false
            title: qsTr("Accelerometers")
            count: Cpp_UI_Dashboard.accelerometerCount
            titles: Cpp_UI_Dashboard.accelerometerTitles
            icon: "qrc:/rcc/icons/dashboard/accelerometer.svg"
            onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setAccelerometerVisible(index, checked)
          }

          //
          // Maps
          //
          ViewOptionsDelegate {
            title: qsTr("GPS")
            colorfulSwitches: false
            count: Cpp_UI_Dashboard.gpsCount
            titles: Cpp_UI_Dashboard.gpsTitles
            icon: "qrc:/rcc/icons/dashboard/gps.svg"
            onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setGpsVisible(index, checked)
          }
        }
      }

      //
      // Spacer
      //
      Item {
        Layout.fillHeight: true
      }

      //
      // Add buttons
      //
      Rectangle {
        height: 1
        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["groupbox_border"]
      } Rectangle {
        Layout.fillWidth: true
        height: buttons.implicitHeight + 18
        color: Cpp_ThemeManager.colors["groupbox_background"]

        ColumnLayout {
          id: buttons
          anchors.fill: parent
          anchors.margins: 9

          Button {
            Layout.fillWidth: true
            onClicked: Cpp_UI_Dashboard.resetData()

            RowLayout {
              id: layout1
              spacing: 8
              anchors.centerIn: parent
              width: Math.max(layout1.implicitWidth,
                              layout2.implicitWidth)

              Image {
                sourceSize: Qt.size(18, 18)
                Layout.alignment: Qt.AlignVCenter
                source: "qrc:/rcc/icons/panes/clear.svg"
              }

              Label {
                text: qsTr("Clear Dashboard Data")
                Layout.alignment: Qt.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                color: Cpp_ThemeManager.colors["button_text"]
              }
            }
          }

          Button {
            Layout.fillWidth: true
            onClicked: app.showExternalConsole()

            RowLayout {
              id: layout2
              spacing: 8
              anchors.centerIn: parent
              width: Math.max(layout1.implicitWidth,
                              layout2.implicitWidth)

              Image {
                sourceSize: Qt.size(18, 18)
                Layout.alignment: Qt.AlignVCenter
                source: "qrc:/rcc/icons/panes/console.svg"
              }

              Label {
                text: qsTr("Display Console Window")
                Layout.alignment: Qt.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                color: Cpp_ThemeManager.colors["button_text"]
              }
            }
          }
        }
      }
    }
  }
}
