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

#include <QCoreApplication>

namespace DataModel {

class ProjectEditor;

/**
 * @brief Fills the facade's combo vocabulary: the translated option lists and label maps the
 *        forms read, plus one widget-picker entry per installed extension. The lists stay on the
 *        facade because Forms, Commit and MultiSelect read them; this sub-object only writes.
 */
class EditorVocabulary {
  Q_DECLARE_TR_FUNCTIONS(DataModel::ProjectEditor)

public:
  explicit EditorVocabulary(ProjectEditor& editor);
  EditorVocabulary(EditorVocabulary&&)                 = delete;
  EditorVocabulary(const EditorVocabulary&)            = delete;
  EditorVocabulary& operator=(EditorVocabulary&&)      = delete;
  EditorVocabulary& operator=(const EditorVocabulary&) = delete;

  void generateComboBoxModels();
  void appendExtensionWidgets();

private:
  void generateFftLists();
  void generateFormatMaps();
  void generateWidgetMaps();
  void generateSourceLists();

private:
  ProjectEditor& m_editor;
};

}  // namespace DataModel
