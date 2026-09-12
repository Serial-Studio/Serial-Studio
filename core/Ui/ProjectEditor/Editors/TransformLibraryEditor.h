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

#include "ProjectEditor/Editors/EmbeddedCodeEditorItem.h"

namespace DataModel {

class ProjectModel;

/**
 * @brief QML-embeddable editor for one of the project's shared transform libraries (spec 0083):
 *        the Lua chunk every Lua dataset transform can call, or the JavaScript one (addendum A).
 */
class TransformLibraryEditor : public EmbeddedCodeEditorItem {
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
  Q_PROPERTY(bool lua
             READ lua
             WRITE setLua
             NOTIFY luaChanged)
  // clang-format on

signals:
  void luaChanged();
  void textChanged();
  void modifiedChanged();

public:
  explicit TransformLibraryEditor(QQuickItem* parent = nullptr);

  [[nodiscard]] bool isModified() const noexcept;
  [[nodiscard]] bool undoAvailable() const noexcept;
  [[nodiscard]] bool redoAvailable() const noexcept;
  [[nodiscard]] bool lua() const noexcept;
  [[nodiscard]] QString text() const;
  [[nodiscard]] static QString starterLibrary(bool lua);

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
  void setLua(bool lua);
  void formatDocument();
  void formatSelection();

private:
  void evaluateJs();

private:
  bool m_lua;
  bool m_readingCode;
  bool m_initialLoad;
  DataModel::ProjectModel& m_projectModel;
};

}  // namespace DataModel
