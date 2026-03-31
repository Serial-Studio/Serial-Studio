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
      // Group actions panel
      //
      Rectangle {
        id: header

        z: 2
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
          // Dataset add buttons (hidden for output groups)
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Dataset")
            Layout.alignment: Qt.AlignVCenter
            visible: !Cpp_JSON_ProjectEditor.currentGroupIsOutputPanel
            enabled: Cpp_JSON_ProjectEditor.currentGroupIsEditable
            ToolTip.text: qsTr("Add a generic dataset to the current group")
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-dataset.svg"
            onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetGeneric)
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Plot")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            visible: !Cpp_JSON_ProjectEditor.currentGroupIsOutputPanel
            enabled: Cpp_JSON_ProjectEditor.currentGroupIsEditable
            ToolTip.text: qsTr("Add a 2D plot to visualize numeric data")
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-plot.svg"
            onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetPlot)
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("FFT Plot")
            Layout.alignment: Qt.AlignVCenter
            visible: !Cpp_JSON_ProjectEditor.currentGroupIsOutputPanel
            enabled: Cpp_JSON_ProjectEditor.currentGroupIsEditable
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-fft.svg"
            onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetFFT)
            ToolTip.text: qsTr("Add an FFT plot for frequency domain visualization")
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Bar/Level")
            Layout.alignment: Qt.AlignVCenter
            visible: !Cpp_JSON_ProjectEditor.currentGroupIsOutputPanel
            enabled: Cpp_JSON_ProjectEditor.currentGroupIsEditable
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-bar.svg"
            ToolTip.text: qsTr("Add a bar or level indicator for scaled values")
            onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetBar)
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Gauge")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            visible: !Cpp_JSON_ProjectEditor.currentGroupIsOutputPanel
            enabled: Cpp_JSON_ProjectEditor.currentGroupIsEditable
            ToolTip.text: qsTr("Add a gauge widget for analog-style visualization")
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-gauge.svg"
            onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetGauge)
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Compass")
            Layout.alignment: Qt.AlignVCenter
            visible: !Cpp_JSON_ProjectEditor.currentGroupIsOutputPanel
            enabled: Cpp_JSON_ProjectEditor.currentGroupIsEditable
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-compass.svg"
            ToolTip.text: qsTr("Add a compass to display directional or angular data")
            onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetCompass)
          }

          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("LED")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            visible: !Cpp_JSON_ProjectEditor.currentGroupIsOutputPanel
            enabled: Cpp_JSON_ProjectEditor.currentGroupIsEditable
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-led.svg"
            ToolTip.text: qsTr("Add an LED indicator for binary status signals")
            onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetLED)
          }

          //
          // Output widget add buttons (only visible for output groups)
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Button")
            Layout.alignment: Qt.AlignVCenter
            visible: Cpp_JSON_ProjectEditor.currentGroupIsOutputPanel
            onClicked: Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputButton)
            ToolTip.text: qsTr("Add a button that sends a command on click")
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-output-button.svg"
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Slider")
            Layout.alignment: Qt.AlignVCenter
            visible: Cpp_JSON_ProjectEditor.currentGroupIsOutputPanel
            onClicked: Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputSlider)
            ToolTip.text: qsTr("Add a slider for sending scaled numeric values")
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-output-slider.svg"
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Toggle")
            Layout.alignment: Qt.AlignVCenter
            visible: Cpp_JSON_ProjectEditor.currentGroupIsOutputPanel
            onClicked: Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputToggle)
            ToolTip.text: qsTr("Add a toggle switch for on/off commands")
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-output-toggle.svg"
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Text Field")
            Layout.alignment: Qt.AlignVCenter
            visible: Cpp_JSON_ProjectEditor.currentGroupIsOutputPanel
            onClicked: Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputTextField)
            ToolTip.text: qsTr("Add a text field for typing and sending commands")
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-output-textfield.svg"
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Knob")
            Layout.alignment: Qt.AlignVCenter
            visible: Cpp_JSON_ProjectEditor.currentGroupIsOutputPanel
            onClicked: Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputKnob)
            ToolTip.text: qsTr("Add a rotary knob for setpoint control")
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-output-knob.svg"
          }

          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Ramp")
            Layout.alignment: Qt.AlignVCenter
            visible: Cpp_JSON_ProjectEditor.currentGroupIsOutputPanel
            onClicked: Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputRampGenerator)
            ToolTip.text: qsTr("Add a ramp generator for timed value sweeps")
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-output-ramp.svg"
          }

          //
          // Spacer
          //
          Item {
            Layout.fillWidth: true
          }

          //
          // Duplicate group
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Duplicate")
            Layout.alignment: Qt.AlignVCenter
            onClicked: Cpp_JSON_ProjectModel.duplicateCurrentGroup()
            ToolTip.text: qsTr("Duplicate the current group and its contents")
            icon.source: "qrc:/rcc/icons/project-editor/actions/duplicate.svg"
          }

          //
          // Delete group
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Delete")
            Layout.alignment: Qt.AlignVCenter
            onClicked: Cpp_JSON_ProjectModel.deleteCurrentGroup()
            icon.source: "qrc:/rcc/icons/project-editor/actions/delete.svg"
            ToolTip.text: qsTr("Delete the current group and all contained datasets")
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
      // Group model editor
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
          parameterWidth: Math.min(delegate.width * 0.3, 256)

          Binding {
            target: delegate
            property: "modelPointer"
            value: Cpp_JSON_ProjectEditor.groupModel
          }
        }
      }
    }
  }
}
