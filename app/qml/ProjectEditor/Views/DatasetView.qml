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

    //
    // Lazy-loaded alarm-band editor dialog. Activated via Cpp_JSON_ProjectEditor signal.
    //
    Loader {
      id: alarmBandsDialog

      active: false
      asynchronous: false
      source: "qrc:/serial-studio.com/gui/qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml"

      function show(gId, dId, rMin, rMax, bands) {
        alarmBandsDialog.active = true
        if (alarmBandsDialog.item) {
          alarmBandsDialog.item.closing.connect(() => alarmBandsDialog.active = false)
          alarmBandsDialog.item.showDialog(gId, dId, rMin, rMax, bands)
        }
      }
    }

    Connections {
      target: Cpp_JSON_ProjectEditor
      function onOpenAlarmBandsEditor(groupId, datasetId, rangeMin, rangeMax, bands) {
        alarmBandsDialog.show(groupId, datasetId, rangeMin, rangeMax, bands)
      }
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
        subtitleText: qsTr("Using fallback widgets. Buy a license to unlock full functionality.")
      }

      //
      // Dataset actions panel: a ribbon toolbar that collapses sections when the
      // window is narrower than the full set of buttons would need.
      //
      Rectangle {
        id: header

        z: 2
        Layout.fillWidth: true
        Layout.preferredHeight: 80
        color: Cpp_ThemeManager.colors["groupbox_background"]

        Widgets.RibbonToolbar {
          anchors.fill: parent
          anchors.leftMargin: 4
          anchors.rightMargin: 4
          secondaryToolbar: true
          showScrollFades: false

          //
          // Visualisations: time/freq plots
          //
          Widgets.RibbonSection {
            collapsible: true
            collapsePriority: 30
            collapsedText: qsTr("Plots")
            collapsedIcon: "qrc:/icons/project-editor/actions/plot.svg"

            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("Plot")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              icon.source: "qrc:/icons/project-editor/actions/plot.svg"
              ToolTip.text: qsTr("Toggle 2D plot visualization for this dataset")
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetPlot
              onClicked: {
                const option = SerialStudio.DatasetPlot
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
              }
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("FFT Plot")
              Layout.alignment: Qt.AlignVCenter
              icon.source: "qrc:/icons/project-editor/actions/fft.svg"
              ToolTip.text: qsTr("Toggle FFT plot to visualize frequency content")
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetFFT
              onClicked: {
                const option = SerialStudio.DatasetFFT
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
              }
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Waterfall")
              Layout.alignment: Qt.AlignVCenter
              icon.source: "qrc:/icons/project-editor/actions/waterfall.svg"
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetWaterfall
              ToolTip.text: qsTr("Toggle waterfall (spectrogram) plot — uses the FFT settings")
              onClicked: {
                const option = SerialStudio.DatasetWaterfall
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
              }
            }
          }

          //
          // Instantaneous-value widgets. No right separator: the spacer below
          // carries the boundary marker via its own showSeparator flag.
          //
          Widgets.RibbonSection {
            collapsible: true
            collapsePriority: 20
            showSeparator: false
            collapsedText: qsTr("Widgets")
            collapsedIcon: "qrc:/icons/project-editor/actions/gauge.svg"

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Bar/Level")
              Layout.alignment: Qt.AlignVCenter
              enabled: Cpp_JSON_ProjectEditor.currentDatasetIsEditable
              icon.source: "qrc:/icons/project-editor/actions/bar.svg"
              ToolTip.text: qsTr("Toggle bar/level indicator for this dataset")
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetBar
              onClicked: {
                const option = SerialStudio.DatasetBar
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
              }
            }

            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("Gauge")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              enabled: Cpp_JSON_ProjectEditor.currentDatasetIsEditable
              icon.source: "qrc:/icons/project-editor/actions/gauge.svg"
              ToolTip.text: qsTr("Toggle gauge widget for analog-style display")
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetGauge
              onClicked: {
                const option = SerialStudio.DatasetGauge
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
              }
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Compass")
              Layout.alignment: Qt.AlignVCenter
              enabled: Cpp_JSON_ProjectEditor.currentDatasetIsEditable
              icon.source: "qrc:/icons/project-editor/actions/compass.svg"
              ToolTip.text: qsTr("Toggle compass widget for directional data")
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetCompass
              onClicked: {
                const option = SerialStudio.DatasetCompass
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
              }
            }

            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("Meter")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              enabled: Cpp_JSON_ProjectEditor.currentDatasetIsEditable
              icon.source: "qrc:/icons/project-editor/actions/meter.svg"
              ToolTip.text: qsTr("Toggle analog meter (half-arc) widget")
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetMeter
              onClicked: {
                const option = SerialStudio.DatasetMeter
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
              }
            }

            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("LED")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              icon.source: "qrc:/icons/project-editor/actions/led.svg"
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetLED
              ToolTip.text: qsTr("Toggle LED indicator for binary or thresholded values")
              onClicked: {
                const option = SerialStudio.DatasetLED
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectModel.changeDatasetOption(option, !value)
              }
            }
          }

          //
          // Spacer
          //
          Widgets.RibbonSpacer {}

          //
          // Behaviour: alarm thresholds and value transforms
          //
          Widgets.RibbonSection {
            collapsible: true
            collapsePriority: 10
            collapsedText: qsTr("Behavior")
            collapsedIcon: "qrc:/icons/project-editor/actions/transform.svg"

            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("Alarm Bands")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              enabled: Cpp_JSON_ProjectEditor.currentDatasetIsEditable
                       && (Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetBar
                           || Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetGauge
                           || Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetMeter)
              onClicked: Cpp_JSON_ProjectEditor.openAlarmBandsEditorForSelection()
              icon.source: "qrc:/icons/project-editor/actions/alarm-bands.svg"
              ToolTip.text: qsTr("Define colored value ranges with severity tiers for this dataset's gauge.")
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Transform")
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_JSON_ProjectEditor.openTransformEditor()
              icon.source: "qrc:/icons/project-editor/actions/transform.svg"
              ToolTip.text: qsTr("Edit a value transform expression for calibration, filtering, or unit conversion")
            }
          }

          //
          // Lifecycle: always visible so the user can always duplicate / delete.
          //
          Widgets.RibbonSection {
            showSeparator: false

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Duplicate")
              Layout.alignment: Qt.AlignVCenter
              enabled: Cpp_JSON_ProjectEditor.currentDatasetIsEditable
              onClicked: Cpp_JSON_ProjectModel.duplicateCurrentDataset()
              icon.source: "qrc:/icons/project-editor/actions/duplicate.svg"
              ToolTip.text: qsTr("Duplicate this dataset with the same configuration")
            }

            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("Delete")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              ToolTip.text: qsTr("Delete this dataset from the group")
              onClicked: Cpp_JSON_ProjectModel.deleteCurrentDataset()
              enabled: Cpp_JSON_ProjectEditor.currentDatasetIsEditable
              icon.source: "qrc:/icons/project-editor/actions/delete.svg"
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
