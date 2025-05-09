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
    // New project
    //
    Widgets.BigButton {
      text: qsTr("New")
      Layout.alignment: Qt.AlignVCenter
      onClicked: Cpp_JSON_ProjectModel.newJsonFile()
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/new.svg"
    }

    //
    // Open
    //
    Widgets.BigButton {
      text: qsTr("Open")
      Layout.alignment: Qt.AlignVCenter
      onClicked: Cpp_JSON_ProjectModel.openJsonFile()
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/open.svg"
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
    // Save
    //
    Widgets.BigButton {
      text: qsTr("Save")
      Layout.alignment: Qt.AlignVCenter
      onClicked: Cpp_JSON_ProjectModel.saveJsonFile(false)
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/save.svg"
      enabled: Cpp_JSON_ProjectModel.modified && Cpp_JSON_ProjectModel.groupCount > 0 && Cpp_JSON_ProjectModel.datasetCount > 0
    }

    //
    // Save As
    //
    Widgets.BigButton {
      text: qsTr("Save As")
      Layout.alignment: Qt.AlignVCenter
      onClicked: Cpp_JSON_ProjectModel.saveJsonFile(true)
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/save-as.svg"
      enabled: Cpp_JSON_ProjectModel.groupCount > 0 && Cpp_JSON_ProjectModel.datasetCount > 0
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
    Widgets.BigButton {
      text: qsTr("Action")
      Layout.alignment: Qt.AlignVCenter
      onClicked: Cpp_JSON_ProjectModel.addAction()
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
    // Add data grid
    //
    Widgets.BigButton {
      text: qsTr("Data Grid")
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-datagrid.svg"
      onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Data Grid"), SerialStudio.DataGrid)
    }

    //
    // Add multiplot
    //
    Widgets.BigButton {
      text: qsTr("Multiple Plots")
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-multiplot.svg"
      onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Multiple Plot"), SerialStudio.MultiPlot)
    }

    //
    // Add 3D Plot
    //
    Widgets.BigButton {
      text: qsTr("3D Plot")
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-plot3d.svg"
      onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("3D Plot"), SerialStudio.Plot3D)
    }

    //
    // Add accelerometer
    //
    Widgets.BigButton {
      text: qsTr("Accelerometer")
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-accelerometer.svg"
      onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Accelerometer"), SerialStudio.Accelerometer)
    }

    //
    // Add gyroscope
    //
    Widgets.BigButton {
      text: qsTr("Gyroscope")
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-gyroscope.svg"
      onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Gyroscope"), SerialStudio.Gyroscope)
    }

    //
    // Add map
    //
    Widgets.BigButton {
      text: qsTr("Map")
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-gps.svg"
      onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("GPS Map"), SerialStudio.GPS)
    }

    //
    // Add group
    //
    Widgets.BigButton {
      text: qsTr("Container")
      Layout.alignment: Qt.AlignVCenter
      icon.source: "qrc:/rcc/icons/project-editor/toolbar/add-group.svg"
      onClicked: Cpp_JSON_ProjectModel.addGroup(qsTr("Dataset Container"), SerialStudio.NoGroupWidget)
    }

    //
    // Horizontal spacer
    //
    Item {
      Layout.fillWidth: true
    }
  }
}
