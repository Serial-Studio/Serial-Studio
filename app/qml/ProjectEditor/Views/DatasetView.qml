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
