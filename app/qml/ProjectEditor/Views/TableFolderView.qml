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

  addItemIcon: "add-table"
  folderId: Cpp_JSON_ProjectEditor.selectedTableFolderId
  folderName: Cpp_JSON_ProjectModel.tableFolderTitle(folderId)

  renameText: qsTr("Rename")
  deleteText: qsTr("Delete")
  titleColumn: qsTr("Title")
  fallbackTitle: qsTr("Folder")
  countColumn: qsTr("Variables")
  renameToolTip: qsTr("Rename folder")
  deleteToolTip: qsTr("Delete folder")
  addFolderText: qsTr("Add Sub-folder")
  addItemText: qsTr("Add Variables Table")
  addFolderToolTip: qsTr("Add a sub-folder inside this folder")
  addItemToolTip: qsTr("Add a Variables table inside this folder")
  emptyText: qsTr("This folder is empty. Use the toolbar to add a table or sub-folder.")

  onRefreshRequested: {
    folderName = Cpp_JSON_ProjectModel.tableFolderTitle(folderId)
    contents = Cpp_JSON_ProjectEditor.tableFolderContents(folderId)
  }
  onAddItemClicked: Cpp_JSON_ProjectModel.promptAddTableInFolder(folderId)
  onAddFolderClicked: Cpp_JSON_ProjectModel.promptAddTableFolder(folderId)
  onRenameClicked: Cpp_JSON_ProjectModel.promptRenameTableFolder(folderId)
  onDeleteClicked: Cpp_JSON_ProjectModel.confirmDeleteTableFolder(folderId)
  onEntryClicked: (entry) => {
    if (entry.isFolder)
      Cpp_JSON_ProjectEditor.selectTableFolder(entry.id)
    else
      Cpp_JSON_ProjectEditor.selectUserTable(entry.path)
  }

  Connections {
    target: Cpp_JSON_ProjectModel
    function onTablesChanged() { Qt.callLater(root.refresh) }
  }
}
