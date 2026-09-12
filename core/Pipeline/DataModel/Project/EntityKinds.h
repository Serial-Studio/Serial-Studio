/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#pragma once

namespace DataModel {

/**
 * @brief Kind of a project entity as the editor tree, the bulk operations and the model's
 *        select-after-create requests address it. The numeric values are the contract with QML
 *        (ProjectEditor.Kind*) and the tree roles; ProjectEditor::ItemKind aliases every one.
 */
enum EntityKind : int {
  KindNone = 0,
  KindGroup,
  KindDataset,
  KindWorkspace,
  KindWorkspaceFolder,
  KindAction,
  KindOutputWidget,
  KindMqttPublisher,
  KindControlScript,
  KindGroupFolder,
  KindUserTable,
  KindTableFolder,
  KindSource,
  KindProjectRoot,
  KindFrameParser,
  KindGroupsRoot,
  KindTablesRoot,
  KindSystemDatasets,
  KindWorkspacesRoot,
  KindInfluxSink,
  KindTransformLibrary,
  KindScriptsRoot,
  KindJsLibrary,
  KindExportRoot,
};

/**
 * @brief Widget rendered for one editor form row, as the widgetType role carries it; the model's
 *        row filter keys its section-header grouping rule off RowSectionHeader.
 *        ProjectEditor::EditorWidget aliases every one.
 */
enum FormRowKind : int {
  RowTextField = 0,
  RowHexTextField,
  RowIntField,
  RowFloatField,
  RowCheckBox,
  RowComboBox,
  RowIconPicker,
  RowSectionHeader,
  RowPasswordField,
  RowAutoIntField,
  RowButton,
  RowNavRow,
  RowColorPicker,
};

}  // namespace DataModel
