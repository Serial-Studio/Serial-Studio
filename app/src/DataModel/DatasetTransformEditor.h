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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QCodeEditor>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSyntaxStyle>
#include <QVBoxLayout>

namespace DataModel {

/**
 * @brief Dialog for editing per-dataset value transform expressions.
 *
 * Provides a syntax-highlighted code editor (Lua or JavaScript), built-in
 * template snippets loaded from Qt resources, and a live test area where
 * the user can enter a raw value and see the transformed output.
 *
 * Template code is tracked so that switching language automatically loads
 * the equivalent template in the new language. If the user has manually
 * edited the code (diverged from any template), the language switch only
 * changes the syntax highlighter and leaves the code untouched.
 */
class DatasetTransformEditor : public QDialog {
  Q_OBJECT

public:
  explicit DatasetTransformEditor(QWidget* parent = nullptr);

  [[nodiscard]] QString code() const;
  [[nodiscard]] int language() const;
  [[nodiscard]] int targetGroupId() const noexcept;
  [[nodiscard]] int targetDatasetId() const noexcept;

signals:
  void transformApplied(const QString& code, int language, int groupId, int datasetId);

public slots:
  void displayDialog(const QString& datasetTitle,
                     const QString& currentCode,
                     int language,
                     int groupId,
                     int datasetId);

private slots:
  void onApply();
  void onTest();
  void onClear();
  void buildTemplates();
  void onThemeChanged();
  void applyLanguage(int language);
  void onTemplateSelected(int index);
  void onLanguageChanged(int index);

private:
  [[nodiscard]] int detectTemplate() const;
  [[nodiscard]] QString testTransform(const QString& code, int language, double inputValue);
  [[nodiscard]] static QString defaultPlaceholder(int language);
  [[nodiscard]] static bool isDefaultPlaceholder(const QString& code, int language);
  [[nodiscard]] static bool definesTransformFunction(const QString& code, int language);

private:
  int m_language;
  int m_targetGroupId;
  int m_targetDatasetId;

  QSyntaxStyle m_style;
  QCodeEditor* m_editor;

  QComboBox* m_languageCombo;
  QComboBox* m_templateCombo;

  QLineEdit* m_testInput;
  QLabel* m_testOutput;
  QPushButton* m_testButton;

  QPushButton* m_applyButton;
  QPushButton* m_cancelButton;
  QPushButton* m_clearButton;

  struct Template {
    QString file;
    QString name;
    QString luaCode;
    QString jsCode;
  };

  QList<Template> m_templates;
};

}  // namespace DataModel
