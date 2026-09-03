/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
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

#include <memory>

#include "DataModel/Dialogs/TransmitTestDialog.h"
#include "DataModel/Editors/EmbeddedCodeEditorItem.h"
#include "DataModel/Editors/ScriptTemplateCatalog.h"
#include "DataModel/Frame.h"

namespace Misc {
class Translator;
}  // namespace Misc

namespace DataModel {

class ProjectEditor;
class ProjectModel;

/**
 * @brief QML-embeddable code editor for output-widget transmit scripts.
 */
class OutputCodeEditor : public EmbeddedCodeEditorItem {
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
  explicit OutputCodeEditor(QQuickItem* parent = nullptr);

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
  void selectAll();
  void readCode();
  void formatDocument();
  void formatSelection();
  void selectTemplate();
  void testTransmitFunction();
  void reload(bool guiTrigger = false);

public:
  [[nodiscard]] static QString defaultTemplate();

private:
  void loadTemplates();

private:
  bool m_readingCode;
  Misc::Translator& m_translator;
  DataModel::ProjectEditor& m_projectEditor;
  DataModel::ProjectModel& m_projectModel;
  ScriptTemplateCatalog m_templates;
  std::unique_ptr<TransmitTestDialog> m_testDialog;
};

}  // namespace DataModel
