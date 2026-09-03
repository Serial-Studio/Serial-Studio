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

#pragma once

#include "DataModel/Editors/EmbeddedCodeEditorItem.h"

namespace DataModel {

class FrameParser;
class ProjectEditor;
class ProjectModel;

/**
 * @brief QML-embeddable code editor for the JavaScript / Lua frame parser.
 */
class JsCodeEditor : public EmbeddedCodeEditorItem {
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
  Q_PROPERTY(int sourceId
             READ  sourceId
             WRITE setSourceId
             NOTIFY sourceIdChanged)
  Q_PROPERTY(int language
             READ  language
             WRITE setLanguage
             NOTIFY languageChanged)
  // clang-format on

signals:
  void textChanged();
  void modifiedChanged();
  void sourceIdChanged();
  void languageChanged();

public:
  explicit JsCodeEditor(QQuickItem* parent = nullptr);

  [[nodiscard]] bool isModified() const noexcept;
  [[nodiscard]] bool undoAvailable() const noexcept;
  [[nodiscard]] bool redoAvailable() const noexcept;
  [[nodiscard]] int sourceId() const noexcept;
  [[nodiscard]] int language() const noexcept;
  [[nodiscard]] QString text() const;

  Q_INVOKABLE [[nodiscard]] bool prepareParserTest();

public slots:
  void cut();
  void undo();
  void redo();
  void help();
  void copy();
  void paste();
  void apply();
  void importFile();
  void evaluate();
  void readCode();
  void selectAll();
  void formatDocument();
  void formatSelection();
  void selectTemplate();
  void setSourceId(const int sourceId);
  void setLanguage(const int language);
  void switchLanguage(const int language);
  void reload(const bool guiTrigger = false);
  void loadDefaultTemplate(const bool guiTrigger = false);

private:
  void switchNativeLanguage(const int language);

private:
  int m_sourceId;
  int m_language;
  bool m_readingCode;
  DataModel::ProjectModel& m_projectModel;
  DataModel::ProjectEditor& m_projectEditor;
  DataModel::FrameParser& m_frameParser;
};

}  // namespace DataModel
