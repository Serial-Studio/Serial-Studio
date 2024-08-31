/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
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

import "../../Widgets" as Widgets

Widgets.Pane {
  title: qsTr("Group Editor")
  icon: "qrc:/rcc/icons/project-editor/windows/group.svg"

  property int groupIndex: 0

  //
  // User interface elements
  //
  Page {
    anchors.fill: parent
    palette.base: Cpp_ThemeManager.colors["base"]
    palette.text: Cpp_ThemeManager.colors["text"]
    palette.button: Cpp_ThemeManager.colors["button"]
    palette.window: Cpp_ThemeManager.colors["window"]
    palette.highlight: Cpp_ThemeManager.colors["highlight"]
    palette.buttonText: Cpp_ThemeManager.colors["button_text"]
    palette.placeholderText: Cpp_ThemeManager.colors["placeholder_text"]
    palette.highlightedText: Cpp_ThemeManager.colors["highlighted_text"]

    ColumnLayout {
      spacing: 0
      anchors {
        fill: parent
        topMargin: -16
        leftMargin: -9
        rightMargin: -9
        bottomMargin: -10
      }

      //
      // Group actions panel
      //
      Rectangle {
        z: 2
        Layout.fillWidth: true
        Layout.minimumHeight: 64
        Layout.maximumHeight: Layout.minimumHeight
        color: Cpp_ThemeManager.colors["groupbox_background"]

        //
        // Buttons
        //
        RowLayout {
          spacing: 4

          anchors {
            margins: 8
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }

          //
          // Add generic dataset
          //
          Widgets.BigButton {
            icon.width: 24
            icon.height: 24
            text: qsTr("Dataset")
            Layout.alignment: Qt.AlignVCenter
            palette.buttonText: Cpp_ThemeManager.colors["button_text"]
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-dataset.svg"
          }

          //
          // Add plot
          //
          Widgets.BigButton {
            icon.width: 24
            icon.height: 24
            text: qsTr("Plot")
            Layout.alignment: Qt.AlignVCenter
            palette.buttonText: Cpp_ThemeManager.colors["button_text"]
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-plot.svg"
          }

          //
          // Add FFT plot
          //
          Widgets.BigButton {
            icon.width: 24
            icon.height: 24
            text: qsTr("FFT Plot")
            Layout.alignment: Qt.AlignVCenter
            palette.buttonText: Cpp_ThemeManager.colors["button_text"]
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-fft.svg"
          }

          //
          // Add bar
          //
          Widgets.BigButton {
            icon.width: 24
            icon.height: 24
            text: qsTr("Bar/Level")
            Layout.alignment: Qt.AlignVCenter
            palette.buttonText: Cpp_ThemeManager.colors["button_text"]
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-bar.svg"
          }

          //
          // Add gauge
          //
          Widgets.BigButton {
            icon.width: 24
            icon.height: 24
            text: qsTr("Gauge")
            Layout.alignment: Qt.AlignVCenter
            palette.buttonText: Cpp_ThemeManager.colors["button_text"]
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-gauge.svg"
          }

          //
          // Add compass
          //
          Widgets.BigButton {
            icon.width: 24
            icon.height: 24
            text: qsTr("Compass")
            Layout.alignment: Qt.AlignVCenter
            palette.buttonText: Cpp_ThemeManager.colors["button_text"]
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-compass.svg"
          }

          //
          // Spacer
          //
          Item {
            Layout.fillWidth: true
          }


          //
          // Delete group
          //
          Widgets.BigButton {
            icon.width: 24
            icon.height: 24
            text: qsTr("Delete Group")
            Layout.alignment: Qt.AlignVCenter
            palette.buttonText: Cpp_ThemeManager.colors["button_text"]
            icon.source: "qrc:/rcc/icons/project-editor/actions/delete-group.svg"
          }
        }

        //
        // Bottom border
        //
        Rectangle {
          height: 1
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.bottom: parent.bottom
          color: Cpp_ThemeManager.colors["groupbox_border"]
        }
      }

      //
      // Datasets
      //
      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
      }
    }
  }
}
