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

import SerialStudio
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
  property int widgetColumns: Math.round(columns.value)

  //
  // Maps the slider position to points
  // https://stackoverflow.com/a/846249
  //
  function logslider(position) {
    var minp = 0;
    var maxp = 100;
    var minv = Math.log(10);
    var maxv = Math.log(10000);
    var scale = (maxv - minv) / (maxp - minp);
    var value = Math.exp(minv + scale * (position - minp));
    var roundedValue = Math.round(value / 10) * 10;
    return roundedValue.toFixed(0);
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
    property alias axisOptions: axisVisibility.currentIndex
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

    //
    // Widgets
    //
    Flickable {
      id: flickable
      anchors.fill: parent
      contentWidth: width
      contentHeight: layout.implicitHeight
      anchors.bottomMargin: buttonContainer.height

      ScrollBar.vertical: ScrollBar {
        id: scroll
      }

      //
      // Main layout
      //
      ColumnLayout {
        id: layout
        spacing: 4
        anchors.leftMargin: 8
        anchors.rightMargin: 24
        anchors.left: parent.left
        anchors.right: parent.right

        //
        // Spacer
        //
        Item {
          implicitHeight: 10
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
          implicitHeight: 4
        }

        //
        // Axis visibility
        //
        ComboBox {
          id: axisVisibility
          Layout.fillWidth: true
          model: [
            qsTr("Show both X and Y axes"),
            qsTr("Show only X axis"),
            qsTr("Show only Y axis"),
            qsTr("Hide all axes")
          ]

          currentIndex: {
            switch (Cpp_UI_Dashboard.axisVisibility) {
            case Dashboard.AxisXY:
              return 0;
            case Dashboard.AxisX:
              return 1;
            case Dashboard.AxisY:
              return 2;
            case Dashboard.NoAxesVisible:
              return 3;
            }
          }

          visible: Cpp_UI_Dashboard.plotCount > 0 || Cpp_UI_Dashboard.fftCount > 0 || Cpp_UI_Dashboard.multiPlotCount > 0

          onCurrentIndexChanged: {
            switch (currentIndex) {
            case 0:
              Cpp_UI_Dashboard.axisVisibility = Dashboard.AxisXY
              break
            case 1:
              Cpp_UI_Dashboard.axisVisibility = Dashboard.AxisX
              break
            case 2:
              Cpp_UI_Dashboard.axisVisibility = Dashboard.AxisY
              break
            case 3:
              Cpp_UI_Dashboard.axisVisibility = Dashboard.NoAxesVisible
              break
            }
          }
        }

        //
        // Spacer
        //
        Item {
          implicitHeight: 2
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
          } Slider {
            id: decimalPlaces
            to: 6
            from: 0
            value: 2
            Layout.fillWidth: true
            onValueChanged: Cpp_UI_Dashboard.precision = value
          } Label {
            text: Cpp_UI_Dashboard.precision
          }

          //
          // Number of plot points slider
          //
          Label {
            text: qsTr("Columns:")
          } Slider {
            id: columns
            to: 10
            from: 1
            value: 3
            Layout.fillWidth: true
            onValueChanged: root.widgetColumns = Math.round(value)
            Component.onCompleted: root.widgetColumns = Math.round(value)
          } Label {
            text: Math.round(columns.value)
          }
        }

        //
        // Spacer
        //
        Item {
          implicitHeight: 8
        }

        //
        // Groups
        //
        ViewOptionsDelegate {
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
          colors: Cpp_UI_Dashboard.fftColors
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
          colors: Cpp_UI_Dashboard.plotColors
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
          colors: Cpp_UI_Dashboard.barColors
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
          colors: Cpp_UI_Dashboard.gaugeColors
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
          colors: Cpp_UI_Dashboard.compassColors
          titles: Cpp_UI_Dashboard.compassTitles
          icon: "qrc:/rcc/icons/dashboard/compass.svg"
          onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setCompassVisible(index, checked)
        }

        //
        // Gyroscopes
        //
        ViewOptionsDelegate {
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
          count: Cpp_UI_Dashboard.gpsCount
          titles: Cpp_UI_Dashboard.gpsTitles
          icon: "qrc:/rcc/icons/dashboard/gps.svg"
          onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setGpsVisible(index, checked)
        }
      }
    }

    //
    // Add buttons
    //
    ColumnLayout {
      id: buttonContainer

      spacing: 0
      height: implicitHeight
      anchors {
        left: parent.left
        right: parent.right
        bottom: parent.bottom
      }

      Rectangle {
        implicitHeight: 1
        Layout.fillWidth: true
        color: Cpp_ThemeManager.colors["groupbox_border"]
      } Rectangle {
        Layout.fillWidth: true
        implicitHeight: buttons.implicitHeight + 18
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
                              layout2.implicitWidth,
                              layout3.implicitWidth)

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

              Item {
                Layout.fillWidth: true
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
                              layout2.implicitWidth,
                              layout3.implicitWidth)

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

              Item {
                Layout.fillWidth: true
              }
            }
          }

          Button {
            opacity: 0.5
            enabled: false
            Layout.fillWidth: true
            visible: false //Cpp_UI_Dashboard.plotCount > 0 || Cpp_UI_Dashboard.multiPlotCount > 0

            RowLayout {
              id: layout3
              spacing: 8
              anchors.centerIn: parent
              width: Math.max(layout1.implicitWidth,
                              layout2.implicitWidth,
                              layout3.implicitWidth)

              Image {
                sourceSize: Qt.size(18, 18)
                Layout.alignment: Qt.AlignVCenter
                source: "qrc:/rcc/icons/panes/scale.svg"
                visible: Cpp_UI_Dashboard.plotCount > 0 || Cpp_UI_Dashboard.multiPlotCount > 0
              }

              Label {
                Layout.alignment: Qt.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                text: qsTr("Adjust Plot Scales & Positions")
                color: Cpp_ThemeManager.colors["button_text"]
                visible: Cpp_UI_Dashboard.plotCount > 0 || Cpp_UI_Dashboard.multiPlotCount > 0
              }

              Item {
                Layout.fillWidth: true
              }
            }
          }
        }
      }
    }
  }
}
