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

#include <QCodeEditor>
#include <QComboBox>
#include <QDialog>
#include <QGraphicsOpacityEffect>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSyntaxStyle>
#include <QTableWidget>
#include <QVariantMap>
#include <QVBoxLayout>
#include <QWidget>

#include "DataModel/Scripting/ExpressionTransform.h"

namespace Misc {
class CommonFonts;
class ThemeManager;
class Translator;
}  // namespace Misc

namespace DataModel {

class FrameBuilder;

/**
 * @brief Dialog for editing per-dataset value transform scripts (Lua, JS or Expression) with their
 *        named parameters (spec 0083); the project's Lua library is edited from the tree.
 */
class DatasetTransformEditor : public QDialog {
  Q_OBJECT

public:
  explicit DatasetTransformEditor(QWidget* parent = nullptr);

  [[nodiscard]] QString code() const;
  [[nodiscard]] int language() const;
  [[nodiscard]] QVariantMap params() const;
  [[nodiscard]] int targetGroupId() const noexcept;
  [[nodiscard]] int targetDatasetId() const noexcept;

signals:
  void transformApplied(
    const QString& code, int language, int groupId, int datasetId, const QVariantMap& params);
  void libraryEditRequested(int language);

public slots:
  void displayDialog(const QString& datasetTitle,
                     const QString& currentCode,
                     int language,
                     int groupId,
                     int datasetId,
                     const QVariantMap& params,
                     const QString& luaLibrary,
                     const QString& jsLibrary);
  void setLibraryCode(const QString& luaLibrary, const QString& jsLibrary);

private slots:
  void onApply();
  void onTest();
  void onClear();
  void onFormat();
  void onFormatLine();
  void onAddParam();
  void onRemoveParam();
  void buildTemplates();
  void onThemeChanged();
  void applyLanguage(int language);
  void onTemplateSelected(int index);
  void onLanguageChanged(int index);
  void showEditorContextMenu(const QPoint& localPos);

private:
  enum class TransformStatus {
    Ok,
    SyntaxError,
    NoFunction
  };

  [[nodiscard]] int detectTemplate() const;
  [[nodiscard]] QString testTransform(const QString& code, int language, double inputValue);
  [[nodiscard]] static QString defaultPlaceholder(int language);
  [[nodiscard]] static bool isDefaultPlaceholder(const QString& code, int language);
  [[nodiscard]] static int comboIndexForLanguage(int language);
  [[nodiscard]] static int languageForComboIndex(int index);
  [[nodiscard]] static DataModel::Expression::TableResolver expressionTables();
  [[nodiscard]] static DataModel::Expression::NameResolver expressionResolver(
    DataModel::Expression::SlotTable& table);
  [[nodiscard]] static TransformStatus validateTransform(const QString& code,
                                                         int language,
                                                         const QString& libraryCode,
                                                         QString& error);
  [[nodiscard]] static QVariant parseParamValue(const QString& text);
  [[nodiscard]] static QString paramValueText(const QVariant& value);

  void setParams(const QVariantMap& params);
  void refreshParamsHint();
  void refreshLibraryButton();
  [[nodiscard]] QString libraryFor(int language) const;
  void buildEditorWidgets();
  [[nodiscard]] QHBoxLayout* buildToolbarLayout();
  [[nodiscard]] QWidget* buildTestRow();
  [[nodiscard]] QGroupBox* buildParamsBox();
  [[nodiscard]] QHBoxLayout* buildButtonLayout();
  void wireSignals();
  void installShortcuts();

private:
  int m_language;
  int m_targetGroupId;
  int m_targetDatasetId;
  QString m_libraryCode;
  QString m_libraryCodeJs;

  QSyntaxStyle m_style;
  QCodeEditor* m_editor;

  QComboBox* m_languageCombo;
  QComboBox* m_templateCombo;
  QGraphicsOpacityEffect* m_templateOpacity;

  QLineEdit* m_testInput;
  QLineEdit* m_testOutput;
  QPushButton* m_testButton;

  QPushButton* m_applyButton;
  QPushButton* m_cancelButton;
  QPushButton* m_clearButton;
  QPushButton* m_libraryButton;

  QWidget* m_testRow;
  QGroupBox* m_paramsBox;
  QLabel* m_paramsHint;
  QTableWidget* m_paramsTable;
  QPushButton* m_addParamButton;
  QPushButton* m_removeParamButton;

  struct Template {
    QString file;
    QString name;
    QString luaCode;
    QString jsCode;
  };

  QList<Template> m_templates;

  Misc::CommonFonts& m_commonFonts;
  Misc::ThemeManager& m_themeManager;
  Misc::Translator& m_translator;
  DataModel::FrameBuilder& m_frameBuilder;
};

}  // namespace DataModel
