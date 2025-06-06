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
      // Group actions panel
      //
      Rectangle {
        z: 2
        id: header
        Layout.fillWidth: true
        height: layout.implicitHeight + 12
        color: Cpp_ThemeManager.colors["window_toolbar_background"]

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
          // Add generic dataset
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Dataset")
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectModel.currentGroupIsEditable
            onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetGeneric)
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-dataset.svg"
          }

          //
          // Add plot
          //
          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Plot")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectModel.currentGroupIsEditable
            onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetPlot)
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-plot.svg"
          }

          //
          // Add FFT plot
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("FFT Plot")
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectModel.currentGroupIsEditable
            onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetFFT)
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-fft.svg"
          }

          //
          // Add bar
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Bar/Level")
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectModel.currentGroupIsEditable
            onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetBar)
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-bar.svg"
          }

          //
          // Add gauge
          //
          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("Gauge")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectModel.currentGroupIsEditable
            onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetGauge)
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-gauge.svg"
          }

          //
          // Add compass
          //
          Widgets.ToolbarButton {
            iconSize: 24
            toolbarButton: false
            text: qsTr("Compass")
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectModel.currentGroupIsEditable
            onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetCompass)
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-compass.svg"
          }

          //
          // Add LED
          //
          Widgets.ToolbarButton {
            iconSize: 24
            text: qsTr("LED")
            toolbarButton: false
            Layout.alignment: Qt.AlignVCenter
            enabled: Cpp_JSON_ProjectModel.currentGroupIsEditable
            onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetLED)
            icon.source: "qrc:/rcc/icons/project-editor/actions/add-led.svg"
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
          modelPointer: Cpp_JSON_ProjectModel.groupModel

          footerItem: ColumnLayout {
            spacing: 0

            Image {
              sourceSize: Qt.size(96, 96)
              Layout.alignment: Qt.AlignHCenter
              source: "qrc:/rcc/images/tip.svg"
            }

            Item {
              implicitHeight: 16
            }

            Label {
              Layout.alignment: Qt.AlignHCenter
              horizontalAlignment: Label.AlignHCenter
              font: Cpp_Misc_CommonFonts.customUiFont(1.6, true)
              text: qsTr("Groups hold multiple datasets and organize your data.")
            }

            Item {
              implicitHeight: 8
            }

            Label {
              opacity: 0.8
              Layout.alignment: Qt.AlignHCenter
              horizontalAlignment: Label.AlignHCenter
              Layout.maximumWidth: delegate.width * 0.9
              wrapMode: Label.WrapAtWordBoundaryOrAnywhere
              font: Cpp_Misc_CommonFonts.customUiFont(1.4, false)
              text: qsTr("Use the sub-toolbar to quickly add plots, FFTs, and other dataset types to this group.")
            }
          }
        }
      }
    }
  }
}
