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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
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

  actionComponent: EditorNavActions {}

  //
  // User interface elements
  //
  Page {
    enabled: Cpp_JSON_ProjectEditor.selectedDatasetEnabled
    opacity: Cpp_JSON_ProjectEditor.selectedDatasetEnabled ? 1 : 0.5

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
      // Search band: above the secondary toolbar, aligned with the tree search
      //
      EditorSearchBand {
        tableDelegate: delegate
      }

      //
      // Pro notice
      //
      Widgets.ProNotice {
        Layout.margins: -1
        Layout.bottomMargin: 0
        Layout.fillWidth: true
        closeButtonEnabled: false
        items: Cpp_JSON_ProjectModel.commercialFeatureSummary
        activationFlag: Cpp_JSON_ProjectModel.containsCommercialFeatures
      }

      //
      // Computed dataset explanation (spec 0100 R14), next to the greyed frame index
      //
      Rectangle {
        Layout.fillWidth: true
        Layout.margins: -1
        Layout.bottomMargin: 0
        border.width: 1
        implicitHeight: computedNote.implicitHeight + 16
        visible: Cpp_JSON_ProjectEditor.selectedDatasetComputed
        color: Cpp_ThemeManager.colors["notebox_background"]
        border.color: Cpp_ThemeManager.colors["notebox_border"]

        RowLayout {
          id: computedNote

          spacing: 12
          anchors.margins: 8
          anchors.fill: parent

          Image {
            sourceSize.width: 32
            Layout.alignment: Qt.AlignVCenter
            source: Cpp_Misc_IconRegistry.icon("editor", "transform", 32)
          }

          Label {
            Layout.fillWidth: true
            wrapMode: Label.WordWrap
            Layout.alignment: Qt.AlignVCenter
            color: Cpp_ThemeManager.colors["notebox_text"]
            text: qsTr("This is a computed dataset: its transform calculates the value, so it does "
                       + "not read a position in the frame. A dataset becomes computed when its "
                       + "transform does not use the incoming value.")
          }
        }
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
            collapsedIcon: Cpp_Misc_IconRegistry.icon("editor", "plot", 24)

            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("Plot")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "plot", 24)
              ToolTip.text: qsTr("Show or hide a plot of this value over time.")
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
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "fft", 24)
              ToolTip.text: qsTr("Show or hide a plot of the signal's frequencies.")
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
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "waterfall", 24)
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetWaterfall
              ToolTip.text: qsTr("Show or hide a scrolling picture of the frequencies over time. "
                                 + "Uses the FFT settings.")
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
            collapsedIcon: Cpp_Misc_IconRegistry.icon("widgets", "gauge", 32)

            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("Bar")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              enabled: Cpp_JSON_ProjectEditor.currentDatasetIsEditable
              ToolTip.text: qsTr("Show or hide a bar for this value.")
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "bar", 24)
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
              icon.source: Cpp_Misc_IconRegistry.icon("widgets", "gauge", 24)
              ToolTip.text: qsTr("Show or hide a round gauge for this value.")
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
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "compass", 24)
              ToolTip.text: qsTr("Show or hide a compass for a direction value.")
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
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "meter", 24)
              ToolTip.text: qsTr("Show or hide a half-circle meter for this value.")
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetMeter
              onClicked: {
                const option = SerialStudio.DatasetMeter
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
            collapsedIcon: Cpp_Misc_IconRegistry.icon("editor", "transform", 32)

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Alarm Bands")
              Layout.alignment: Qt.AlignVCenter
              enabled: Cpp_JSON_ProjectEditor.alarmBandsApplicable
              onClicked: Cpp_JSON_ProjectEditor.openAlarmBandsEditorForSelection()
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "alarm-bands", 24)
              ToolTip.text: qsTr("Color ranges of the value, such as normal, warning and "
                                 + "critical, and choose which ones raise alarms. Available for "
                                 + "Bar, Gauge, Meter, LED, and Bar Panel datasets.")
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Freq. Markers")
              Layout.alignment: Qt.AlignVCenter
              enabled: Cpp_JSON_ProjectEditor.frequencyMarkersApplicable
              onClicked: Cpp_JSON_ProjectEditor.openFrequencyMarkersEditorForSelection()
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "frequency-markers", 24)
              ToolTip.text: qsTr("Mark named frequencies or ranges on the FFT plot and waterfall, "
                                 + "with optional warning and alarm levels.")
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Labels")
              objectName: "datasetView.labels"
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_JSON_ProjectEditor.openValueLabelsEditorForSelection()
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "dataset-values", 24)
              ToolTip.text: qsTr("Show words instead of numbers, such as Running for 1. Plots and "
                                 + "alarms keep the number.")
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Transform")
              Layout.alignment: Qt.AlignVCenter
              onClicked: Cpp_JSON_ProjectEditor.openTransformEditor()
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "transform", 24)
              ToolTip.text: qsTr("Calibrate, filter, or convert the value with a short formula or "
                                 + "script.")
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
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "duplicate", 24)
              ToolTip.text: qsTr("Make a copy of this dataset with the same settings.")
            }

            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("Delete")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              ToolTip.text: qsTr("Remove this dataset from the group.")
              onClicked: Cpp_JSON_ProjectModel.deleteCurrentDataset()
              enabled: Cpp_JSON_ProjectEditor.currentDatasetIsEditable
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "delete", 24)
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

          searchable: true
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
