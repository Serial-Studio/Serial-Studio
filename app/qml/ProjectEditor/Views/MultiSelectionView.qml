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
  icon: Cpp_Misc_IconRegistry.icon("editor", "dataset", 16)
  title: qsTr("%1 items selected").arg(Cpp_JSON_ProjectEditor.multiSelectionCount)

  actionComponent: EditorNavActions {}

  //
  // Picks the aggregate form model for the current multi-selection kind.
  //
  function batchModel() {
    switch (Cpp_JSON_ProjectEditor.multiSelectionKind) {
    case ProjectEditor.KindDataset:
      return Cpp_JSON_ProjectEditor.datasetModel
    case ProjectEditor.KindGroup:
      return Cpp_JSON_ProjectEditor.groupModel
    case ProjectEditor.KindAction:
      return Cpp_JSON_ProjectEditor.actionModel
    case ProjectEditor.KindSource:
      return Cpp_JSON_ProjectEditor.sourceModel
    case ProjectEditor.KindOutputWidget:
      return Cpp_JSON_ProjectEditor.outputWidgetModel
    }

    return null
  }

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
    // Lazy-loaded alarm-band editor dialog. In multi-select the edited bands are applied to every
    // selected dataset. Activated via Cpp_JSON_ProjectEditor signal.
    //
    Loader {
      id: alarmBandsDialog

      active: false
      asynchronous: false
      source: "qrc:/serial-studio.com/gui/qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml"

      onLoaded: alarmBandsDialog.item.closing.connect(() => alarmBandsDialog.active = false)

      function show(gId, dId, rMin, rMax, bands) {
        alarmBandsDialog.active = true
        if (alarmBandsDialog.item)
          alarmBandsDialog.item.showDialog(gId, dId, rMin, rMax, bands)
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
      // Search band: above the secondary toolbar, aligned with the tree search
      //
      EditorSearchBand {
        tableDelegate: delegate
      }

      //
      // Dataset visualization toolbar: toggles fan out to every selected dataset. Datasets only.
      //
      Rectangle {
        z: 3
        Layout.fillWidth: true
        Layout.preferredHeight: 80
        color: Cpp_ThemeManager.colors["groupbox_background"]
        visible: Cpp_JSON_ProjectEditor.multiSelectionKind === ProjectEditor.KindDataset

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
              ToolTip.text: qsTr("Toggle 2D plot for every selected dataset")
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetPlot
              onClicked: {
                const option = SerialStudio.DatasetPlot
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectEditor.changeDatasetOptionForSelection(option, !value)
              }
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("FFT Plot")
              Layout.alignment: Qt.AlignVCenter
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "fft", 24)
              ToolTip.text: qsTr("Toggle FFT plot for every selected dataset")
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetFFT
              onClicked: {
                const option = SerialStudio.DatasetFFT
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectEditor.changeDatasetOptionForSelection(option, !value)
              }
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Waterfall")
              Layout.alignment: Qt.AlignVCenter
              ToolTip.text: qsTr("Toggle waterfall for every selected dataset")
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "waterfall", 24)
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetWaterfall
              onClicked: {
                const option = SerialStudio.DatasetWaterfall
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectEditor.changeDatasetOptionForSelection(option, !value)
              }
            }
          }

          //
          // Instantaneous-value widgets
          //
          Widgets.RibbonSection {
            collapsible: true
            collapsePriority: 20
            collapsedText: qsTr("Widgets")
            collapsedIcon: Cpp_Misc_IconRegistry.icon("widgets", "gauge", 32)

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Bar/Level")
              Layout.alignment: Qt.AlignVCenter
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "bar", 24)
              ToolTip.text: qsTr("Set bar/level for every selected dataset")
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetBar
              onClicked: {
                const option = SerialStudio.DatasetBar
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectEditor.changeDatasetOptionForSelection(option, !value)
              }
            }

            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("Gauge")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              ToolTip.text: qsTr("Set gauge for every selected dataset")
              icon.source: Cpp_Misc_IconRegistry.icon("widgets", "gauge", 24)
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetGauge
              onClicked: {
                const option = SerialStudio.DatasetGauge
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectEditor.changeDatasetOptionForSelection(option, !value)
              }
            }

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Compass")
              Layout.alignment: Qt.AlignVCenter
              ToolTip.text: qsTr("Set compass for every selected dataset")
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "compass", 24)
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetCompass
              onClicked: {
                const option = SerialStudio.DatasetCompass
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectEditor.changeDatasetOptionForSelection(option, !value)
              }
            }

            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("Meter")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              ToolTip.text: qsTr("Set meter for every selected dataset")
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "meter", 24)
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetMeter
              onClicked: {
                const option = SerialStudio.DatasetMeter
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectEditor.changeDatasetOptionForSelection(option, !value)
              }
            }

            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("LED")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              ToolTip.text: qsTr("Toggle LED for every selected dataset")
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "led", 24)
              checked: Cpp_JSON_ProjectEditor.datasetOptions & SerialStudio.DatasetLED
              onClicked: {
                const option = SerialStudio.DatasetLED
                const value = Cpp_JSON_ProjectEditor.datasetOptions & option
                Cpp_JSON_ProjectEditor.changeDatasetOptionForSelection(option, !value)
              }
            }
          }

          //
          // Behavior: alarm thresholds shared across the whole selection
          //
          Widgets.RibbonSection {
            collapsible: true
            collapsePriority: 10
            collapsedText: qsTr("Behavior")
            collapsedIcon: Cpp_Misc_IconRegistry.icon("editor", "alarm-bands", 24)

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Alarm Bands")
              Layout.alignment: Qt.AlignVCenter
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "alarm-bands", 24)
              ToolTip.text: qsTr("Define colored value ranges and apply them to every selected dataset.")
              onClicked: Cpp_JSON_ProjectEditor.openAlarmBandsEditorForSelection()
            }
          }

          //
          // Spacer
          //
          Widgets.RibbonSpacer {}

          //
          // Lifecycle: duplicate / delete the whole selection
          //
          Widgets.RibbonSection {
            showSeparator: false

            Widgets.ToolbarButton {
              iconSize: 24
              toolbarButton: false
              text: qsTr("Duplicate")
              Layout.alignment: Qt.AlignVCenter
              ToolTip.text: qsTr("Duplicate every selected dataset")
              onClicked: Cpp_JSON_ProjectModel.duplicateSelectedItems(
                           Cpp_JSON_ProjectEditor.selectedTreeItems())
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "duplicate", 24)
            }

            Widgets.ToolbarButton {
              iconSize: 24
              text: qsTr("Delete")
              toolbarButton: false
              Layout.alignment: Qt.AlignVCenter
              ToolTip.text: qsTr("Delete every selected dataset")
              icon.source: Cpp_Misc_IconRegistry.icon("editor", "delete", 24)
              onClicked: Cpp_JSON_ProjectModel.confirmDeleteSelectedItems(
                           Cpp_JSON_ProjectEditor.selectedTreeItems())
            }
          }
        }

        Rectangle {
          height: 1
          color: Cpp_ThemeManager.colors["groupbox_border"]
          anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        }
      }

      //
      // Explanatory banner: shared fields fan out, per-item fields are locked.
      //
      Rectangle {
        z: 2
        Layout.fillWidth: true
        Layout.preferredHeight: 40
        color: Cpp_ThemeManager.colors["groupbox_background"]

        RowLayout {
          spacing: 8
          anchors.fill: parent
          anchors.leftMargin: 12
          anchors.rightMargin: 12

          Image {
            sourceSize: Qt.size(18, 18)
            Layout.alignment: Qt.AlignVCenter
            source: Cpp_Misc_IconRegistry.icon("editor", "dataset", 24)
          }

          Label {
            Layout.fillWidth: true
            elide: Label.ElideRight
            color: Cpp_ThemeManager.colors["text"]
            text: qsTr("Editing %1 items. Shared fields apply to all; per-item fields are locked.").arg(Cpp_JSON_ProjectEditor.multiSelectionCount)
          }
        }

        Rectangle {
          height: 1
          color: Cpp_ThemeManager.colors["groupbox_border"]
          anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        }
      }

      //
      // Aggregate form
      //
      ScrollView {
        id: view

        contentWidth: width
        Layout.fillWidth: true
        Layout.fillHeight: true
        contentHeight: delegate.implicitHeight
        ScrollBar.vertical.policy: delegate.implicitHeight > view.height ? ScrollBar.AlwaysOn
                                                                         : ScrollBar.AsNeeded

        TableDelegate {
          id: delegate

          searchable: true
          width: parent.width
          headerVisible: false

          Binding {
            target: delegate
            property: "modelPointer"
            value: root.batchModel()
          }
        }
      }
    }
  }
}
