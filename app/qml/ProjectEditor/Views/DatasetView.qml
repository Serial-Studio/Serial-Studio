/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
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

    ColumnLayout {
      spacing: 0
      anchors.fill: parent

      //
      // Pro notice
      //
      Widgets.ProNotice {
        Layout.margins: -1
        Layout.bottomMargin: 0
        Layout.fillWidth: true
        closeButtonEnabled: false
        titleText: qsTr("Pro features detected in this project.")
        activationFlag: Cpp_JSON_ProjectModel.containsCommercialFeatures
        subtitleText: qsTr("Fallback widgets will be used. Buy a license to unlock full functionality.")
      }

      //
      // Dataset actions panel
      //
      Rectangle {
        z: 2
        id: header
        Layout.fillWidth: true
        height: layout.implicitHeight + 12
        color: Cpp_ThemeManager.colors["groupbox_background"]

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
          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Plot")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/project-editor/actions/plot.svg"
            checked: Cpp_JSON_ProjectModel.datasetOptions & SerialStudio.DatasetPlot
            ToolTip.text: qsTr("Toggle 2D plot visualization for this dataset")
            onClicked: {
              const option = SerialStudio.DatasetPlot
              const value = Cpp_JSON_ProjectModel.datasetOptions & option
              Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
            }
          }

          //
          // Add FFT plot
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("FFT Plot")
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/project-editor/actions/fft.svg"
            checked: Cpp_JSON_ProjectModel.datasetOptions & SerialStudio.DatasetFFT
            ToolTip.text: qsTr("Toggle FFT plot to visualize frequency content")
            onClicked: {
              const option = SerialStudio.DatasetFFT
              const value = Cpp_JSON_ProjectModel.datasetOptions & option
              Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
            }
          }

          //
          // Add bar
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Bar/Level")
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectModel.currentDatasetIsEditable
            icon.source: "qrc:/rcc/icons/project-editor/actions/bar.svg"
            ToolTip.text: qsTr("Toggle bar/level indicator for this dataset")
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
          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Gauge")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectModel.currentDatasetIsEditable
            icon.source: "qrc:/rcc/icons/project-editor/actions/gauge.svg"
            ToolTip.text: qsTr("Toggle gauge widget for analog-style display")
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
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Compass")
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectModel.currentDatasetIsEditable
            icon.source: "qrc:/rcc/icons/project-editor/actions/compass.svg"
            ToolTip.text: qsTr("Toggle compass widget for directional data")
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
          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("LED")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/project-editor/actions/led.svg"
            ToolTip.text: qsTr("Toggle LED indicator for binary or thresholded values")
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
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Duplicate")
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectModel.currentDatasetIsEditable
            onClicked: Cpp_JSON_ProjectModel.duplicateCurrentDataset()
            ToolTip.text: qsTr("Duplicate this dataset with the same configuration")
            icon.source: "qrc:/rcc/icons/project-editor/actions/duplicate.svg"
          }

          //
          // Delete dataset
          //
          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Delete")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectModel.currentDatasetIsEditable
            onClicked: Cpp_JSON_ProjectModel.deleteCurrentDataset()
            ToolTip.text: qsTr("Delete this dataset from the group")
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
      ScrollView {
        id: view
        contentWidth: width
        Layout.fillWidth: true
        Layout.fillHeight: true
        contentHeight: delegate.implicitHeight
        ScrollBar.vertical.policy: delegate.implicitHeight > view.height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

        TableDelegate {
          id: delegate
          width: parent.width
          headerVisible: false

          Binding {
            target: delegate
            property: "modelPointer"
            value: Cpp_JSON_ProjectModel.datasetModel
          }
        }
      }
    }
  }
}
