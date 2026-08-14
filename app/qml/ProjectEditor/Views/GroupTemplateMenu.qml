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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
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
    icon.source: Cpp_Misc_IconRegistry.icon("editor", "group", 16)
    onTriggered: root.add(qsTr("Dataset Container"), SerialStudio.NoGroupWidget)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Data Grid")
    icon.source: Cpp_Misc_IconRegistry.icon("widgets", "datagrid", 16)
    onTriggered: root.add(qsTr("Data Grid"), SerialStudio.DataGrid)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Bar Panel")
    icon.source: Cpp_Misc_IconRegistry.icon("widgets", "barpanel", 16)
    onTriggered: root.add(qsTr("Bar Panel"), SerialStudio.BarPanel)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Multi-Plot")
    icon.source: Cpp_Misc_IconRegistry.icon("widgets", "multiplot", 16)
    onTriggered: root.add(qsTr("Multiple Plot"), SerialStudio.MultiPlot)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("3D Plot")
    icon.source: Cpp_Misc_IconRegistry.icon("widgets", "plot3d", 16)
    onTriggered: root.add(qsTr("3D Plot"), SerialStudio.Plot3D)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Accelerometer")
    icon.source: Cpp_Misc_IconRegistry.icon("widgets", "accelerometer", 16)
    onTriggered: root.add(qsTr("Accelerometer"), SerialStudio.Accelerometer)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Gyroscope")
    icon.source: Cpp_Misc_IconRegistry.icon("widgets", "gyroscope", 16)
    onTriggered: root.add(qsTr("Gyroscope"), SerialStudio.Gyroscope)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("GPS Map")
    icon.source: Cpp_Misc_IconRegistry.icon("widgets", "gps", 16)
    onTriggered: root.add(qsTr("GPS Map"), SerialStudio.GPS)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Image View")
    icon.source: Cpp_Misc_IconRegistry.icon("widgets", "image", 16)
    onTriggered: root.add(qsTr("Image View"), SerialStudio.ImageView)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Web View")
    icon.source: Cpp_Misc_IconRegistry.icon("widgets", "webview", 16)
    onTriggered: root.add(qsTr("Web View"), SerialStudio.WebView)
  }

  MenuItem {
    icon.width: 16
    icon.height: 16
    text: qsTr("Painter Widget")
    icon.source: Cpp_Misc_IconRegistry.icon("editor", "add-painter", 16)
    onTriggered: root.add(qsTr("Painter Widget"), SerialStudio.Painter)
  }
}
