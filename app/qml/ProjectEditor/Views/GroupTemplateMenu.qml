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

import SerialStudio

import "../../Widgets" as Widgets
import "../../Commands" as Commands

//
// Shared "Add Group" menu for the toolbars, rendered from the same registry fragment the
// context menus use. Files the new group into parentFolderId (-1 = top level).
//
Widgets.CommandMenu {
  id: root

  property int parentFolderId: -1

  model: _templateModel
  onAboutToShow: {
    _templateBindings.setTarget({
                                  "id": root.parentFolderId,
                                  "kind": root.parentFolderId >= 0
                                          ? ProjectEditor.KindGroupFolder
                                          : ProjectEditor.KindGroupsRoot
                                })
    root.rebuild("editor-menu/add-group")
  }

  Commands.ProjectEditorMenuBindings {
    id: _templateBindings
  }

  Commands.CommandModel {
    id: _templateModel

    context: "editor"
    bindingSets: [_templateBindings]
  }
}
