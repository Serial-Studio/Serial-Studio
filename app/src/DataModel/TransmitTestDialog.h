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

#include <QCheckBox>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace DataModel {

/**
 * @brief Dialog for testing output widget transmit functions with sample input.
 *
 * Provides an interface for users to:
 * - Enter a sample value (number or text, optionally in hex)
 * - Execute the transmit function with that value
 * - View the output as raw text, hex bytes, and byte count
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
