/*
 * Copyright (c) 2020-2024 Alex Spataru <https://github.com/alex-spataru>
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

import SerialStudio
import "../../Widgets" as Widgets

Widgets.Pane {
  id: root
  implicitWidth: 0
  implicitHeight: 0
  icon: Cpp_JSON_ProjectModel.selectedIcon
  title: Cpp_JSON_ProjectModel.selectedText

  //
  // User interface elements
  //
  Page {
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

    anchors {
      fill: parent
      topMargin: -16
      leftMargin: -9
      rightMargin: -9
      bottomMargin: -10
    }

    //
    // Dataset actions panel
    //
    Rectangle {
      z: 2
      id: header
      height: layout.implicitHeight + 12
      color: Cpp_ThemeManager.colors["groupbox_background"]
      anchors {
        top: parent.top
        left: parent.left
        right: parent.right
      }

      //
      // Buttons
      //
      RowLayout {
        id: layout
        spacing: 4

        anchors {
          margins: 8
          left: parent.left
          right: parent.right
          verticalCenter: parent.verticalCenter
        }

        //
        // Add plot
        //
        Widgets.BigButton {
          iconSize: 24
          text: qsTr("Plot")
          toolbarButton: false
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/rcc/icons/project-editor/actions/plot.svg"
          checked: Cpp_JSON_ProjectModel.datasetOptions & SerialStudio.DatasetPlot
          onClicked: {
            const option = SerialStudio.DatasetPlot
            const value = Cpp_JSON_ProjectModel.datasetOptions & option
            Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
          }
        }

        //
        // Add FFT plot
        //
        Widgets.BigButton {
          iconSize: 24
          toolbarButton: false
          text: qsTr("FFT Plot")
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/rcc/icons/project-editor/actions/fft.svg"
          checked: Cpp_JSON_ProjectModel.datasetOptions & SerialStudio.DatasetFFT
          onClicked: {
            const option = SerialStudio.DatasetFFT
            const value = Cpp_JSON_ProjectModel.datasetOptions & option
            Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
          }
        }

        //
        // Add bar
        //
        Widgets.BigButton {
          iconSize: 24
          toolbarButton: false
          text: qsTr("Bar/Level")
          Layout.alignment: Qt.AlignVCenter
          enabled: Cpp_JSON_ProjectModel.currentDatasetIsEditable
          icon.source: "qrc:/rcc/icons/project-editor/actions/bar.svg"
          checked: Cpp_JSON_ProjectModel.datasetOptions & SerialStudio.DatasetBar
          onClicked: {
            const option = SerialStudio.DatasetBar
            const value = Cpp_JSON_ProjectModel.datasetOptions & option
            Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
          }
        }

        //
        // Add gauge
        //
        Widgets.BigButton {
          iconSize: 24
          text: qsTr("Gauge")
          toolbarButton: false
          Layout.alignment: Qt.AlignVCenter
          enabled: Cpp_JSON_ProjectModel.currentDatasetIsEditable
          icon.source: "qrc:/rcc/icons/project-editor/actions/gauge.svg"
          checked: Cpp_JSON_ProjectModel.datasetOptions & SerialStudio.DatasetGauge
          onClicked: {
            const option = SerialStudio.DatasetGauge
            const value = Cpp_JSON_ProjectModel.datasetOptions & option
            Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
          }
        }

        //
        // Add compass
        //
        Widgets.BigButton {
          iconSize: 24
          toolbarButton: false
          text: qsTr("Compass")
          Layout.alignment: Qt.AlignVCenter
          enabled: Cpp_JSON_ProjectModel.currentDatasetIsEditable
          icon.source: "qrc:/rcc/icons/project-editor/actions/compass.svg"
          checked: Cpp_JSON_ProjectModel.datasetOptions & SerialStudio.DatasetCompass
          onClicked: {
            const option = SerialStudio.DatasetCompass
            const value = Cpp_JSON_ProjectModel.datasetOptions & option
            Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
          }
        }

        //
        // Add LED
        //
        Widgets.BigButton {
          iconSize: 24
          text: qsTr("LED")
          toolbarButton: false
          Layout.alignment: Qt.AlignVCenter
          icon.source: "qrc:/rcc/icons/project-editor/actions/led.svg"
          checked: Cpp_JSON_ProjectModel.datasetOptions & SerialStudio.DatasetLED
          onClicked: {
            const option = SerialStudio.DatasetLED
            const value = Cpp_JSON_ProjectModel.datasetOptions & option
            Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
          }
        }

        //
        // Spacer
        //
        Item {
          Layout.fillWidth: true
        }

        //
        // Duplicate dataset
        //
        Widgets.BigButton {
          iconSize: 24
          toolbarButton: false
          text: qsTr("Duplicate")
          Layout.alignment: Qt.AlignVCenter
          enabled: Cpp_JSON_ProjectModel.currentDatasetIsEditable
          onClicked: Cpp_JSON_ProjectModel.duplicateCurrentDataset()
          icon.source: "qrc:/rcc/icons/project-editor/actions/duplicate.svg"
        }

        //
        // Delete dataset
        //
        Widgets.BigButton {
          iconSize: 24
          text: qsTr("Delete")
          toolbarButton: false
          Layout.alignment: Qt.AlignVCenter
          enabled: Cpp_JSON_ProjectModel.currentDatasetIsEditable
          onClicked: Cpp_JSON_ProjectModel.deleteCurrentDataset()
          icon.source: "qrc:/rcc/icons/project-editor/actions/delete.svg"
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
    // Dataset model editor
    //
    TableDelegate {
      anchors.fill: parent
      anchors.topMargin: header.height
      modelPointer: Cpp_JSON_ProjectModel.datasetModel
    }
  }
}
