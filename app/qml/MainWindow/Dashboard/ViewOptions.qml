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
          visible: Cpp_UI_Dashboard.axisOptionsWidgetVisible ||
                   Cpp_UI_Dashboard.pointsWidgetVisible ||
                   Cpp_UI_Dashboard.totalWidgetCount > 1

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
          visible: Cpp_UI_Dashboard.axisOptionsWidgetVisible ||
                   Cpp_UI_Dashboard.pointsWidgetVisible ||
                   Cpp_UI_Dashboard.totalWidgetCount > 1
        }

        //
        // Axis visibility
        //
        ComboBox {
          id: axisVisibility
          Layout.fillWidth: true
          visible: Cpp_UI_Dashboard.axisOptionsWidgetVisible

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
          visible: Cpp_UI_Dashboard.axisOptionsWidgetVisible ||
                   Cpp_UI_Dashboard.pointsWidgetVisible ||
                   Cpp_UI_Dashboard.totalWidgetCount > 1
        }

        //
        // Visualization controls
        //
        GridLayout {
          columns: 3
          rowSpacing: 4
          columnSpacing: 4
          visible: Cpp_UI_Dashboard.axisOptionsWidgetVisible ||
                   Cpp_UI_Dashboard.pointsWidgetVisible ||
                   Cpp_UI_Dashboard.totalWidgetCount > 1

          //
          // Number of plot points slider
          //
          Label {
            text: qsTr("Points:")
            visible: Cpp_UI_Dashboard.pointsWidgetVisible
          } Slider {
            id: plotPoints
            from: 0
            to: 100
            Layout.fillWidth: true
            value: logposition(100)
            visible: Cpp_UI_Dashboard.pointsWidgetVisible
            onValueChanged: {
              var log = logslider(value)
              if (Cpp_UI_Dashboard.points !== log)
                Cpp_UI_Dashboard.points = log
            }
          } Label {
            text: logslider(plotPoints.value)
            visible: Cpp_UI_Dashboard.pointsWidgetVisible
          }

          //
          // Number of decimal places
          //
          Label {
            text: qsTr("Decimal places:")
            visible: Cpp_UI_Dashboard.precisionWidgetVisible
          } Slider {
            id: decimalPlaces
            to: 6
            from: 0
            value: 2
            Layout.fillWidth: true
            visible: Cpp_UI_Dashboard.precisionWidgetVisible
            onValueChanged: Cpp_UI_Dashboard.precision = value
          } Label {
            text: Cpp_UI_Dashboard.precision
            visible: Cpp_UI_Dashboard.precisionWidgetVisible
          }

          //
          // Number of plot points slider
          //
          Label {
            text: qsTr("Columns:")
            visible: Cpp_UI_Dashboard.totalWidgetCount > 1
          } Slider {
            id: columns
            to: 10
            from: 1
            value: 3
            Layout.fillWidth: true
            visible: Cpp_UI_Dashboard.totalWidgetCount > 1
            onValueChanged: root.widgetColumns = Math.round(value)
            Component.onCompleted: root.widgetColumns = Math.round(value)
          } Label {
            text: Math.round(columns.value)
            visible: Cpp_UI_Dashboard.totalWidgetCount > 1
          }
        }

        //
        // Spacer
        //
        Item {
          implicitHeight: 8
          visible: Cpp_UI_Dashboard.axisOptionsWidgetVisible ||
                   Cpp_UI_Dashboard.pointsWidgetVisible ||
                   Cpp_UI_Dashboard.totalWidgetCount > 1
        }

        //
        // Groups & Widgets
        //
        Repeater {
          model: Cpp_UI_Dashboard.availableWidgets
          delegate: ViewOptionsDelegate {
            id: _viewDelegate
            count: Cpp_UI_Dashboard.widgetCount(modelData)
            colors: Cpp_UI_Dashboard.widgetColors(modelData)
            titles: Cpp_UI_Dashboard.widgetTitles(modelData)
            icon: Cpp_UI_Dashboard.availableWidgetIcons[index]
            title: Cpp_UI_Dashboard.availableWidgetTitles[index]
            onCheckedChanged: (index, checked) => Cpp_UI_Dashboard.setWidgetVisible(modelData, index, checked)

            Connections {
              target: Cpp_ThemeManager

              function onThemeChanged() {
                _viewDelegate.colors = Cpp_UI_Dashboard.widgetColors(modelData)
              }
            }
          }
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
