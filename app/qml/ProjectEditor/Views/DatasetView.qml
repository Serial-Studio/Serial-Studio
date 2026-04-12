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
  icon: Cpp_JSON_ProjectEditor.selectedIcon
  title: Cpp_JSON_ProjectEditor.selectedText

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
      leftMargin: -9
      topMargin: -16
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
        id: header

        z: 2
        Layout.fillWidth: true
        height: layout.implicitHeight + 12
        color: Cpp_ThemeManager.colors["groupbox_background"]

        //
        // Scrollable toolbar
        //
        Flickable {
          id: flickable

          clip: true
          contentHeight: height
          boundsBehavior: Flickable.StopAtBounds
          contentWidth: layout.implicitWidth + 16
          flickableDirection: Flickable.HorizontalFlick

          anchors {
            margins: 8
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            topMargin: 0
            bottomMargin: 0
          }

          height: layout.implicitHeight

          ScrollBar.horizontal: ScrollBar {
            height: 3
            policy: flickable.contentWidth > flickable.width
                    ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
          }

          //
          // Buttons
          //
          RowLayout {
            id: layout

            spacing: 4
            width: Math.max(implicitWidth, flickable.width)
            anchors.verticalCenter: parent.verticalCenter

          //
          // Add plot
          //
          Widgets.ToolbarButton {
            iconSize: 24
            onClicked: {
              const option = SerialStudio.DatasetPlot
              const value = Cpp_JSON_ProjectEditor.datasetOptions & option
              Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
            }
            text: qsTr("Plot")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/project-editor/actions/plot.svg"
            ToolTip.text: qsTr("Toggle 2D plot visualization for this dataset")
            checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetPlot
          }

          //
          // Add FFT plot
          //
          Widgets.ToolbarButton {
            iconSize: 24
            onClicked: {
              const option = SerialStudio.DatasetFFT
              const value = Cpp_JSON_ProjectEditor.datasetOptions & option
              Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
            }
            toolbarButton: false
            text: qsTr("FFT Plot")
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/project-editor/actions/fft.svg"
            ToolTip.text: qsTr("Toggle FFT plot to visualize frequency content")
            checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetFFT
          }

          //
          // Add bar
          //
          Widgets.ToolbarButton {
            iconSize: 24
            onClicked: {
              const option = SerialStudio.DatasetBar
              const value = Cpp_JSON_ProjectEditor.datasetOptions & option
              Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
            }
            toolbarButton: false
            text: qsTr("Bar/Level")
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectEditor.currentDatasetIsEditable
            icon.source: "qrc:/rcc/icons/project-editor/actions/bar.svg"
            ToolTip.text: qsTr("Toggle bar/level indicator for this dataset")
            checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetBar
          }

          //
          // Add gauge
          //
          Widgets.ToolbarButton {
            iconSize: 24
            onClicked: {
              const option = SerialStudio.DatasetGauge
              const value = Cpp_JSON_ProjectEditor.datasetOptions & option
              Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
            }
            text: qsTr("Gauge")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectEditor.currentDatasetIsEditable
            ToolTip.text: qsTr("Toggle gauge widget for analog-style display")
            icon.source: "qrc:/rcc/icons/project-editor/actions/gauge.svg"
            checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetGauge
          }

          //
          // Add compass
          //
          Widgets.ToolbarButton {
            iconSize: 24
            onClicked: {
              const option = SerialStudio.DatasetCompass
              const value = Cpp_JSON_ProjectEditor.datasetOptions & option
              Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
            }
            toolbarButton: false
            text: qsTr("Compass")
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectEditor.currentDatasetIsEditable
            ToolTip.text: qsTr("Toggle compass widget for directional data")
            icon.source: "qrc:/rcc/icons/project-editor/actions/compass.svg"
            checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetCompass
          }

          //
          // Add LED
          //
          Widgets.ToolbarButton {
            iconSize: 24
            onClicked: {
              const option = SerialStudio.DatasetLED
              const value = Cpp_JSON_ProjectEditor.datasetOptions & option
              Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
            }
            text: qsTr("LED")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            icon.source: "qrc:/rcc/icons/project-editor/actions/led.svg"
            ToolTip.text: qsTr("Toggle LED indicator for binary or thresholded values")
            checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetLED
          }

          //
          // Spacer
          //
          Item {
            Layout.fillWidth: true
            Layout.minimumWidth: 16
          }

          //
          // Transform code editor
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Transform")
            Layout.alignment: Qt.AlignVCenter
            onClicked: Cpp_JSON_ProjectEditor.openTransformEditor()
            icon.source: "qrc:/rcc/icons/project-editor/actions/transform.svg"
            ToolTip.text: qsTr("Edit a value transform expression for calibration, filtering, or unit conversion")
          }

          //
          // Separator
          //
          Rectangle {
            implicitWidth: 1
            Layout.fillHeight: true
            Layout.maximumHeight: 48
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          //
          // Duplicate dataset
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Duplicate")
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectEditor.currentDatasetIsEditable
            onClicked: Cpp_JSON_ProjectModel.duplicateCurrentDataset()
            icon.source: "qrc:/rcc/icons/project-editor/actions/duplicate.svg"
            ToolTip.text: qsTr("Duplicate this dataset with the same configuration")
          }

          //
          // Delete dataset
          //
          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Delete")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            ToolTip.text: qsTr("Delete this dataset from the group")
            onClicked: Cpp_JSON_ProjectModel.deleteCurrentDataset()
            enabled: Cpp_JSON_ProjectEditor.currentDatasetIsEditable
            icon.source: "qrc:/rcc/icons/project-editor/actions/delete.svg"
          }
        }
        }

        //
        // Left fade indicator
        //
        Rectangle {
          z: 10
          width: 16
          visible: flickable.contentX > 4
          anchors.left: flickable.left
          anchors.top: flickable.top
          anchors.bottom: flickable.bottom

          gradient: Gradient {
            orientation: Gradient.Horizontal

            GradientStop {
              position: 0
              color: Cpp_ThemeManager.colors["groupbox_background"]
            }

            GradientStop {
              position: 1
              color: "transparent"
            }
          }
        }

        //
        // Right fade indicator
        //
        Rectangle {
          z: 10
          width: 16
          visible: flickable.contentX + flickable.width < flickable.contentWidth - 4
          anchors.right: flickable.right
          anchors.top: flickable.top
          anchors.bottom: flickable.bottom

          gradient: Gradient {
            orientation: Gradient.Horizontal

            GradientStop {
              position: 0
              color: "transparent"
            }

            GradientStop {
              position: 1
              color: Cpp_ThemeManager.colors["groupbox_background"]
            }
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
            value: Cpp_JSON_ProjectEditor.datasetModel
          }
        }
      }
    }
  }
}
