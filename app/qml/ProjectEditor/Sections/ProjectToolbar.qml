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
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../../Widgets" as Widgets

Rectangle {
  id: root
  implicitWidth: layout.implicitWidth + 32

  //
  // Calculate offset based on platform
  //
  property int titlebarHeight: Cpp_NativeWindow.titlebarHeight(projectEditor)
  Connections {
    target: projectEditor
    function onVisibilityChanged() {
      root.titlebarHeight = Cpp_NativeWindow.titlebarHeight(projectEditor)
    }
  }

  //
  // Set toolbar height
  //
  Layout.minimumHeight: titlebarHeight + 64 + 16
  Layout.maximumHeight: titlebarHeight + 64 + 16

  //
  // Titlebar text
  //
  Label {
    text: projectEditor.title
    visible: root.titlebarHeight > 0
    color: Cpp_ThemeManager.colors["titlebar_text"]
    font: Cpp_Misc_CommonFonts.customUiFont(1.07, true)

    anchors {
      topMargin: 6
      top: parent.top
      horizontalCenter: parent.horizontalCenter
    }
  }

  //
  // Toolbar background
  //
  gradient: Gradient {
    GradientStop {
      position: 0
      color: Cpp_ThemeManager.colors["toolbar_top"]
    }

    GradientStop {
      position: 1
      color: Cpp_ThemeManager.colors["toolbar_bottom"]
    }
  }

  //
  // Toolbar bottom border
  //
  Rectangle {
    height: 1
    color: Cpp_ThemeManager.colors["toolbar_border"]

    anchors {
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
  }

  //
  // Move window with toolbar
  //
  DragHandler {
    target: null
    onActiveChanged: {
      if (active)
        projectEditor.startSystemMove()
    }
  }

  //
  // Toolbar controls
  //
  RowLayout {
    id: layout
    spacing: 4

    anchors {
      margins: 2
      left: parent.left
      right: parent.right
      verticalCenter: parent.verticalCenter
      verticalCenterOffset: root.titlebarHeight / 2
    }

    //
    // Horizontal spacer
    //
    Item {
      implicitWidth: 1
    }

    //
    // New button
    //
    Widgets.ToolbarButton {
      text: qsTr("New")
      Layout.alignment: Qt.AlignVCenter
      onClicked: Cpp_JSON_ProjectModel.newJsonFile()
      ToolTip.text: qsTr("Create a new JSON project")
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/new.svg"
    }

    //
    // Other file operations
    //
    GridLayout {
      rows: 3
      columns: 1
      rowSpacing: 4
      columnSpacing: 4
      Layout.alignment: Qt.AlignVCenter

      Widgets.ToolbarButton {
        iconSize: 16
        text: qsTr("Open")
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        onClicked: Cpp_JSON_ProjectModel.openJsonFile()
        ToolTip.text: qsTr("Open an existing JSON project")
        icon.source: "qrc:/rcc/icons/project-editor/toolbar/open.svg"
      }

      Widgets.ToolbarButton {
        iconSize: 16
        text: qsTr("Save")
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        ToolTip.text: qsTr("Save the current project")
        onClicked: Cpp_JSON_ProjectModel.saveJsonFile(false)
        icon.source: "qrc:/rcc/icons/project-editor/toolbar/save.svg"
        enabled: Cpp_JSON_ProjectModel.modified && Cpp_JSON_ProjectModel.groupCount > 0 && Cpp_JSON_ProjectModel.datasetCount > 0
      }

      Widgets.ToolbarButton {
        iconSize: 16
        text: qsTr("Save As")
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        onClicked: Cpp_JSON_ProjectModel.saveJsonFile(true)
        ToolTip.text: qsTr("Save the current project under a new name")
        icon.source: "qrc:/rcc/icons/project-editor/toolbar/save-as.svg"
        enabled: Cpp_JSON_ProjectModel.groupCount > 0 && Cpp_JSON_ProjectModel.datasetCount > 0
      }
    }

    //
    // Separator
    //
    Rectangle {
      width: 1
      Layout.fillHeight: true
      Layout.maximumHeight: 64
      Layout.alignment: Qt.AlignVCenter
      color: Cpp_ThemeManager.colors["toolbar_separator"]
    }

    //
    // Add action
    //
    Widgets.ToolbarButton {
      text: qsTr("Action")
      Layout.alignment: Qt.AlignVCenter
      onClicked: Cpp_JSON_ProjectModel.addAction()
      ToolTip.text: qsTr("Add a new action to the project")
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-action.svg"
    }

    //
    // Separator
    //
    Rectangle {
      width: 1
      Layout.fillHeight: true
      Layout.maximumHeight: 64
      Layout.alignment: Qt.AlignVCenter
      color: Cpp_ThemeManager.colors["toolbar_separator"]
    }

    //
    // Add Dataset
    //
    Widgets.ToolbarButton {
      text: qsTr("Dataset")
      Layout.alignment: Qt.AlignVCenter
      ToolTip.text: qsTr("Add a generic dataset")
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-dataset.svg"
      onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetGeneric)
    }

    //
    // Other dataset types
    //
    GridLayout {
      rows: 3
      columns: 2
      rowSpacing: 4
      columnSpacing: 4
      Layout.alignment: Qt.AlignVCenter

      Widgets.ToolbarButton {
        text: qsTr("Plot")

        iconSize: 16
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        ToolTip.text: qsTr("Add a 2D plot dataset")
        icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-plot.svg"
        onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetPlot)
      }

      Widgets.ToolbarButton {
        text: qsTr("FFT Plot")

        iconSize: 16
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        ToolTip.text: qsTr("Add a Fast Fourier Transform plot")
        icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-fft.svg"
        onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetFFT)
      }

      Widgets.ToolbarButton {
        text: qsTr("Gauge")

        iconSize: 16
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        ToolTip.text: qsTr("Add a gauge widget for numeric data")
        icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-gauge.svg"
        onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetGauge)
      }

      Widgets.ToolbarButton {
        text: qsTr("Level Indicator")

        iconSize: 16
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        ToolTip.text: qsTr("Add a vertical bar level indicator")
        icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-bar.svg"
        onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetBar)
      }

      Widgets.ToolbarButton {
        text: qsTr("Compass")

        iconSize: 16
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        ToolTip.text: qsTr("Add a compass widget for directional data")
        icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-compass.svg"
        onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetCompass)
      }

      Widgets.ToolbarButton {
        text: qsTr("LED Indicator")

        iconSize: 16
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        ToolTip.text: qsTr("Add an LED-style status indicator")
        icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-led.svg"
        onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetLED)
      }
    }

    //
    // Separator
    //
    Rectangle {
      width: 1
      Layout.fillHeight: true
      Layout.maximumHeight: 64
      Layout.alignment: Qt.AlignVCenter
      color: Cpp_ThemeManager.colors["toolbar_separator"]
    }

    //
    // Add Group
    //
    Widgets.ToolbarButton {
      text: qsTr("Group")
      Layout.alignment: Qt.AlignVCenter
      ToolTip.text: qsTr("Add a dataset container group")
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-group.svg"
      onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Dataset Container"), SerialStudio.NoGroupWidget)
    }

    //
    // Other group types
    //
    GridLayout {
      rows: 3
      columns: 2
      rowSpacing: 4
      columnSpacing: 4
      Layout.alignment: Qt.AlignVCenter

      Widgets.ToolbarButton {
        text: qsTr("Table")

        iconSize: 16
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        ToolTip.text: qsTr("Add a data table view")
        icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-datagrid.svg"
        onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Data Grid"), SerialStudio.DataGrid)
      }

      Widgets.ToolbarButton {
        text: qsTr("Multi-Plot")

        iconSize: 16
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        ToolTip.text: qsTr("Add a 2D plot with multiple signals")
        icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-multiplot.svg"
        onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Multiple Plot"), SerialStudio.MultiPlot)
      }

      Widgets.ToolbarButton {
        text: qsTr("3D Plot")

        iconSize: 16
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        ToolTip.text: qsTr("Add a 3D plot visualization")
        icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-plot3d.svg"
        onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("3D Plot"), SerialStudio.Plot3D)
      }

      Widgets.ToolbarButton {
        text: qsTr("Accelerometer")

        iconSize: 16
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        ToolTip.text: qsTr("Add a group for 3-axis accelerometer data")
        icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-accelerometer.svg"
        onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Accelerometer"), SerialStudio.Accelerometer)
      }

      Widgets.ToolbarButton {
        text: qsTr("Gyroscope")

        iconSize: 16
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-gyroscope.svg"
        onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Gyroscope"), SerialStudio.Gyroscope)
       ToolTip.text: qsTr("Add a group for 3-axis gyroscope data (angular rates are integrated into orientation automatically)")
      }

      Widgets.ToolbarButton {
        text: qsTr("GPS Map")

        iconSize: 16
        horizontalLayout: true
        Layout.alignment: Qt.AlignLeft
        ToolTip.text: qsTr("Add a map widget for GPS data")
        icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-gps.svg"
        onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("GPS Map"), SerialStudio.GPS)
      }
    }

    //
    // Separator
    //
    Rectangle {
      width: 1
      Layout.fillHeight: true
      Layout.maximumHeight: 64
      Layout.alignment: Qt.AlignVCenter
      color: Cpp_ThemeManager.colors["toolbar_separator"]
    }

    //
    // Help
    //
    Widgets.ToolbarButton {
      text: qsTr("Help")
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/help.svg"
      ToolTip.text: qsTr("Open the online Project Editor documentation")
      onClicked: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/wiki/Project-Editor")
    }


    //
    // Horizontal spacer
    //
    Item {
      Layout.fillWidth: true
    }
  }
}
