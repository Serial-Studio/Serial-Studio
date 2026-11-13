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

FolderView {
  id: root

  folderId: Cpp_JSON_ProjectEditor.selectedFolderId
  editable: Cpp_JSON_ProjectModel.customizeWorkspaces
  folderName: Cpp_JSON_ProjectModel.workspaceFolderTitle(folderId)

  renameText: qsTr("Rename")
  deleteText: qsTr("Delete")
  titleColumn: qsTr("Title")
  fallbackTitle: qsTr("Folder")
  countColumn: qsTr("Contents")
  addItemText: qsTr("Add Workspace")
  renameToolTip: qsTr("Rename folder")
  deleteToolTip: qsTr("Delete folder")
  addFolderText: qsTr("Add Sub-folder")
  addItemToolTip: qsTr("Add a workspace inside this folder")
  addFolderToolTip: qsTr("Add a sub-folder inside this folder")
  emptyText: qsTr("This folder is empty. Use the toolbar to add a workspace or sub-folder.")

  onRefreshRequested: {
    folderName = Cpp_JSON_ProjectModel.workspaceFolderTitle(folderId)
    contents = Cpp_JSON_ProjectEditor.workspaceFolderContents(folderId)
  }
  onAddItemClicked: Cpp_JSON_ProjectModel.promptAddWorkspaceInFolder(folderId)
  onAddFolderClicked: Cpp_JSON_ProjectModel.promptAddWorkspaceFolder(folderId)
  onRenameClicked: Cpp_JSON_ProjectModel.promptRenameWorkspaceFolder(folderId)
  onDeleteClicked: Cpp_JSON_ProjectModel.confirmDeleteWorkspaceFolder(folderId)
  onEntryClicked: (entry) => {
    if (entry.isFolder)
      Cpp_JSON_ProjectEditor.selectWorkspaceFolder(entry.id)
    else
      Cpp_JSON_ProjectEditor.selectWorkspace(entry.id)
  }

  Connections {
    target: Cpp_JSON_ProjectModel
    function onEditorWorkspacesChanged() { Qt.callLater(root.refresh) }
  }
}
