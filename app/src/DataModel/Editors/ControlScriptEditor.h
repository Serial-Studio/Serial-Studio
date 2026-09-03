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

#include "DataModel/Editors/EmbeddedCodeEditorItem.h"

namespace DataModel {

class ProjectEditor;
class ProjectModel;

/**
 * @brief QML-embeddable code editor for the project setup()/loop() control script.
 */
class ControlScriptEditor : public EmbeddedCodeEditorItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool isModified
             READ isModified
             NOTIFY modifiedChanged)
  Q_PROPERTY(bool undoAvailable
             READ undoAvailable
             NOTIFY modifiedChanged)
  Q_PROPERTY(bool redoAvailable
             READ redoAvailable
             NOTIFY modifiedChanged)
  Q_PROPERTY(QString text
             READ text
             NOTIFY textChanged)
  // clang-format on

signals:
  void textChanged();
  void modifiedChanged();

public:
  explicit ControlScriptEditor(QQuickItem* parent = nullptr);

  [[nodiscard]] bool isModified() const noexcept;
  [[nodiscard]] bool undoAvailable() const noexcept;
  [[nodiscard]] bool redoAvailable() const noexcept;
  [[nodiscard]] QString text() const;

public slots:
  void cut();
  void undo();
  void redo();
  void copy();
  void paste();
  void importFile();
  void reload();
  void evaluate();
  void selectAll();
  void readCode();
  void formatDocument();
  void formatSelection();

private:
  bool m_readingCode;
  bool m_initialLoad;
  DataModel::ProjectEditor& m_projectEditor;
  DataModel::ProjectModel& m_projectModel;
};

}  // namespace DataModel
