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

//
// Shared editor caption actions (back/forward + move up/down); buttons fade to 0.5 when inactive.
//
RowLayout {
  id: root

  spacing: 2
  visible: !app.runtimeMode

  readonly property int view: Cpp_JSON_ProjectEditor.currentView

  function doReorder(dir) {
    if (view === ProjectEditor.GroupView)
      return Cpp_JSON_ProjectEditor.moveCurrentGroup(dir)

    if (view === ProjectEditor.DatasetView)
      return Cpp_JSON_ProjectEditor.moveCurrentDataset(dir)

    if (view === ProjectEditor.ActionView)
      return Cpp_JSON_ProjectEditor.moveCurrentAction(dir)

    if (view === ProjectEditor.OutputWidgetView)
      return Cpp_JSON_ProjectEditor.moveCurrentOutputWidget(dir)

    if (view === ProjectEditor.WorkspaceView)
      return Cpp_JSON_ProjectEditor.moveWorkspace(Cpp_JSON_ProjectEditor.selectedWorkspaceId, dir)

    if (view === ProjectEditor.GroupFolderView)
      Cpp_JSON_ProjectModel.moveGroupFolderInParent(Cpp_JSON_ProjectEditor.selectedGroupFolderId, dir)

    if (view === ProjectEditor.TableFolderView)
      Cpp_JSON_ProjectModel.moveTableFolderInParent(Cpp_JSON_ProjectEditor.selectedTableFolderId, dir)

    if (view === ProjectEditor.WorkspaceFolderView)
      Cpp_JSON_ProjectModel.moveWorkspaceFolderInParent(Cpp_JSON_ProjectEditor.selectedFolderId, dir)
  }

  function actEnabled(act) {
    if (act === "back")
      return Cpp_JSON_ProjectEditor.canGoBack

    if (act === "forward")
      return Cpp_JSON_ProjectEditor.canGoForward

    if (act === "moveUp")
      return Cpp_JSON_ProjectEditor.canMoveCurrentUp

    return Cpp_JSON_ProjectEditor.canMoveCurrentDown
  }

  function invoke(act) {
    if (act === "back") {
      Cpp_JSON_ProjectEditor.navigateBack()
      return
    }

    if (act === "forward") {
      Cpp_JSON_ProjectEditor.navigateForward()
      return
    }

    if (act === "moveUp") {
      root.doReorder(-1)
      return
    }

    if (act === "moveDown")
      root.doReorder(1)
  }

  Item {
    Layout.fillWidth: true
  }

  Repeater {
    delegate: _navButton
    model: [
      {
        act: "back",
        tip: qsTr("Go back"),
        icon: Cpp_Misc_IconRegistry.icon("editor", "left", 16)
      },
      {
        act: "forward",
        tip: qsTr("Go forward"),
        icon: Cpp_Misc_IconRegistry.icon("editor", "right", 16)
      }
    ]
  }

  Rectangle {
    Layout.preferredWidth: 1
    Layout.preferredHeight: 18
    Layout.alignment: Qt.AlignVCenter
    color: Cpp_ThemeManager.colors["pane_caption_border"]
  }

  Repeater {
    delegate: _navButton
    model: [
      {
        act: "moveUp",
        tip: qsTr("Move Up"),
        icon: Cpp_Misc_IconRegistry.icon("editor", "move-up", 16)
      },
      {
        act: "moveDown",
        tip: qsTr("Move Down"),
        icon: Cpp_Misc_IconRegistry.icon("editor", "move-down", 16)
      }
    ]
  }

  Component {
    id: _navButton

    Widgets.IconButton {
      required property var modelData

      flat: true
      iconSize: 16
      implicitWidth: 24
      background: Item {}
      icon.color: "transparent"
      ToolTip.text: modelData.tip
      icon.source: modelData.icon
      opacity: enabled ? 1.0 : 0.5
      enabled: root.actEnabled(modelData.act)
      Layout.alignment: Qt.AlignVCenter
      onClicked: root.invoke(modelData.act)
    }
  }
}
