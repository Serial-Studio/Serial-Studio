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
#include <QStringList>
#include <QTimer>

#include "Core/DataModel/Frame.h"
#include "DataModel/Editors/ScriptTemplateCatalog.h"
#include "DataModel/Scripting/TransmitScriptCheck.h"
#include "ProjectEditor/Editors/EmbeddedCodeEditorItem.h"

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
  Q_PROPERTY(QStringList templateNames
             READ templateNames
             NOTIFY templatesChanged)
  Q_PROPERTY(bool scriptValid
             READ scriptValid
             NOTIFY validityChanged)
  Q_PROPERTY(int scriptStatus
             READ scriptStatus
             NOTIFY validityChanged)
  Q_PROPERTY(int scriptErrorLine
             READ scriptErrorLine
             NOTIFY validityChanged)
  Q_PROPERTY(QString scriptError
             READ scriptError
             NOTIFY validityChanged)
  // clang-format on

signals:
  void textChanged();
  void modifiedChanged();
  void validityChanged();
  void templatesChanged();
  void scriptAccepted(const QString& code);

public:
  explicit OutputCodeEditor(QQuickItem* parent = nullptr);

  [[nodiscard]] bool isModified() const noexcept;
  [[nodiscard]] bool undoAvailable() const noexcept;
  [[nodiscard]] bool redoAvailable() const noexcept;
  [[nodiscard]] QString text() const;
  [[nodiscard]] bool scriptValid() const noexcept;
  [[nodiscard]] int scriptStatus() const noexcept;
  [[nodiscard]] int scriptErrorLine() const noexcept;
  [[nodiscard]] QString scriptError() const;
  [[nodiscard]] QStringList templateNames() const;
  [[nodiscard]] Q_INVOKABLE bool save();

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
  void commit();
  void reportVerdict();
  void bindPreview(QObject* preview);
  void applyTemplate(const int index);
  void reload(bool guiTrigger = false);

public:
  [[nodiscard]] static QString defaultTemplate();

private:
  void loadTemplates();
  void flushInputMethod();
  void validateNow();
  void refreshVerdict(const bool persist);
  [[nodiscard]] QString verdictDetail() const;
  void scheduleValidation();

private:
  bool m_readingCode;
  bool m_validating;
  QTimer m_validateTimer;
  TransmitScriptVerdict m_verdict;
  Misc::Translator& m_translator;
  DataModel::ProjectEditor& m_projectEditor;
  DataModel::ProjectModel& m_projectModel;
  ScriptTemplateCatalog m_templates;
};

}  // namespace DataModel
