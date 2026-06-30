/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
import QtQuick.Controls

import SerialStudio

//
// Shared "Add Group" menu listing the group-type templates. Files the new group into
// parentFolderId (-1 = top level), so the Groups root and group folder views stay consistent.
//
Menu {
  id: root

  property int parentFolderId: -1

  function add(title, widget) {
    Cpp_JSON_ProjectModel.addGroup(title, widget, -1, root.parentFolderId)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Dataset Container")
    icon.source: "qrc:/icons/project-editor/toolbar/add-group.svg"
    onTriggered: root.add(qsTr("Dataset Container"), SerialStudio.NoGroupWidget)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Data Grid")
    icon.source: "qrc:/icons/project-editor/toolbar/add-datagrid.svg"
    onTriggered: root.add(qsTr("Data Grid"), SerialStudio.DataGrid)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Multi-Plot")
    icon.source: "qrc:/icons/project-editor/toolbar/add-multiplot.svg"
    onTriggered: root.add(qsTr("Multiple Plot"), SerialStudio.MultiPlot)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("3D Plot")
    icon.source: "qrc:/icons/project-editor/toolbar/add-plot3d.svg"
    onTriggered: root.add(qsTr("3D Plot"), SerialStudio.Plot3D)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Accelerometer")
    icon.source: "qrc:/icons/project-editor/toolbar/add-accelerometer.svg"
    onTriggered: root.add(qsTr("Accelerometer"), SerialStudio.Accelerometer)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Gyroscope")
    icon.source: "qrc:/icons/project-editor/toolbar/add-gyroscope.svg"
    onTriggered: root.add(qsTr("Gyroscope"), SerialStudio.Gyroscope)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("GPS Map")
    icon.source: "qrc:/icons/project-editor/toolbar/add-gps.svg"
    onTriggered: root.add(qsTr("GPS Map"), SerialStudio.GPS)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Image View")
    icon.source: "qrc:/icons/project-editor/toolbar/image.svg"
    onTriggered: root.add(qsTr("Image View"), SerialStudio.ImageView)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Web View")
    icon.source: "qrc:/icons/project-editor/toolbar/add-webview.svg"
    onTriggered: root.add(qsTr("Web View"), SerialStudio.WebView)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Painter Widget")
    icon.source: "qrc:/icons/project-editor/toolbar/add-painter.svg"
    onTriggered: root.add(qsTr("Painter Widget"), SerialStudio.Painter)
  }
}
