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

#include <QCheckBox>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace Misc {
class ThemeManager;
class CommonFonts;
class Translator;
}  // namespace Misc

namespace DataModel {

class FrameBuilder;

/**
 * @brief Dialog for exercising an output-widget transmit script against sample input.
 */
class TransmitTestDialog : public QDialog {
  Q_OBJECT

public:
  explicit TransmitTestDialog(QWidget* parent = nullptr);

  void setTransmitCode(const QString& code);

public slots:
  void clear();
  void evaluate();

private slots:
  void onThemeChanged();
  void onLanguageChanged();
  void onInputModeChanged(Qt::CheckState state);
  void onInputDataChanged(const QString& text);

private:
  bool validateHexInput(const QString& text);
  QString formatHexInput(const QString& text);
  void displayOutput(const QByteArray& result, const QString& errorMsg);

private:
  QString m_transmitCode;
  Misc::ThemeManager& m_themeManager;
  Misc::CommonFonts& m_commonFonts;
  Misc::Translator& m_translator;
  DataModel::FrameBuilder& m_frameBuilder;
  QLabel* m_inputTitle;
  QLabel* m_outputTitle;
  QLabel* m_byteCountLabel;
  QGroupBox* m_inputGroup;
  QGroupBox* m_outputGroup;
  QLineEdit* m_userInput;
  QCheckBox* m_hexCheckBox;
  QPushButton* m_clearButton;
  QPushButton* m_evaluateButton;
  QPlainTextEdit* m_rawOutput;
  QPlainTextEdit* m_hexOutput;
};

}  // namespace DataModel
