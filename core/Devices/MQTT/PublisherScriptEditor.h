/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QCheckBox>
#  include <QCodeEditor>
#  include <QComboBox>
#  include <QDialog>
#  include <QLabel>
#  include <QLineEdit>
#  include <QPushButton>
#  include <QSyntaxStyle>
#  include <QVBoxLayout>

namespace MQTT {

/**
 * @brief Native QDialog editor for the MQTT publisher script.
 */
class PublisherScriptEditor : public QDialog {
  Q_OBJECT

public:
  explicit PublisherScriptEditor(QWidget* parent = nullptr);

  [[nodiscard]] QString code() const;
  [[nodiscard]] int language() const;

signals:
  void scriptApplied(const QString& code, int language);

public slots:
  void displayDialog(const QString& currentCode, int language);

private slots:
  void onApply();
  void onTest();
  void onClear();
  void onFormat();
  void onFormatLine();
  void buildTemplates();
  void onThemeChanged();
  void onTemplateSelected(int index);
  void onLanguageChanged(int index);
  void onHexToggled(Qt::CheckState state);
  void onInputChanged(const QString& text);
  void showEditorContextMenu(const QPoint& localPos);

private:
  void applyLanguage(int language);
  [[nodiscard]] int detectTemplate() const;
  [[nodiscard]] QString runScript(const QString& code,
                                  int language,
                                  const QByteArray& frame,
                                  QString& errorOut);
  [[nodiscard]] static QString defaultPlaceholder(int language);
  [[nodiscard]] static bool definesMqttFunction(const QString& code, int language);
  [[nodiscard]] static bool validateHexInput(const QString& text);
  [[nodiscard]] static QString formatHexInput(const QString& text);

  void buildEditorWidgets();
  [[nodiscard]] QHBoxLayout* buildToolbarLayout();
  [[nodiscard]] QHBoxLayout* buildTestLayout();
  [[nodiscard]] QHBoxLayout* buildButtonLayout();
  void wireSignals();
  void installShortcuts();

private:
  int m_language;
  bool m_hexInput;

  QSyntaxStyle m_style;
  QCodeEditor* m_editor;

  QComboBox* m_languageCombo;
  QComboBox* m_templateCombo;

  QLineEdit* m_testInput;
  QLineEdit* m_testOutput;
  QCheckBox* m_hexCheckBox;
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

}  // namespace MQTT

#endif  // BUILD_COMMERCIAL
