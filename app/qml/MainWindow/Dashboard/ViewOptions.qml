/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
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
  // Expose widget aspect ratio
  //
  property real aspectRatio: aspectComboBox.model[0].value

  //
  // Generate the list of allowed point values for the slider
  // These are the values we want the user to be able to pick:
  //  - From 10 to 100 → steps of 10
  //  - From 200 to 1000 → steps of 100
  //  - From 2000 to 10000 → steps of 1000
  //
  readonly property var allowedPoints: {
    var i = 0;
    var result = [];

    for (i = 10; i <= 100; i += 10)
      result.push(i);
    for (i = 200; i <= 1000; i += 100)
      result.push(i);
    for (i = 2000; i <= 10000; i += 1000)
      result.push(i);

    return result;
  }

  //
  // Precompute the positions that correspond to each allowed point value.
  // Why? Because the slider is on a log scale (0–100), not a linear scale.
  // We convert each point value to a slider position using the formula:
  //    pos = (log(value) - log(min)) * scale
  //
  readonly property var allowedPositions: {
    var allowed = root.allowedPoints;
    var minv = Math.log(10);
    var maxv = Math.log(10000);
    var scale = 100 / (maxv - minv);
    var positions = [];

    for (var i = 0; i < allowed.length; i++) {
      var value = allowed[i];
      var pos = (Math.log(value) - minv) * scale;
      positions.push({ value: value, position: pos });
    }

    return positions;
  }

  //
  // Given a slider position (0–100), find the closest allowed position from
  // the precomputed list. This is used to snap the slider to valid values as
  // the user moves it.
  //
  function snapToAllowedValue(position) {
    var positions = root.allowedPositions;
    var closest = positions[0];
    var minDiff = Math.abs(position - closest.position);

    for (var i = 1; i < positions.length; i++) {
      var diff = Math.abs(position - positions[i].position);
      if (diff < minDiff) {
        closest = positions[i];
        minDiff = diff;
      }
    }

    return closest;
  }

  //
  // Settings
  //
  Settings {
    category: "Dashboard"
    property alias points: plotPoints.value
    property alias showLegends: legends.checked
    property alias decimalPlaces: decimalPlaces.value
    property alias showCrosshairs: crosshairs.checked
    property alias axisOptions: axisVisibility.currentIndex
    property alias showAreaUnderPlot: areaUnderPlot.checked
    property alias widgetAspectRatio: aspectComboBox.currentIndex
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
        anchors.rightMargin: 8
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
            font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          }

          Item {
            Layout.fillWidth: true
          }
        }

        //
        // Rectangle with view options
        //
        Rectangle {
          radius: 2
          border.width: 1
          Layout.fillWidth: true
          implicitHeight: viewOptionsLayout.implicitHeight + 16
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]

          visible: Cpp_UI_Dashboard.axisOptionsWidgetVisible ||
                   Cpp_UI_Dashboard.pointsWidgetVisible ||
                   Cpp_UI_Dashboard.totalWidgetCount > 1

          ColumnLayout {
            spacing: 4
            anchors.margins: 8
            anchors.fill: parent
            id: viewOptionsLayout

            //
            // Widget aspect ratio
            //
            ComboBox {
              id: aspectComboBox

              model: [
                  { label: qsTr("4:3 (Standard)"),      value: 4 / 3 },
                  { label: qsTr("1:1 (Square)"),        value: 1 / 1 },
                  { label: qsTr("16:9 (Widescreen)"),   value: 16 / 9 },
                  { label: qsTr("3:4 (Portrait)"),      value: 3 / 4 },
                  { label: qsTr("9:16 (Vertical)"),     value: 9 / 16 }
              ]

              textRole: "label"
              Layout.fillWidth: true
              displayText: qsTr("Aspect: %1").arg(currentText)
              onCurrentIndexChanged: root.aspectRatio = model[currentIndex].value
            }


            //
            // Axis visibility
            //
            ComboBox {
              id: axisVisibility
              Layout.fillWidth: true
              displayText: qsTr("Axes: %1").arg(currentText)
              visible: Cpp_UI_Dashboard.axisOptionsWidgetVisible

              model: [
                qsTr("Both"),
                qsTr("X only"),
                qsTr("Y only"),
                qsTr("None")
              ]

              currentIndex: {
                switch (Cpp_UI_Dashboard.axisVisibility) {
                case SerialStudio.AxisXY:
                  return 0;
                case SerialStudio.AxisX:
                  return 1;
                case SerialStudio.AxisY:
                  return 2;
                case SerialStudio.NoAxesVisible:
                  return 3;
                }
              }

              onCurrentIndexChanged: {
                switch (currentIndex) {
                case 0:
                  Cpp_UI_Dashboard.axisVisibility = SerialStudio.AxisXY
                  break
                case 1:
                  Cpp_UI_Dashboard.axisVisibility = SerialStudio.AxisX
                  break
                case 2:
                  Cpp_UI_Dashboard.axisVisibility = SerialStudio.AxisY
                  break
                case 3:
                  Cpp_UI_Dashboard.axisVisibility = SerialStudio.NoAxesVisible
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
                rightPadding: 8
                text: qsTr("Points:")
                visible: Cpp_UI_Dashboard.pointsWidgetVisible
              } Slider {
                id: plotPoints
                from: 0
                to: 100
                stepSize: 0.1
                implicitHeight: 18
                Layout.fillWidth: true
                visible: Cpp_UI_Dashboard.pointsWidgetVisible

                value: {
                  var val = Cpp_UI_Dashboard.points;
                  var minv = Math.log(10);
                  var maxv = Math.log(10000);
                  var scale = 100 / (maxv - minv);
                  return (Math.log(val) - minv) * scale;
                }

                onMoved: {
                  var snapped = snapToAllowedValue(value);
                  value = snapped.position;

                  if (Cpp_UI_Dashboard.points !== snapped.value)
                    Cpp_UI_Dashboard.points = snapped.value;
                }

                onValueChanged: {
                  var snapped = snapToAllowedValue(value);
                  if (Cpp_UI_Dashboard.points !== snapped.value)
                    Cpp_UI_Dashboard.points = snapped.value;
                }
              } Label {
                text: snapToAllowedValue(plotPoints.value).value
                visible: Cpp_UI_Dashboard.pointsWidgetVisible
              }

              //
              // Number of decimal places
              //
              Label {
                rightPadding: 8
                text: qsTr("Decimal places:")
                visible: Cpp_UI_Dashboard.precisionWidgetVisible
              } Slider {
                id: decimalPlaces
                to: 6
                from: 0
                value: 2
                implicitHeight: 18
                Layout.fillWidth: true
                visible: Cpp_UI_Dashboard.precisionWidgetVisible
                onValueChanged: Cpp_UI_Dashboard.precision = value
              } Label {
                text: Cpp_UI_Dashboard.precision
                visible: Cpp_UI_Dashboard.precisionWidgetVisible
              }

              //
              // Show Legends
              //
              Label {
                rightPadding: 8
                text: qsTr("Show Legends:")
                visible: Cpp_UI_Dashboard.totalWidgetCount > 0 && Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardMultiPlot) >= 1
              } CheckBox {
                id: legends
                implicitHeight: 18
                Layout.leftMargin: -8
                Layout.alignment: Qt.AlignLeft
                checked: Cpp_UI_Dashboard.showLegends
                visible: Cpp_UI_Dashboard.totalWidgetCount > 0 && Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardMultiPlot) >= 1
                onCheckedChanged: {
                  if (checked !== Cpp_UI_Dashboard.showLegends)
                    Cpp_UI_Dashboard.showLegends = checked
                }
              } Item {
                visible: Cpp_UI_Dashboard.totalWidgetCount > 0 && Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardMultiPlot) >= 1
              }

              //
              // Show crosshairs
              //
              Label {
                rightPadding: 8
                text: qsTr("Show Crosshairs:")
                visible: Cpp_UI_Dashboard.totalWidgetCount > 0 &&
                         (Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardFFT) >= 1 ||
                          Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardPlot) >= 1 ||
                          Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardMultiPlot) >= 1)
              } CheckBox {
                id: crosshairs
                implicitHeight: 18
                Layout.leftMargin: -8
                Layout.alignment: Qt.AlignLeft
                checked: Cpp_UI_Dashboard.showCrosshairs
                visible: Cpp_UI_Dashboard.totalWidgetCount > 0 &&
                         (Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardFFT) >= 1 ||
                          Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardPlot) >= 1 ||
                          Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardMultiPlot) >= 1)
                onCheckedChanged: {
                  if (checked !== Cpp_UI_Dashboard.showCrosshairs)
                    Cpp_UI_Dashboard.showCrosshairs = checked
                }
              } Item {
                visible: Cpp_UI_Dashboard.totalWidgetCount > 0 &&
                         (Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardFFT) >= 1 ||
                          Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardPlot) >= 1 ||
                          Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardMultiPlot) >= 1)
              }

              //
              // Show Area Under Plot
              //
              Label {
                rightPadding: 8
                text: qsTr("Show Area Under Plots:")
                visible: Cpp_UI_Dashboard.totalWidgetCount > 0 &&
                         (Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardPlot) >= 1 ||
                          Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardFFT) >= 1)
              } CheckBox {
                id: areaUnderPlot
                implicitHeight: 18
                Layout.leftMargin: -8
                Layout.alignment: Qt.AlignLeft
                checked: Cpp_UI_Dashboard.showAreaUnderPlots
                visible: Cpp_UI_Dashboard.totalWidgetCount > 0 &&
                         (Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardPlot) >= 1 ||
                          Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardFFT) >= 1)
                onCheckedChanged: {
                  if (checked !== Cpp_UI_Dashboard.showAreaUnderPlots)
                    Cpp_UI_Dashboard.showAreaUnderPlots = checked
                }
              } Item {
                visible: Cpp_UI_Dashboard.totalWidgetCount > 0 &&
                         (Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardPlot) >= 1 ||
                          Cpp_UI_Dashboard.widgetCount(SerialStudio.DashboardFFT) >= 1)
              }
            }
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
            opacity: enabled ? 1 : 0.5
            checked: Cpp_IO_Manager.paused
            enabled: Cpp_IO_Manager.isConnected
            onClicked: Cpp_IO_Manager.paused = !Cpp_IO_Manager.paused

            RowLayout {
              id: layout1
              spacing: 8
              anchors.centerIn: parent
              width: Math.max(layout1.implicitWidth,
                              layout2.implicitWidth,
                              layout3.implicitWidth,
                              layout4.implicitWidth)

              Image {
                sourceSize: Qt.size(18, 18)
                Layout.alignment: Qt.AlignVCenter
                source: Cpp_IO_Manager.paused ? "qrc:/rcc/icons/panes/resume.svg" :
                                                "qrc:/rcc/icons/panes/pause.svg"
              }

              Label {
                Layout.alignment: Qt.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                font: Cpp_Misc_CommonFonts.boldUiFont
                color: Cpp_ThemeManager.colors["button_text"]
                text: Cpp_IO_Manager.paused ? qsTr("Resume (%1)").arg(shortcutText) :
                                              qsTr("Pause (%1)").arg(shortcutText)

                property string shortcutText: {
                  var os = Qt.platform.os
                  if (os === "osx")
                    return qsTr("⌘ + P")
                  else if (os === "windows")
                    return qsTr("Ctrl + P")
                  else
                    return qsTr("Ctrl + P")
                }
              }

              Item {
                Layout.fillWidth: true
              }
            }
          }

          Button {
            Layout.fillWidth: true
            onClicked: Cpp_UI_Dashboard.resetData()

            RowLayout {
              id: layout2
              spacing: 8
              anchors.centerIn: parent
              width: Math.max(layout1.implicitWidth,
                              layout2.implicitWidth,
                              layout3.implicitWidth,
                              layout4.implicitWidth)

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
              id: layout3
              spacing: 8
              anchors.centerIn: parent
              width: Math.max(layout1.implicitWidth,
                              layout2.implicitWidth,
                              layout3.implicitWidth,
                              layout4.implicitWidth)

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
              id: layout4
              spacing: 8
              anchors.centerIn: parent
              width: Math.max(layout1.implicitWidth,
                              layout2.implicitWidth,
                              layout3.implicitWidth,
                              layout4.implicitWidth)

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
