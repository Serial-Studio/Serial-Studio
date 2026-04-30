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
  // Top toolbar section
  //
  Rectangle {
    height: root.titlebarHeight
    color: Cpp_ThemeManager.colors["toolbar_top"]
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
  }

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
  Rectangle {
    anchors.fill: parent
    anchors.topMargin: root.titlebarHeight
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
  // Ribbon toolbar
  //
  Widgets.RibbonToolbar {
    anchors {
      left: parent.left
      right: parent.right
      verticalCenter: parent.verticalCenter
      verticalCenterOffset: root.titlebarHeight / 2
    }
    height: 64 + 16

    //
    // File section
    //
    Widgets.RibbonSection {
      Widgets.ToolbarButton {
        text: qsTr("New")
        Layout.alignment: Qt.AlignVCenter
        onClicked: Cpp_JSON_ProjectModel.newJsonFile()
        ToolTip.text: qsTr("Create a new JSON project")
        icon.source: "qrc:/icons/project-editor/toolbar/new.svg"
      }

      GridLayout {
        rows: 3
        columns: 1
        rowSpacing: 0
        columnSpacing: 4
        Layout.alignment: Qt.AlignVCenter

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("Open")
          horizontalLayout: true
          Layout.alignment: Qt.AlignLeft
          onClicked: Cpp_JSON_ProjectModel.openJsonFile()
          ToolTip.text: qsTr("Open an existing JSON project")
          icon.source: "qrc:/icons/project-editor/toolbar/open.svg"
        }

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("Save")
          horizontalLayout: true
          Layout.alignment: Qt.AlignLeft
          enabled: Cpp_JSON_ProjectModel.modified
          ToolTip.text: qsTr("Save the current project")
          onClicked: Cpp_JSON_ProjectModel.saveJsonFile(false)
          icon.source: "qrc:/icons/project-editor/toolbar/save.svg"
        }

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("Save As")
          horizontalLayout: true
          Layout.alignment: Qt.AlignLeft
          onClicked: Cpp_JSON_ProjectModel.saveJsonFile(true)
          ToolTip.text: qsTr("Save the current project under a new name")
          icon.source: "qrc:/icons/project-editor/toolbar/save-as.svg"
        }
      }
    }

    //
    // Project lock section
    //
    Widgets.RibbonSection {
      Widgets.ToolbarButton {
        text: qsTr("Lock")
        Layout.alignment: Qt.AlignVCenter
        onClicked: Cpp_JSON_ProjectModel.lockProject()
        icon.source: "qrc:/icons/project-editor/toolbar/lock.svg"
        ToolTip.text: qsTr("Set a password and lock the Project Editor")
      }
    }

    //
    // Add Device section (commercial only)
    //
    Widgets.RibbonSection {
      visible: Cpp_CommercialBuild

      Widgets.ToolbarButton {
        text: qsTr("Add Device")
        Layout.alignment: Qt.AlignVCenter
        onClicked: Cpp_JSON_ProjectModel.addSource()
        ToolTip.text: qsTr("Add a new data source (device) to the project")
        icon.source: "qrc:/icons/project-editor/toolbar/add-device.svg"
      }
    }

    //
    // Action section
    //
    Widgets.RibbonSection {
      Widgets.ToolbarButton {
        text: qsTr("Action")
        Layout.alignment: Qt.AlignVCenter
        onClicked: Cpp_JSON_ProjectModel.addAction()
        ToolTip.text: qsTr("Add a new action to the project")
        icon.source: "qrc:/icons/project-editor/toolbar/add-action.svg"
      }
    }

    //
    // Output controls section (collapsible)
    //
    Widgets.RibbonSection {
      collapsible: true
      collapsePriority: 10
      collapsedText: qsTr("Output")
      collapsedIcon: "qrc:/icons/project-editor/toolbar/add-output-panel.svg"

      Widgets.ToolbarButton {
        text: qsTr("Output")
        Layout.alignment: Qt.AlignVCenter
        onClicked: Cpp_JSON_ProjectModel.addOutputPanel()
        ToolTip.text: qsTr("Add a new output control panel with a button")
        icon.source: "qrc:/icons/project-editor/toolbar/add-output-panel.svg"
      }

      GridLayout {
        rows: 3
        columns: 2
        rowSpacing: 0
        columnSpacing: 4
        Layout.alignment: Qt.AlignVCenter

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("Slider")
          horizontalLayout: true
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add an output slider control")
          icon.source: "qrc:/icons/project-editor/toolbar/add-output-slider.svg"
          onClicked: Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputSlider)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("Toggle")
          horizontalLayout: true
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add an output toggle control")
          icon.source: "qrc:/icons/project-editor/toolbar/add-output-toggle.svg"
          onClicked: Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputToggle)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("Knob")
          horizontalLayout: true
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add an output knob control")
          icon.source: "qrc:/icons/project-editor/toolbar/add-output-knob.svg"
          onClicked: Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputKnob)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          horizontalLayout: true
          text: qsTr("Text Field")
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add an output text field control")
          icon.source: "qrc:/icons/project-editor/toolbar/add-output-textfield.svg"
          onClicked: Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputTextField)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("Button")
          horizontalLayout: true
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add an output button control")
          icon.source: "qrc:/icons/project-editor/toolbar/add-output-button.svg"
          onClicked: Cpp_JSON_ProjectModel.addOutputControl(SerialStudio.OutputButton)
        }
      }
    }

    //
    // Dataset section (collapsible)
    //
    Widgets.RibbonSection {
      collapsible: true
      collapsePriority: 20
      collapsedText: qsTr("Dataset")
      collapsedIcon: "qrc:/icons/project-editor/toolbar/add-dataset.svg"

      Widgets.ToolbarButton {
        text: qsTr("Dataset")
        Layout.alignment: Qt.AlignVCenter
        ToolTip.text: qsTr("Add a generic dataset")
        icon.source: "qrc:/icons/project-editor/toolbar/add-dataset.svg"
        onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetGeneric)
      }

      GridLayout {
        rows: 3
        columns: 2
        rowSpacing: 0
        columnSpacing: 4
        Layout.alignment: Qt.AlignVCenter

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("Plot")
          horizontalLayout: true
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add a 2D plot dataset")
          icon.source: "qrc:/icons/project-editor/toolbar/add-plot.svg"
          onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetPlot)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("FFT Plot")
          horizontalLayout: true
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add a Fast Fourier Transform plot")
          icon.source: "qrc:/icons/project-editor/toolbar/add-fft.svg"
          onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetFFT)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("Gauge")
          horizontalLayout: true
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add a gauge widget for numeric data")
          icon.source: "qrc:/icons/project-editor/toolbar/add-gauge.svg"
          onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetGauge)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          horizontalLayout: true
          text: qsTr("Level Indicator")
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add a vertical bar level indicator")
          icon.source: "qrc:/icons/project-editor/toolbar/add-bar.svg"
          onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetBar)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("Compass")
          horizontalLayout: true
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add a compass widget for directional data")
          icon.source: "qrc:/icons/project-editor/toolbar/add-compass.svg"
          onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetCompass)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          horizontalLayout: true
          text: qsTr("LED Indicator")
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add an LED-style status indicator")
          icon.source: "qrc:/icons/project-editor/toolbar/add-led.svg"
          onClicked: Cpp_JSON_ProjectModel.addDataset(SerialStudio.DatasetLED)
        }
      }
    }

    //
    // Group section (collapsible)
    //
    Widgets.RibbonSection {
      collapsible: true
      collapsePriority: 30
      collapsedText: qsTr("Group")
      collapsedIcon: "qrc:/icons/project-editor/toolbar/add-group.svg"

      Widgets.ToolbarButton {
        text: qsTr("Group")
        Layout.alignment: Qt.AlignVCenter
        ToolTip.text: qsTr("Add a dataset container group")
        icon.source: "qrc:/icons/project-editor/toolbar/add-group.svg"
        onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Dataset Container"), SerialStudio.NoGroupWidget)
      }

      Widgets.ToolbarButton {
        text: qsTr("Image")
        Layout.alignment: Qt.AlignVCenter
        ToolTip.text: qsTr("Add an image/video stream viewer")
        icon.source: "qrc:/icons/project-editor/toolbar/image.svg"
        onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Image View"), SerialStudio.ImageView)
      }

      GridLayout {
        rows: 3
        columns: 2
        rowSpacing: 0
        columnSpacing: 4
        Layout.alignment: Qt.AlignVCenter

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("Table")
          horizontalLayout: true
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add a data table view")
          icon.source: "qrc:/icons/project-editor/toolbar/add-datagrid.svg"
          onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Data Grid"), SerialStudio.DataGrid)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          horizontalLayout: true
          text: qsTr("Multi-Plot")
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add a 2D plot with multiple signals")
          icon.source: "qrc:/icons/project-editor/toolbar/add-multiplot.svg"
          onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Multiple Plot"), SerialStudio.MultiPlot)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("3D Plot")
          horizontalLayout: true
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add a 3D plot visualization")
          icon.source: "qrc:/icons/project-editor/toolbar/add-plot3d.svg"
          onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("3D Plot"), SerialStudio.Plot3D)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          horizontalLayout: true
          text: qsTr("Accelerometer")
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add a group for 3-axis accelerometer data")
          icon.source: "qrc:/icons/project-editor/toolbar/add-accelerometer.svg"
          onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Accelerometer"), SerialStudio.Accelerometer)
        }

        Widgets.ToolbarButton {
          iconSize: 16
          horizontalLayout: true
          text: qsTr("Gyroscope")
          Layout.alignment: Qt.AlignLeft
          icon.source: "qrc:/icons/project-editor/toolbar/add-gyroscope.svg"
          onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Gyroscope"), SerialStudio.Gyroscope)
          ToolTip.text: qsTr("Add a group for 3-axis gyroscope data")
        }

        Widgets.ToolbarButton {
          iconSize: 16
          text: qsTr("GPS Map")
          horizontalLayout: true
          Layout.alignment: Qt.AlignLeft
          ToolTip.text: qsTr("Add a map widget for GPS data")
          icon.source: "qrc:/icons/project-editor/toolbar/add-gps.svg"
          onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("GPS Map"), SerialStudio.GPS)
        }
      }
    }

    //
    // Help section
    //
    RowLayout {
      spacing: 4
      Layout.fillHeight: true
      Layout.alignment: Qt.AlignVCenter

      Widgets.ToolbarButton {
        text: qsTr("Help Center")
        Layout.alignment: Qt.AlignVCenter
        onClicked: app.showHelpCenter("project-editor")
        ToolTip.text: qsTr("Open the Project Editor documentation")
        icon.source: "qrc:/icons/project-editor/toolbar/help.svg"
      }
    }
  }
}
